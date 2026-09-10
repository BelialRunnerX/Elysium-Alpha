// The OpenGL backend.
//
// Everything above this file — generator, mesher, culling, LOD scheduler,
// vertex format, shading model — is tested without a graphics API at all. This
// is the boundary, and it is the only place in the engine that touches GL.
//
// The design, in three decisions:
//
//   * ONE DRAW CALL PER CHUNK, with nothing bound per draw but a vertex array
//     and two uniforms. The chunk's origin and voxel size are a vec4; the
//     view-projection is set once per frame. There is no per-chunk state to
//     allocate or rebind.
//
//   * MATERIALS ARE INDICES, NOT COLOURS. The palette lives in a uniform block
//     the shader reads, so re-tinting the world — biome variation, a damage
//     overlay, a night palette — is a 1 KB buffer write instead of remeshing
//     everything on screen.
//
//   * THE RENDERER OWNS NO WORLD STATE. It is told "here is chunk 47's
//     geometry" and "draw these ids"; it does not know what a planet is. That
//     is what lets the streamer be tested without it, and it without a planet.
//
// The same class drives a window (context from GLFW/SDL) or an offscreen
// framebuffer (HeadlessGl). Only who supplies the context differs.
#pragma once
#include "../image/image.h"
#include "../gfx/mat.h"
#include "../gfx/vertex.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ely {

struct RendererStats {
    uint32_t chunksResident = 0;
    uint32_t chunksDrawn = 0;
    uint64_t trianglesDrawn = 0;
    uint64_t bufferBytes = 0;
};

class GlRenderer {
public:
    GlRenderer() = default;
    ~GlRenderer();

    GlRenderer(const GlRenderer&) = delete;
    GlRenderer& operator=(const GlRenderer&) = delete;

    // Compile the program, build the palette buffer, set fixed state. A
    // context must already be current. Returns false with the shader compiler's
    // own message in `error` — which is the single most useful string in
    // graphics programming and must never be swallowed.
    bool init(std::string* error);
    void shutdown();

    // Add or replace a chunk. Uploading an empty mesh removes it, which is what
    // a chunk that meshed to nothing should do rather than costing a draw call
    // that emits no pixels.
    //
    // `id` is the caller's key — the streamer's tile hash. Not interpreted.
    void uploadChunk(uint64_t id, const GpuMesh& mesh);
    void removeChunk(uint64_t id);
    void clearChunks();
    bool hasChunk(uint64_t id) const { return chunks_.count(id) != 0; }
    size_t chunkCount() const { return chunks_.size(); }

    // Draw. `visible` is the set of chunk ids that survived frustum culling, in
    // the order to draw them — nearest first, so early-Z rejects the most
    // fragments. Ids that are not resident are skipped rather than treated as
    // an error: a tile can be culled in the same frame its upload is still
    // queued, and that is normal, not a bug.
    void render(const Mat4& viewProj, const std::vector<uint64_t>& visible,
                int width, int height, const float clearColour[3]);

    // Re-upload the material palette. Cheap; call it whenever colours change.
    void updatePalette();

    // Offscreen rendering, for tools and tests.
    bool createTarget(int width, int height, std::string* error);
    void destroyTarget();
    // Read the current target back as an image. The GL origin is bottom-left
    // and an Image's is top-left, so this flips; forgetting that is the classic
    // "why is my screenshot upside down".
    bool readTarget(Image& out, std::string* error) const;

    const RendererStats& stats() const { return stats_; }

    // Face culling, exposed because the correct setting is a property of the
    // mesher's vertex order and is decided by measurement, not by argument.
    // See the winding-order test.
    enum class Cull { Off, BackIsCw, BackIsCcw };
    Cull cull = Cull::BackIsCw;   // front faces wind counter-clockwise

    // Check and clear the GL error state. Called at the end of each stage
    // rather than once at the end of a frame: "an error somewhere in the last
    // two hundred calls" is not a bug report.
    static bool checkError(const char* stage, std::string* error);

private:
    struct ChunkBuffers {
        unsigned vao = 0, vbo = 0, ebo = 0;
        int indexCount = 0;
        float origin[3] = {0, 0, 0};
        float voxelSize = 1.0f;
        size_t bytes = 0;
    };

    void destroyChunk(ChunkBuffers& c);

    unsigned program_ = 0;
    unsigned paletteUbo_ = 0;
    int locViewProj_ = -1;
    int locOriginScale_ = -1;

    unsigned fbo_ = 0, colourTex_ = 0, depthRbo_ = 0;
    int targetW_ = 0, targetH_ = 0;

    std::unordered_map<uint64_t, ChunkBuffers> chunks_;
    RendererStats stats_;
};

}  // namespace ely
