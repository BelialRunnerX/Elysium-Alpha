// Visit a planet, change it, leave, come back.
//
// The save paradigm stated as a picture. Three renders of the same piece of
// ground, from the same camera:
//
//   1. The planet as generated. Nobody has been here; there is no save file.
//   2. After cutting a doorway and a shaft into the hillside, in memory.
//   3. After writing the journal, throwing every byte of state away, and
//      regenerating the world from the seed alone with the journal replayed.
//
// The second and third must be identical. That is the whole claim: the world is
// a pure function of its seed, so what gets stored is only what somebody did to
// it, and coming back gets you the same place plus your own changes.
//
//   savedemo [seed] [outdir]
#include "gfx/tilebuild.h"
#include "gl/glcontext.h"
#include "gl/glrenderer.h"
#include "image/image.h"
#include "save/editstore.h"
#include "world/terrain.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace ely;

namespace {

constexpr double kPi = 3.14159265358979;
constexpr int kPanelW = 460, kPanelH = 420;

void blit(Image& dst, const Image& src, int x0, int y0) {
    const uint8_t* p = src.data();
    for (int y = 0; y < src.height(); ++y)
        for (int x = 0; x < src.width(); ++x) {
            const size_t i = (size_t(y) * src.width() + x) * 3;
            dst.set(x0 + x, y0 + y, p[i], p[i + 1], p[i + 2]);
        }
}

size_t fileSize(const std::string& path) {
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return 0;
    std::fseek(f, 0, SEEK_END);
    const long n = std::ftell(f);
    std::fclose(f);
    return n < 0 ? 0 : size_t(n);
}

long countDifferingPixels(const Image& a, const Image& b) {
    const uint8_t* pa = a.data();
    const uint8_t* pb = b.data();
    const size_t n = size_t(a.width()) * a.height() * 3;
    long differing = 0;
    for (size_t i = 0; i < n; i += 3)
        if (pa[i] != pb[i] || pa[i + 1] != pb[i + 1] || pa[i + 2] != pb[i + 2])
            ++differing;
    return differing;
}

}  // namespace

int main(int argc, char** argv) {
    const uint64_t seed = argc > 1 ? std::strtoull(argv[1], nullptr, 0) : 23;
    const std::string out = argc > 2 ? argv[2] : ".";
    const std::string savePath = out + "/planet_" + std::to_string(seed) + ".edits";
    std::remove(savePath.c_str());

    HeadlessGl gl;
    std::string error;
    if (!gl.create(3, 3, &error)) {
        std::printf("no OpenGL context: %s\n", error.c_str());
        return 1;
    }
    GlRenderer renderer;
    if (!renderer.init(&error) ||
        !renderer.createTarget(kPanelW, kPanelH, &error)) {
        std::printf("renderer setup failed: %s\n", error.c_str());
        return 1;
    }

    PlanetParams params = generatePlanetFromSeed(seed);
    Terrain terrain(params);
    const double mpu = params.radius * kPi / 4.0;

    // A chunk-aligned patch, so the tile and the save's chunk grid line up and
    // the demo is about saving rather than about arithmetic.
    const double uCentre = 0.0, vCentre = 0.0;
    const double surface = terrain.surfaceAltitude(FACE_PZ, uCentre, vCentre);

    TileSpec spec;
    spec.face = FACE_PZ;
    spec.uCentre = uCentre;
    spec.vCentre = vCentre;
    spec.ox = 0.0;
    spec.oz = 0.0;
    // The chunk the surface actually passes through, so the picture is of
    // ground rather than of the rock below it.
    spec.oy = std::floor(surface / double(kChunkSize)) * kChunkSize;
    spec.sizeX = spec.sizeZ = double(kChunkSize);
    spec.sizeY = double(kChunkSize);
    spec.lod = 0;

    std::printf("planet %llu (%s)\n", (unsigned long long)seed,
                planetClassName(params.cls));
    std::printf("one %d m chunk at the surface, altitude %+.0f m\n\n",
                kChunkSize, spec.oy);

    // The chunk this tile occupies, in the save's global address space.
    const ChunkAddress chunk = ChunkAddress::fromMetres(
        uint8_t(FACE_PZ), uCentre * mpu + spec.ox, spec.oy,
        vCentre * mpu + spec.oz);
    std::printf("chunk address: face %d, (%d, %d, %d)\n\n",
                int(chunk.face), chunk.x, chunk.y, chunk.z);

    // A camera looking at the patch from outside.
    // Framed on the chunk's centre from outside one corner. The angles are
    // derived from the offset rather than guessed: looking at the wrong place
    // is the most boring way for a demo to fail.
    Camera cam;
    cam.position = Vec3{16.0 - 30.0, spec.oy + 12.0 + 14.0, 16.0 - 30.0};
    cam.yaw = kPi * 1.25;
    cam.pitch = -0.318;
    cam.zNear = 0.2;
    cam.zFar = 400.0;
    const Mat4 viewProj = cam.viewProjection(double(kPanelW) / kPanelH);
    const float clear[3] = {18 / 255.0f, 20 / 255.0f, 28 / 255.0f};

    auto renderTile = [&](const EditStore* edits, Image& image) {
        TileSpec s = spec;
        s.edits = edits;
        BuiltTile built = buildTile(terrain, s);
        renderer.clearChunks();
        renderer.uploadChunk(1, built.gpu);
        renderer.render(viewProj, {1}, kPanelW, kPanelH, clear);
        return renderer.readTarget(image, &error) ? built.gpu.triangles() : 0;
    };

    // --- 1. As generated. Nobody has been here.
    Image pristine(kPanelW, kPanelH);
    const size_t trisPristine = renderTile(nullptr, pristine);
    std::printf("1. as generated          %zu triangles, save file %zu bytes\n",
                trisPristine, fileSize(savePath));

    // --- 2. Cut a doorway and a shaft, in memory.
    EditStore store;
    LoadReport report = store.load(savePath, seed, 0xE1751000ull);
    std::printf("   opening the journal:  %s\n", report.message.c_str());

    // Cut a trench across the hillside and build a pillar beside it. Both
    // directions matter: a save format that only records removal is a save
    // format that loses everything a player built.
    auto localSurface = [&](int x, int z) {
        const double u = uCentre + (spec.ox + x + 0.5) / mpu;
        const double v = vCentre + (spec.oz + z + 0.5) / mpu;
        return int(terrain.surfaceAltitude(FACE_PZ, u, v) - spec.oy);
    };

    int carved = 0, built = 0;
    for (int x = 9; x < 19; ++x) {
        for (int z = 13; z < 16; ++z) {
            const int top = localSurface(x, z);
            for (int y = top; y > top - 5 && y >= 0; --y) {
                store.setBlock(chunk, x, y, z, MAT_AIR);
                ++carved;
            }
        }
    }
    for (int x = 20; x < 23; ++x) {
        for (int z = 13; z < 16; ++z) {
            const int top = localSurface(x, z);
            for (int y = top + 1; y <= top + 4 && y < kChunkSize; ++y) {
                store.setBlock(chunk, x, y, z, MAT_ORE_AETHERIUM);
                ++built;
            }
        }
    }

    Image edited(kPanelW, kPanelH);
    const size_t trisEdited = renderTile(&store, edited);
    std::printf("2. after digging         %zu triangles, %d dug out, %d built\n",
                trisEdited, carved, built);

    // --- 3. Write it, forget everything, come back.
    if (!store.flush(&error)) {
        std::printf("flush failed: %s\n", error.c_str());
        return 1;
    }
    const size_t bytes = fileSize(savePath);

    EditStore reloaded;
    report = reloaded.load(savePath, seed, 0xE1751000ull);
    Image returned(kPanelW, kPanelH);
    const size_t trisReturned = renderTile(&reloaded, returned);

    std::printf("3. saved, reloaded       %zu triangles, %zu bytes on disk\n",
                trisReturned, bytes);
    std::printf("   replaying the journal: %s\n", report.message.c_str());

    const long changedByEdits = countDifferingPixels(pristine, edited);
    const long changedByRoundTrip = countDifferingPixels(edited, returned);
    std::printf("\n  the carving changed        %ld pixels\n", changedByEdits);
    std::printf("  the save round trip changed %ld pixels\n", changedByRoundTrip);

    const bool pass = changedByEdits > 0 && changedByRoundTrip == 0 &&
                      trisEdited == trisReturned;
    std::printf("\n  %s\n", pass
        ? "The world came back exactly as it was left."
        : "*** THE WORLD DID NOT COME BACK THE SAME ***");

    Image sheet(kPanelW * 3 + 4, kPanelH);
    sheet.fill(34, 36, 44);
    blit(sheet, pristine, 0, 0);
    blit(sheet, edited, kPanelW + 2, 0);
    blit(sheet, returned, kPanelW * 2 + 4, 0);
    sheet.writePng(out + "/save_" + std::to_string(seed) + ".png");

    std::printf("\n  %zu bytes of save for a %.0f km planet. Everything else"
                " is the seed.\n", bytes, params.radius * 2 / 1000.0);

    renderer.shutdown();
    return pass ? 0 : 1;
}
