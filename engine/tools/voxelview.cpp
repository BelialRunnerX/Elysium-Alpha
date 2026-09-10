// Generate a region of voxels, greedy-mesh it, and software-render it.
//
// This is the closest thing to "looking at the game" that exists without a
// GPU, and it is the tool that will catch a meshing bug — a hole, an inverted
// face, a mis-merged quad — long before a renderer exists to show it.
//
//   voxelview <seed> [outdir] [size] [--face N] [--u U] [--v V]
#include "mesh/greedy.h"
#include "mesh/visibility.h"
#include "image/image.h"
#include "world/chunk.h"
#include "world/terrain.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace ely;

int main(int argc, char** argv) {
    const uint64_t seed = argc > 1 ? std::strtoull(argv[1], nullptr, 0) : 12345;
    const std::string out = argc > 2 ? argv[2] : ".";
    const int N = argc > 3 ? std::atoi(argv[3]) : 96;      // region edge, metres
    int face = FACE_PZ;
    double u0 = 0.0, v0 = 0.0;
    for (int i = 4; i < argc - 1; ++i) {
        if (!std::strcmp(argv[i], "--face")) face = std::atoi(argv[i + 1]);
        if (!std::strcmp(argv[i], "--u")) u0 = std::atof(argv[i + 1]);
        if (!std::strcmp(argv[i], "--v")) v0 = std::atof(argv[i + 1]);
    }

    PlanetParams p = generatePlanetFromSeed(seed);
    Terrain t(p);
    const double metresPerUv = p.radius * 3.14159265358979 / 4.0;

    // Centre the region vertically on the local surface.
    const double centreAlt = t.surfaceAltitude(Face(face), u0, v0);
    const int baseAlt = int(centreAlt) - N / 2 + 12;

    std::printf("region %dx%dx%d m on %s planet %llu, surface %+.0f m\n",
                N, N, N, planetClassName(p.cls), (unsigned long long)seed, centreAlt);

    auto clock0 = std::chrono::steady_clock::now();
    // Column-major: climate and surface altitude are computed once per column
    // and reused down it. This is the difference between 517 ms and 2 ms per
    // chunk (see the note on Terrain::columnAt).
    std::vector<Material> vox(size_t(N) * N * N);
    for (int z = 0; z < N; ++z)
        for (int x = 0; x < N; ++x) {
            const double u = u0 + (x - N * 0.5) / metresPerUv;
            const double v = v0 + (z - N * 0.5) / metresPerUv;
            const Terrain::Column col = t.columnAt(Face(face), u, v);
            for (int y = 0; y < N; ++y)
                vox[(size_t(y) * N + z) * N + x] =
                    t.materialInColumn(col, baseAlt + y);
        }
    auto clock1 = std::chrono::steady_clock::now();

    auto sample = [&](int x, int y, int z) -> Material {
        if (x < 0 || y < 0 || z < 0 || x >= N || y >= N || z >= N) return MAT_AIR;
        return vox[(size_t(y) * N + z) * N + x];
    };

    // Pack it into chunks too, so the storage path is exercised and measured.
    size_t chunkBytes = 0, homogeneous = 0, chunks = 0;
    for (int cy = 0; cy + kChunkSize <= N; cy += kChunkSize)
    for (int cz = 0; cz + kChunkSize <= N; cz += kChunkSize)
    for (int cx = 0; cx + kChunkSize <= N; cx += kChunkSize) {
        Chunk c;
        c.fill([&](int x, int y, int z) { return sample(cx + x, cy + y, cz + z); });
        chunkBytes += c.bytes();
        if (c.homogeneous()) ++homogeneous;
        ++chunks;
    }

    auto clock2 = std::chrono::steady_clock::now();
    // Exterior-air connectivity: any cavity with no path out of the region can
    // never be looked into, so its walls are not meshed at all.
    VisibilityMask vis(N, N, N, sample);
    MeshOptions opt;
    opt.visibility = &vis;
    MeshStats st;
    Mesh mesh = greedyMesh(N, N, N, sample, opt, &st);
    auto clock3 = std::chrono::steady_clock::now();

    const double genMs = std::chrono::duration<double, std::milli>(clock1 - clock0).count();
    const double packMs = std::chrono::duration<double, std::milli>(clock2 - clock1).count();
    const double meshMs = std::chrono::duration<double, std::milli>(clock3 - clock2).count();
    const long voxels = long(N) * N * N;
    const long chunkCount = long(voxels / (kChunkSize * kChunkSize * kChunkSize));

    std::printf("  generate  %8.1f ms  (%.3f ms per 32^3 chunk)\n",
                genMs, genMs / double(chunkCount));
    std::printf("  pack      %8.1f ms  %zu chunks, %zu homogeneous (%.0f%%), %.1f KB\n",
                packMs, chunks, homogeneous,
                100.0 * homogeneous / double(chunks ? chunks : 1),
                chunkBytes / 1024.0);
    std::printf("  mesh      %8.1f ms  %zu quads, %zu vertices\n",
                meshMs, mesh.quads(), mesh.vertices.size());
    // What each stage of culling and merging actually bought.
    std::printf("  cull      %ld air voxels, %ld sealed off (%.1f%% unreachable)\n",
                vis.airVoxels(), vis.sealedAir(),
                100.0 * vis.sealedAir() / double(vis.airVoxels() ? vis.airVoxels() : 1));
    std::printf("  faces     %ld exposed -> %ld after sealed-cavity cull -> %ld quads"
                "  (%.1fx)\n",
                st.exposedFaces, st.emittedFaces, st.quads, st.faceReduction());

    // --- Render it. Isometric-ish camera, depth buffered, AO applied.
    const int RW = 900, RH = 620;
    Raster raster(RW, RH);
    raster.clear(16, 17, 22);

    const double yaw = 0.7853981634, pitch = 0.5235987756;   // 45deg, 30deg
    const double cy = std::cos(yaw), sy = std::sin(yaw);
    const double cp = std::cos(pitch), sp = std::sin(pitch);
    const double scale = RH * 0.62 / double(N);

    auto project = [&](float vx, float vy, float vz, float outp[3]) {
        const double x = vx - N * 0.5, y = vy - N * 0.5, z = vz - N * 0.5;
        const double rx = x * cy + z * sy;
        const double rz = -x * sy + z * cy;
        const double ry = y * cp - rz * sp;
        const double depth = y * sp + rz * cp;
        outp[0] = float(RW * 0.5 + rx * scale);
        outp[1] = float(RH * 0.55 - ry * scale);
        outp[2] = float(depth);
    };

    // Face shading: the six axes get distinct multipliers so form reads even
    // with flat materials. Same reasoning as the first edition's previewer.
    const float faceLight[6] = {0.86f, 0.74f, 1.00f, 0.55f, 0.92f, 0.68f};

    long drawn = 0;
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        const Vertex& a = mesh.vertices[mesh.indices[i]];
        const Vertex& b = mesh.vertices[mesh.indices[i + 1]];
        const Vertex& c = mesh.vertices[mesh.indices[i + 2]];
        float pa[3], pb[3], pc[3];
        project(a.x, a.y, a.z, pa);
        project(b.x, b.y, b.z, pb);
        project(c.x, c.y, c.z, pc);
        auto shade = [&](const Vertex& v, uint8_t o[3]) {
            const float l = faceLight[v.nx] * (0.52f + 0.16f * float(v.ao));
            const MaterialInfo& mi = materialInfo(Material(v.mat));
            o[0] = uint8_t(clampf(mi.r * l, 0, 255));
            o[1] = uint8_t(clampf(mi.g * l, 0, 255));
            o[2] = uint8_t(clampf(mi.b * l, 0, 255));
        };
        uint8_t ca[3], cb[3], cc[3];
        shade(a, ca); shade(b, cb); shade(c, cc);
        raster.triangle(pa, pb, pc, ca, cb, cc);
        ++drawn;
    }
    std::printf("  render    %ld triangles\n", drawn);

    raster.image().writePng(out + "/voxel_" + std::to_string(seed) + ".png");
    return 0;
}
