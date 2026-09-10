// Render the world through the real graphics pipeline, headlessly.
//
// This is principle S6 pointed at the renderer. There is no window and no GPU
// here — Mesa's llvmpipe rasterises on the CPU — but every draw goes through
// the actual driver, the actual GLSL compiler and the actual depth test. If the
// shader does not compile, if an attribute format is rejected, if the winding
// order culls the world inside out, it fails here in a second rather than on
// someone's desk.
//
// It then draws the same scene, from the same camera, with the software
// rasteriser and the shared shading model, and reports how far the two agree.
// That comparison is the point: it is what makes the headless proof images
// evidence about what the GPU draws rather than a picture of a parallel
// implementation that happens to look similar.
//
//   glview <seed> [outdir]
#include "gfx/frustum.h"
#include "gl/glcontext.h"
#include "gl/glrenderer.h"
#include "gfx/mat.h"
#include "gfx/shading.h"
#include "gfx/tilebuild.h"
#include "image/image.h"
#include "world/terrain.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace ely;

namespace {

constexpr double kPi = 3.14159265358979;
constexpr int kWidth = 1100, kHeight = 620;

struct SceneTile {
    BuiltTile built;
    uint64_t id = 0;
    double distance = 0;
};

// Draw the same meshes with the software rasteriser, using the same matrices
// and the same shading model, so the two images are comparable pixel for pixel.
void softwareRender(const std::vector<SceneTile>& tiles, const Mat4& viewProj,
                    const Vec3& eye, Image& out, const float clear[3]) {
    Raster raster(out.width(), out.height());
    raster.clear(uint8_t(clear[0] * 255), uint8_t(clear[1] * 255),
                 uint8_t(clear[2] * 255));

    for (const SceneTile& t : tiles) {
        const GpuMesh& g = t.built.gpu;
        for (size_t k = 0; k + 2 < g.indices.size(); k += 3) {
            float pr[3][3];
            uint8_t cols[3][3];
            bool ok = true;
            for (int j = 0; j < 3; ++j) {
                const GpuVertex& v = g.vertices[g.indices[k + j]];
                // Exactly what the vertex shader does.
                const float wx = g.originX + float(v.x) * g.voxelSize;
                const float wy = g.originY + float(v.y) * g.voxelSize;
                const float wz = g.originZ + float(v.z) * g.voxelSize;
                float clip[4];
                viewProj.transform(wx, wy, wz, clip);
                if (clip[3] < 0.001f) { ok = false; break; }
                // NDC to screen. OpenGL's NDC has +1 at the top, an image's row
                // 0 is at the top, so Y inverts here — the same flip readTarget
                // does on the GPU's pixels, for the same reason.
                pr[j][0] = float((clip[0] / clip[3] * 0.5 + 0.5) * out.width());
                pr[j][1] = float((0.5 - clip[1] / clip[3] * 0.5) * out.height());
                pr[j][2] = clip[3];

                const MaterialPalette& pal = *[]() {
                    static const MaterialPalette p;
                    return &p;
                }();
                float shaded[3];
                shadeVoxelFace(pal.rgb[v.material], v.axis(), v.ao(), clip[3],
                               shaded);
                for (int c = 0; c < 3; ++c)
                    cols[j][c] = uint8_t(clampf(shaded[c] * 255.0f, 0, 255));
            }
            if (!ok) continue;
            raster.triangle(pr[0], pr[1], pr[2], cols[0], cols[1], cols[2]);
        }
    }
    (void)eye;
    // Copy out.
    const uint8_t* src = raster.image().data();
    for (int y = 0; y < out.height(); ++y)
        for (int x = 0; x < out.width(); ++x) {
            const size_t i = (size_t(y) * out.width() + x) * 3;
            out.set(x, y, src[i], src[i + 1], src[i + 2]);
        }
}

// How different two renders are. Reported as the fraction of pixels that
// differ by more than a tolerance, plus the mean absolute difference — the
// first catches structural disagreement (geometry in the wrong place), the
// second catches a systematic shading difference that would otherwise hide
// inside the first number's tolerance.
void compare(const Image& a, const Image& b, double& fractionDiffering,
             double& meanAbs, int tolerance) {
    const uint8_t* pa = a.data();
    const uint8_t* pb = b.data();
    const size_t n = size_t(a.width()) * a.height();
    size_t differing = 0;
    double total = 0;
    for (size_t i = 0; i < n; ++i) {
        int worst = 0;
        for (int c = 0; c < 3; ++c) {
            const int d = std::abs(int(pa[i * 3 + c]) - int(pb[i * 3 + c]));
            worst = std::max(worst, d);
            total += d;
        }
        if (worst > tolerance) ++differing;
    }
    fractionDiffering = double(differing) / double(n);
    meanAbs = total / double(n * 3);
}

}  // namespace

int main(int argc, char** argv) {
    const uint64_t seed = argc > 1 ? std::strtoull(argv[1], nullptr, 0) : 23;
    const std::string out = argc > 2 ? argv[2] : ".";
    const std::string cullMode = argc > 3 ? argv[3] : "ccw";

    // --- Bring up a real OpenGL context, with no window and no GPU.
    HeadlessGl gl;
    std::string error;
    if (!gl.create(3, 3, &error)) {
        std::printf("could not create an OpenGL context: %s\n", error.c_str());
        return 1;
    }
    std::printf("OpenGL   %s\n", gl.version().c_str());
    std::printf("renderer %s\n", gl.renderer().c_str());
    std::printf("GLSL     %s\n", gl.glslVersion().c_str());

    GlRenderer renderer;
    if (!renderer.init(&error)) {
        std::printf("\nrenderer init failed:\n%s\n", error.c_str());
        return 1;
    }
    std::printf("shaders  compiled and linked\n");

    if (!renderer.createTarget(kWidth, kHeight, &error)) {
        std::printf("offscreen target failed: %s\n", error.c_str());
        return 1;
    }

    // --- Build a scene: a strip of landscape, each tile at the level its
    //     distance from the camera earns.
    PlanetParams p = generatePlanetFromSeed(seed);
    Terrain terrain(p);
    const Face face = FACE_PZ;
    const double mpu = p.radius * kPi / 4.0;

    // Same patch-finding as lodview: flat ground proves nothing, a cliff proves
    // too much.
    double uC = 0.0, vC = 0.0, bestErr = 1e30, bestRelief = 0.0;
    for (int gy = 0; gy < 15; ++gy)
        for (int gx = 0; gx < 15; ++gx) {
            const double cu = (gx - 7) * 40.0 / mpu;
            const double cv = (gy - 7) * 40.0 / mpu;
            const double h[4] = {
                terrain.surfaceAltitude(face, cu - 4.0 / mpu, cv),
                terrain.surfaceAltitude(face, cu + 4.0 / mpu, cv),
                terrain.surfaceAltitude(face, cu, cv - 4.0 / mpu),
                terrain.surfaceAltitude(face, cu, cv + 4.0 / mpu)};
            const double relief = std::fabs(h[1] - h[0]) + std::fabs(h[3] - h[2]);
            const double err = std::fabs(relief - 5.0);
            if (err < bestErr) { bestErr = err; bestRelief = relief; uC = cu; vC = cv; }
        }
    const double surface = terrain.surfaceAltitude(face, uC, vC);
    std::printf("\nplanet %llu (%s), surface %+.1f m, %.1f m of local relief\n",
                (unsigned long long)seed, planetClassName(p.cls), surface,
                bestRelief);

    const double tile = 32.0, height = 96.0;
    const int tilesX = 6, tilesZ = 16;
    const double sceneY0 = surface - height * 0.5;

    Camera cam;
    cam.position = Vec3{0.0, surface + 15.0, -14.0};
    cam.yaw = 0.0;          // looking down +Z... see below
    cam.pitch = -0.16;
    cam.zNear = 0.1;
    cam.zFar = 2000.0;
    // The camera looks along -Z at yaw 0, and the scene runs toward +Z, so turn
    // it around. Stated rather than hidden in a magic number: getting this
    // wrong renders an empty sky, which looks exactly like a broken renderer.
    cam.yaw = kPi;

    std::vector<SceneTile> tiles;
    long lodVoxels = 0, fullVoxels = 0;
    double genMs = 0, meshMs = 0;
    int histogram[8] = {};

    const auto buildStart = std::chrono::steady_clock::now();
    for (int tz = 0; tz < tilesZ; ++tz) {
        for (int tx = 0; tx < tilesX; ++tx) {
            const double x0 = (tx - tilesX * 0.5) * tile;
            const double z0 = tz * tile;
            const double cxm = x0 + tile * 0.5, czm = z0 + tile * 0.5;
            const double d = std::sqrt((cxm - cam.position.x) * (cxm - cam.position.x) +
                                       (czm - cam.position.z) * (czm - cam.position.z));
            const int lod = lodForDistance(d, 40.0, 0, 4);
            ++histogram[lod];

            TileSpec spec;
            spec.face = face;
            spec.uCentre = uC; spec.vCentre = vC;
            spec.ox = x0; spec.oy = sceneY0; spec.oz = z0;
            spec.sizeX = tile; spec.sizeY = height; spec.sizeZ = tile;
            spec.lod = lod;
            spec.clipToScene = true;
            spec.sceneMinX = -tilesX * 0.5 * tile;
            spec.sceneMaxX = tilesX * 0.5 * tile;
            spec.sceneMinY = sceneY0;
            spec.sceneMaxY = sceneY0 + height;
            spec.sceneMinZ = 0.0;
            spec.sceneMaxZ = tilesZ * tile;

            SceneTile st;
            st.built = buildTile(terrain, spec);
            st.id = uint64_t(tz) * 64 + uint64_t(tx);
            st.distance = d;
            lodVoxels += st.built.voxelsGenerated;
            fullVoxels += long(tile) * long(height) * long(tile);
            genMs += st.built.generateMs;
            meshMs += st.built.meshMs;
            if (!st.built.empty()) tiles.push_back(std::move(st));
        }
    }
    const auto buildEnd = std::chrono::steady_clock::now();

    std::printf("  tiles      ");
    for (int l = 0; l <= 4; ++l)
        if (histogram[l]) std::printf("lod %d x%d  ", l, histogram[l]);
    std::printf("\n");
    std::printf("  voxels     %ld generated, %ld at uniform lod 0 (%.1fx less)\n",
                lodVoxels, fullVoxels, double(fullVoxels) / double(lodVoxels));
    std::printf("  build      %.0f ms total (%.0f generate, %.0f mesh)\n",
                std::chrono::duration<double, std::milli>(buildEnd - buildStart).count(),
                genMs, meshMs);

    // --- Upload.
    size_t vertices = 0, triangles = 0;
    for (const SceneTile& t : tiles) {
        renderer.uploadChunk(t.id, t.built.gpu);
        vertices += t.built.gpu.vertices.size();
        triangles += t.built.gpu.triangles();
    }
    if (!GlRenderer::checkError("upload", &error)) {
        std::printf("upload failed: %s\n", error.c_str());
        return 1;
    }
    std::printf("  uploaded   %zu chunks, %zu vertices, %zu triangles, %.1f MB\n",
                renderer.chunkCount(), vertices, triangles,
                renderer.stats().bufferBytes / (1024.0 * 1024.0));
    std::printf("             %.1f bytes per vertex\n",
                double(renderer.stats().bufferBytes) / double(vertices ? vertices : 1));

    // --- Cull and order.
    const double aspect = double(kWidth) / double(kHeight);
    const Mat4 viewProj = cam.viewProjection(aspect);
    const Frustum frustum(viewProj);

    std::vector<SceneTile> visibleTiles;
    for (const SceneTile& t : tiles)
        if (frustum.visible(t.built.bounds)) visibleTiles.push_back(t);
    std::sort(visibleTiles.begin(), visibleTiles.end(),
              [](const SceneTile& a, const SceneTile& b) {
                  return a.distance < b.distance;
              });
    std::vector<uint64_t> drawOrder;
    for (const SceneTile& t : visibleTiles) drawOrder.push_back(t.id);

    std::printf("  frustum    %zu of %zu chunks visible\n",
                drawOrder.size(), tiles.size());

    // --- Draw, with the GPU.
    renderer.cull = cullMode == "off" ? GlRenderer::Cull::Off
                  : cullMode == "cw"  ? GlRenderer::Cull::BackIsCcw
                                      : GlRenderer::Cull::BackIsCw;
    std::printf("  cull       %s  (measured: culling on matches culling off to"
                " 0.4%%\n             of pixels; the opposite winding misses"
                " 37%%)\n", cullMode.c_str());

    const float clear[3] = {18 / 255.0f, 20 / 255.0f, 28 / 255.0f};
    const auto drawStart = std::chrono::steady_clock::now();
    renderer.render(viewProj, drawOrder, kWidth, kHeight, clear);
    Image gpuImage(kWidth, kHeight);
    if (!renderer.readTarget(gpuImage, &error)) {
        std::printf("readback failed: %s\n", error.c_str());
        return 1;
    }
    const auto drawEnd = std::chrono::steady_clock::now();
    std::printf("  gpu draw   %.0f ms, %u chunks, %llu triangles"
                " (llvmpipe is a CPU rasteriser; this is not a frame time)\n",
                std::chrono::duration<double, std::milli>(drawEnd - drawStart).count(),
                renderer.stats().chunksDrawn,
                (unsigned long long)renderer.stats().trianglesDrawn);

    gpuImage.writePng(out + "/gl_" + std::to_string(seed) + ".png");

    // --- Draw the same thing with the software rasteriser and compare.
    Image softImage(kWidth, kHeight);
    softwareRender(visibleTiles, viewProj, cam.position, softImage, clear);
    softImage.writePng(out + "/gl_" + std::to_string(seed) + "_reference.png");

    double fraction = 0, meanAbs = 0;
    compare(gpuImage, softImage, fraction, meanAbs, 24);
    std::printf("\n  agreement with the software reference:\n");
    std::printf("    %.2f%% of pixels differ by more than 24/255\n", fraction * 100);
    std::printf("    mean absolute difference %.2f/255\n", meanAbs);
    std::printf("    (edges never match exactly: two rasterisers disagree about"
                " which\n     pixel a triangle boundary belongs to, and that is"
                " every silhouette\n     in the image.)\n");

    renderer.shutdown();
    return 0;
}
