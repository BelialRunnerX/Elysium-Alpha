#include "glrenderer.h"
#include "../gfx/shading.h"
#include "glcore.h"

#include <cstdio>
#include <cstring>

namespace ely {

namespace {

// The shaders are embedded rather than loaded from disk. A renderer that fails
// because a data file was not copied next to the binary is a renderer that will
// fail on someone else's machine, and these are eighty lines.
//
// GLSL 330 core, the oldest version with everything this needs (integer vertex
// attributes, uniform blocks, `flat` qualifiers). Every lighting constant below
// is a transcription of gfx/shading.h and is pinned by a test; change one,
// change the other.

const char* kVertexShader = R"GLSL(
#version 330 core

// The entire per-vertex input is eight bytes, read as one RGBA16UI attribute
// (see gfx/vertex.h). Component w is not a position: it is the material id in
// its low byte and the packed AO/face-axis in its high byte, which is exactly
// what the last two bytes of the C++ GpuVertex are when read little-endian.
layout(location = 0) in uvec4 inData;

uniform mat4 uViewProj;
uniform vec4 uOriginScale;   // xyz: chunk origin in metres, w: voxel edge

flat out uint vMaterial;
out float vLight;
out float vDepth;

// MUST match kFaceLight in gfx/shading.h. Order: +X, -X, +Y, -Y, +Z, -Z.
const float kFaceLight[6] = float[6](0.86, 0.74, 1.00, 0.55, 0.92, 0.68);
const float kAoBase = 0.52;
const float kAoStep = 0.16;

void main() {
    // Positions arrive in voxel units, chunk-local, and become world metres
    // here. That is deliberate: a float world position three kilometres from
    // the planet centre has about a quarter of a millimetre of precision left,
    // which is coarse against 62.5 mm micro-voxels and shows as vertices
    // shimmering at the far edge of a chunk. An exact integer plus a per-draw
    // origin keeps every corner exact.
    vec3 world = uOriginScale.xyz + vec3(inData.xyz) * uOriginScale.w;
    vec4 clip = uViewProj * vec4(world, 1.0);
    gl_Position = clip;

    vMaterial = inData.w & 0xFFu;
    uint aoAxis = (inData.w >> 8u) & 0xFFu;
    uint ao   = aoAxis & 0x3u;
    uint axis = (aoAxis >> 2u) & 0x7u;

    // Face brightness is constant across a quad (all four corners share an
    // axis) but AO is not, so computing the product here and interpolating it
    // gives the ambient-occlusion gradient for free. This is the same value the
    // software reference renderer computes per vertex.
    vLight = kFaceLight[min(axis, 5u)] * (kAoBase + kAoStep * float(ao));

    // clip.w is -z in view space for this projection: distance along the gaze.
    // Interpolating it, rather than reading gl_FragCoord.z in the fragment
    // shader, keeps fog linear in metres instead of in the depth buffer's
    // curve.
    vDepth = clip.w;
}
)GLSL";

const char* kFragmentShader = R"GLSL(
#version 330 core

layout(std140) uniform Palette {
    vec4 colour[64];   // MAT_COUNT is 43; 64 leaves room to append materials
                       // without touching the shader. vec4 rather than vec3
                       // because std140 aligns array elements to 16 bytes.
} palette;

flat in uint vMaterial;
in float vLight;
in float vDepth;

out vec4 outColour;

const float kFogFull = 520.0;
const float kFogMax = 0.82;
const vec3  kFogColour = vec3(78.0 / 255.0, 92.0 / 255.0, 118.0 / 255.0);

void main() {
    vec3 base = palette.colour[min(vMaterial, 63u)].rgb;
    vec3 lit = base * vLight;

    // Fog saturates below full strength on purpose: at 1.0 distant terrain
    // dissolves into the background and the horizon loses its silhouette.
    float fog = clamp(vDepth / kFogFull, 0.0, kFogMax);
    outColour = vec4(mix(lit, kFogColour, fog), 1.0);
}
)GLSL";

constexpr int kPaletteEntries = 64;

unsigned compileShader(GLenum type, const char* src, std::string* error) {
    const unsigned s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = GL_FALSE;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::string log(size_t(len > 1 ? len : 1), '\0');
        glGetShaderInfoLog(s, GLsizei(log.size()), nullptr, &log[0]);
        if (error) {
            *error = std::string(type == GL_VERTEX_SHADER ? "vertex" : "fragment")
                   + " shader failed to compile:\n" + log;
        }
        glDeleteShader(s);
        return 0;
    }
    return s;
}

}  // namespace

GlRenderer::~GlRenderer() { shutdown(); }

bool GlRenderer::checkError(const char* stage, std::string* error) {
    const GLenum e = glGetError();
    if (e == GL_NO_ERROR) return true;
    if (error) {
        char buf[192];
        std::snprintf(buf, sizeof(buf), "%s: %s (0x%04x)", stage, glErrorName(e),
                      unsigned(e));
        *error = buf;
    }
    return false;
}

bool GlRenderer::init(std::string* error) {
    const unsigned vs = compileShader(GL_VERTEX_SHADER, kVertexShader, error);
    if (!vs) return false;
    const unsigned fs = compileShader(GL_FRAGMENT_SHADER, kFragmentShader, error);
    if (!fs) { glDeleteShader(vs); return false; }

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &len);
        std::string log(size_t(len > 1 ? len : 1), '\0');
        glGetProgramInfoLog(program_, GLsizei(log.size()), nullptr, &log[0]);
        if (error) *error = "shader program failed to link:\n" + log;
        glDeleteProgram(program_);
        program_ = 0;
        return false;
    }

    locViewProj_ = glGetUniformLocation(program_, "uViewProj");
    locOriginScale_ = glGetUniformLocation(program_, "uOriginScale");
    if (locViewProj_ < 0 || locOriginScale_ < 0) {
        if (error) *error = "the shader program is missing uViewProj or "
                            "uOriginScale; they were optimised out, which means "
                            "the vertex shader is not using them";
        return false;
    }

    const GLuint block = glGetUniformBlockIndex(program_, "Palette");
    if (block == 0xFFFFFFFFu) {
        if (error) *error = "the shader program has no Palette uniform block";
        return false;
    }
    glUniformBlockBinding(program_, block, 0);

    glGenBuffers(1, &paletteUbo_);
    glBindBuffer(GL_UNIFORM_BUFFER, paletteUbo_);
    glBufferData(GL_UNIFORM_BUFFER, GLsizeiptr(sizeof(float) * 4 * kPaletteEntries),
                 nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, paletteUbo_);
    updatePalette();

    if (!checkError("renderer init", error)) return false;
    return true;
}

void GlRenderer::updatePalette() {
    if (!paletteUbo_) return;
    // The shader's array is 64 entries; the material table is 43. The tail is
    // zeroed rather than left undefined so that an out-of-range material id
    // draws black — visible and obviously wrong — instead of whatever happened
    // to be in that memory, which would be invisible and subtly wrong.
    float data[kPaletteEntries][4] = {};
    const MaterialPalette pal;
    for (int i = 0; i < MAT_COUNT && i < kPaletteEntries; ++i)
        for (int c = 0; c < 4; ++c) data[i][c] = pal.rgb[i][c];

    glBindBuffer(GL_UNIFORM_BUFFER, paletteUbo_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, GLsizeiptr(sizeof(data)), data);
}

void GlRenderer::uploadChunk(uint64_t id, const GpuMesh& mesh) {
    if (mesh.empty()) { removeChunk(id); return; }

    ChunkBuffers& c = chunks_[id];
    if (c.vao == 0) {
        glGenVertexArrays(1, &c.vao);
        glGenBuffers(1, &c.vbo);
        glGenBuffers(1, &c.ebo);
    }
    stats_.bufferBytes -= c.bytes;

    glBindVertexArray(c.vao);

    glBindBuffer(GL_ARRAY_BUFFER, c.vbo);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(mesh.vertexBytes()),
                 mesh.vertices.data(), GL_STATIC_DRAW);

    // One attribute for the whole eight-byte vertex. glVertexAttribIPointer,
    // not glVertexAttribPointer: the shader declares uvec4 and wants the raw
    // integers. The float version would silently convert and every position
    // would be wrong by whatever the driver's normalisation did.
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 4, GL_UNSIGNED_SHORT, GLsizei(sizeof(GpuVertex)),
                           nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(mesh.indexBytes()),
                 mesh.indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    c.indexCount = int(mesh.indices.size());
    c.origin[0] = mesh.originX;
    c.origin[1] = mesh.originY;
    c.origin[2] = mesh.originZ;
    c.voxelSize = mesh.voxelSize;
    c.bytes = mesh.vertexBytes() + mesh.indexBytes();
    stats_.bufferBytes += c.bytes;
    stats_.chunksResident = uint32_t(chunks_.size());
}

void GlRenderer::destroyChunk(ChunkBuffers& c) {
    if (c.vao) glDeleteVertexArrays(1, &c.vao);
    if (c.vbo) glDeleteBuffers(1, &c.vbo);
    if (c.ebo) glDeleteBuffers(1, &c.ebo);
    c = ChunkBuffers{};
}

void GlRenderer::removeChunk(uint64_t id) {
    auto it = chunks_.find(id);
    if (it == chunks_.end()) return;
    stats_.bufferBytes -= it->second.bytes;
    destroyChunk(it->second);
    chunks_.erase(it);
    stats_.chunksResident = uint32_t(chunks_.size());
}

void GlRenderer::clearChunks() {
    for (auto& kv : chunks_) destroyChunk(kv.second);
    chunks_.clear();
    stats_.bufferBytes = 0;
    stats_.chunksResident = 0;
}

void GlRenderer::render(const Mat4& viewProj, const std::vector<uint64_t>& visible,
                        int width, int height, const float clearColour[3]) {
    stats_.chunksDrawn = 0;
    stats_.trianglesDrawn = 0;

    glViewport(0, 0, width, height);
    glClearColor(clearColour[0], clearColour[1], clearColour[2], 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Back-face culling. Which winding is "front" is a property of the order
    // the greedy mesher pushes its four quad corners, and getting it backwards
    // does not produce an empty screen — it produces a picture of the *inside*
    // of the terrain, which reads as a shading bug rather than a state bug.
    // That is exactly what happened, so this is measured by a test rather than
    // asserted by a comment.
    if (cull == Cull::Off) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(cull == Cull::BackIsCw ? GL_CCW : GL_CW);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program_);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, paletteUbo_);
    glUniformMatrix4fv(locViewProj_, 1, GL_FALSE, &viewProj.m[0][0]);

    for (uint64_t id : visible) {
        auto it = chunks_.find(id);
        // Not an error. A tile can be culled in the same frame its upload is
        // still queued; skipping it is correct and complaining would be noise.
        if (it == chunks_.end()) continue;
        const ChunkBuffers& c = it->second;
        if (c.indexCount == 0) continue;

        const float originScale[4] = {c.origin[0], c.origin[1], c.origin[2],
                                      c.voxelSize};
        glUniform4fv(locOriginScale_, 1, originScale);
        glBindVertexArray(c.vao);
        glDrawElements(GL_TRIANGLES, c.indexCount, GL_UNSIGNED_INT, nullptr);

        ++stats_.chunksDrawn;
        stats_.trianglesDrawn += uint64_t(c.indexCount / 3);
    }
    glBindVertexArray(0);
}

bool GlRenderer::createTarget(int width, int height, std::string* error) {
    destroyTarget();
    targetW_ = width;
    targetH_ = height;

    glGenTextures(1, &colourTex_);
    glBindTexture(GL_TEXTURE_2D, colourTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenRenderbuffers(1, &depthRbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           colourTex_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, depthRbo_);

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        if (error) {
            char buf[128];
            std::snprintf(buf, sizeof(buf),
                          "offscreen framebuffer incomplete (status 0x%04x)",
                          unsigned(status));
            *error = buf;
        }
        destroyTarget();
        return false;
    }
    return checkError("create offscreen target", error);
}

void GlRenderer::destroyTarget() {
    if (fbo_) { glDeleteFramebuffers(1, &fbo_); fbo_ = 0; }
    if (colourTex_) { glDeleteTextures(1, &colourTex_); colourTex_ = 0; }
    if (depthRbo_) { glDeleteRenderbuffers(1, &depthRbo_); depthRbo_ = 0; }
    targetW_ = targetH_ = 0;
}

bool GlRenderer::readTarget(Image& out, std::string* error) const {
    if (!fbo_) {
        if (error) *error = "no offscreen target to read";
        return false;
    }
    const int w = targetW_, h = targetH_;
    std::vector<uint8_t> rgba(size_t(w) * size_t(h) * 4);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFinish();
    // Rows are tightly packed here; the default alignment of 4 happens to be
    // right for RGBA but would silently corrupt an RGB read, so it is set
    // explicitly rather than relied on.
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    if (!checkError("read offscreen target", error)) return false;

    // GL's origin is bottom-left, an Image's is top-left.
    for (int y = 0; y < h; ++y) {
        const uint8_t* row = &rgba[size_t(h - 1 - y) * size_t(w) * 4];
        for (int x = 0; x < w; ++x)
            out.set(x, y, row[x * 4], row[x * 4 + 1], row[x * 4 + 2]);
    }
    return true;
}

void GlRenderer::shutdown() {
    clearChunks();
    destroyTarget();
    if (paletteUbo_) { glDeleteBuffers(1, &paletteUbo_); paletteUbo_ = 0; }
    if (program_) { glDeleteProgram(program_); program_ = 0; }
}

}  // namespace ely
