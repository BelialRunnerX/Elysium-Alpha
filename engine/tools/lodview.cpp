// Level of detail, micro-voxels and visibility culling, made visible.
//
// Three proofs, all headless (principle S6):
//
//   1. The resolution ladder. The same eight cubic metres of ground, generated
//      at 1/16 m, 1/8, 1/4, 1/2 and 1 m. Same code, same generator, one
//      constant different. This is what "a block has 16^3 subunits" looks like.
//
//   2. Distance LOD. A half-kilometre strip meshed tile by tile, each tile at
//      the level its distance from the camera earns: one step coarser per
//      doubling of distance, exactly as asked. Reports how many voxels that
//      saved against meshing the whole strip at full resolution.
//
//   3. Culling. For every region: how many faces exist, how many bound sealed
//      cavities nobody can look into, how many point away from the camera, and
//      how many survive to be merged into quads.
//
//   lodview <seed> [outdir]
#include "mesh/greedy.h"
#include "mesh/visibility.h"
#include "image/image.h"
#include "world/lod.h"
#include "world/terrain.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace ely;

namespace {

constexpr double kPi = 3.14159265358979;

// A generated block of voxels at one resolution, positioned in a local metre
// frame so that regions at different levels can be meshed into one scene.
struct Region {
    int nx = 0, ny = 0, nz = 0;    // logical size, in voxels
    int pad = 0;                   // apron thickness, in voxels
    double size = 1.0;             // voxel edge, metres
    double ox = 0, oy = 0, oz = 0; // metres of voxel (0,0,0) in the local frame
    std::vector<Material> vox;

    int ex() const { return nx + 2 * pad; }
    int ey() const { return ny + 2 * pad; }
    int ez() const { return nz + 2 * pad; }

    // Logical coordinates. With an apron, indices -1 and n are real generated
    // world rather than air, so the mesher does not wall the region in.
    Material at(int x, int y, int z) const {
        const int ax = x + pad, ay = y + pad, az = z + pad;
        if (ax < 0 || ay < 0 || az < 0 || ax >= ex() || ay >= ey() || az >= ez())
            return MAT_AIR;
        return vox[(size_t(ay) * ez() + az) * ex() + ax];
    }
    long voxels() const { return long(nx) * ny * nz; }
};

// Generate a region by walking columns: climate and surface altitude are
// computed once per column and reused down it, which is the difference between
// half a second and twenty milliseconds per chunk.
//
// `pad` generates an apron of that many voxels beyond every face. Without one,
// a tile meshed in isolation treats everything outside itself as air and emits
// a wall around all six sides — which on a tiled landscape shows up as a solid
// seam standing along every tile boundary, plus two coincident walls z-fighting
// between each pair of neighbours. One voxel of apron is the whole fix: the
// mesher asks about the neighbour's first voxel, finds rock, and emits nothing.
Region generate(const Terrain& t, Face face, double uC, double vC,
                double ox, double oy, double oz,
                int nx, int ny, int nz, double size, int pad = 0) {
    Region r;
    r.nx = nx; r.ny = ny; r.nz = nz; r.pad = pad; r.size = size;
    r.ox = ox; r.oy = oy; r.oz = oz;
    r.vox.assign(size_t(r.ex()) * r.ey() * r.ez(), MAT_AIR);
    const double mpu = t.planet().radius * kPi / 4.0;
    for (int az = 0; az < r.ez(); ++az)
        for (int ax = 0; ax < r.ex(); ++ax) {
            const double u = uC + (ox + (ax - pad + 0.5) * size) / mpu;
            const double v = vC + (oz + (az - pad + 0.5) * size) / mpu;
            const Terrain::Column col = t.columnAt(face, u, v);
            for (int ay = 0; ay < r.ey(); ++ay)
                r.vox[(size_t(ay) * r.ez() + az) * r.ex() + ax] =
                    t.materialInColumn(col, oy + (ay - pad + 0.5) * size, size);
        }
    return r;
}

void blit(Image& dst, const Image& src, int x0, int y0) {
    const uint8_t* p = src.data();
    for (int y = 0; y < src.height(); ++y)
        for (int x = 0; x < src.width(); ++x) {
            const size_t i = (size_t(y) * src.width() + x) * 3;
            dst.set(x0 + x, y0 + y, p[i], p[i + 1], p[i + 2]);
        }
}

const float kFaceLight[6] = {0.86f, 0.74f, 1.00f, 0.55f, 0.92f, 0.68f};

void shadeVertex(const Vertex& v, uint8_t out[3]) {
    const float l = kFaceLight[v.nx] * (0.52f + 0.16f * float(v.ao));
    const MaterialInfo& mi = materialInfo(Material(v.mat));
    out[0] = uint8_t(clampf(mi.r * l, 0, 255));
    out[1] = uint8_t(clampf(mi.g * l, 0, 255));
    out[2] = uint8_t(clampf(mi.b * l, 0, 255));
}

// A tiny 5x7 bitmap font, so the panels can label themselves. A picture that
// needs a caption in another file is a picture that will be misread.
void drawText(Image& img, int x0, int y0, const std::string& s, int scale,
              uint8_t r, uint8_t g, uint8_t b);

}  // namespace

// ---------------------------------------------------------------------------

namespace {

// 5x7 font, enough for the labels this tool writes.
struct Glyph { char c; const char* rows[7]; };
const Glyph kFont[] = {
 {'0',{"01110","10001","10011","10101","11001","10001","01110"}},
 {'1',{"00100","01100","00100","00100","00100","00100","01110"}},
 {'2',{"01110","10001","00001","00010","00100","01000","11111"}},
 {'3',{"11111","00010","00100","00010","00001","10001","01110"}},
 {'4',{"00010","00110","01010","10010","11111","00010","00010"}},
 {'5',{"11111","10000","11110","00001","00001","10001","01110"}},
 {'6',{"00110","01000","10000","11110","10001","10001","01110"}},
 {'7',{"11111","00001","00010","00100","01000","01000","01000"}},
 {'8',{"01110","10001","10001","01110","10001","10001","01110"}},
 {'9',{"01110","10001","10001","01111","00001","00010","01100"}},
 {'-',{"00000","00000","00000","11111","00000","00000","00000"}},
 {'/',{"00001","00010","00010","00100","01000","01000","10000"}},
 {'.',{"00000","00000","00000","00000","00000","01100","01100"}},
 {' ',{"00000","00000","00000","00000","00000","00000","00000"}},
 {'m',{"00000","00000","11010","10101","10101","10101","10101"}},
 {'l',{"01100","00100","00100","00100","00100","00100","01110"}},
 {'o',{"00000","00000","01110","10001","10001","10001","01110"}},
 {'d',{"00001","00001","01111","10001","10001","10001","01111"}},
 {'v',{"00000","00000","10001","10001","10001","01010","00100"}},
 {'x',{"00000","00000","10001","01010","00100","01010","10001"}},
 {'=',{"00000","00000","11111","00000","11111","00000","00000"}},
 {'k',{"10000","10000","10010","10100","11000","10100","10010"}},
 {'q',{"00000","00000","01111","10001","01111","00001","00001"}},
 {'u',{"00000","00000","10001","10001","10001","10011","01101"}},
 {'a',{"00000","00000","01110","00001","01111","10001","01111"}},
 {'s',{"00000","00000","01111","10000","01110","00001","11110"}},
};

void drawText(Image& img, int x0, int y0, const std::string& s, int scale,
              uint8_t r, uint8_t g, uint8_t b) {
    int pen = x0;
    for (char ch : s) {
        const Glyph* gl = nullptr;
        for (const Glyph& c : kFont) if (c.c == ch) { gl = &c; break; }
        if (gl) {
            for (int row = 0; row < 7; ++row)
                for (int col = 0; col < 5; ++col)
                    if (gl->rows[row][col] == '1')
                        for (int sy = 0; sy < scale; ++sy)
                            for (int sx = 0; sx < scale; ++sx)
                                img.set(pen + col * scale + sx,
                                        y0 + row * scale + sy, r, g, b);
        }
        pen += 6 * scale;
    }
}

}  // namespace

int main(int argc, char** argv) {
    const uint64_t seed = argc > 1 ? std::strtoull(argv[1], nullptr, 0) : 12345;
    const std::string out = argc > 2 ? argv[2] : ".";

    PlanetParams p = generatePlanetFromSeed(seed);
    Terrain t(p);
    const Face face = FACE_PZ;

    // Find ground worth photographing. A flat patch shows nothing — every
    // level of the ladder renders the same slab and the comparison proves
    // nothing — and a cliff shows too much, because a wall that crosses the
    // whole box is not what resolution is for. Search a grid of candidates for
    // the one whose relief across an eight-metre window is closest to a good
    // walking slope.
    const double mpu = p.radius * kPi / 4.0;
    const double wanted = 5.0;
    double uC = 0.0, vC = 0.0, bestRelief = 0.0, bestErr = 1e30;
    for (int gy = 0; gy < 15; ++gy) {
        for (int gx = 0; gx < 15; ++gx) {
            const double cu = (gx - 7) * 40.0 / mpu;
            const double cv = (gy - 7) * 40.0 / mpu;
            const double h[4] = {
                t.surfaceAltitude(face, cu - 4.0 / mpu, cv),
                t.surfaceAltitude(face, cu + 4.0 / mpu, cv),
                t.surfaceAltitude(face, cu, cv - 4.0 / mpu),
                t.surfaceAltitude(face, cu, cv + 4.0 / mpu)};
            const double relief = std::fabs(h[1] - h[0]) + std::fabs(h[3] - h[2]);
            const double err = std::fabs(relief - wanted);
            if (err < bestErr) { bestErr = err; bestRelief = relief; uC = cu; vC = cv; }
        }
    }
    const double surface = t.surfaceAltitude(face, uC, vC);

    std::printf("planet %llu  (%s)  surface %+.1f m,  %.1f m of relief across"
                " the 8 m patch\n",
                (unsigned long long)seed, planetClassName(p.cls), surface,
                bestRelief);
    std::printf("\nresolution ladder: one block = %d^3 = %d subunits of %.4f m\n",
                kMicro, kMicroVolume, kMicroSize);

    // -----------------------------------------------------------------------
    // 1. The ladder. Eight cubic metres, five resolutions.
    // -----------------------------------------------------------------------
    const double extent = 8.0;                  // metres per side
    const int lods[5] = {kFinestLod, -3, -2, -1, 0};
    const int PW = 300, PH = 340;
    Image ladder(PW * 5, PH + 44);
    ladder.fill(14, 15, 20);

    std::printf("\n  lod   voxel      grid        voxels    faces   sealed  backface"
                "    quads   ratio\n");
    for (int i = 0; i < 5; ++i) {
        const int lod = lods[i];
        const double vsize = lodVoxelSize(lod);
        const int n = int(extent / vsize + 0.5);
        Region r = generate(t, face, uC, vC,
                            -extent * 0.5, surface - extent * 0.5, -extent * 0.5,
                            n, n, n, vsize);

        auto samp = [&](int x, int y, int z) { return r.at(x, y, z); };
        VisibilityMask vis(n, n, n, samp);

        MeshOptions opt;
        opt.voxelSize = vsize;
        opt.ox = r.ox; opt.oy = -extent * 0.5; opt.oz = r.oz;
        opt.visibility = &vis;
        MeshStats st;
        Mesh mesh = greedyMesh(n, n, n, samp, opt, &st);
        st.solidVoxels = vis.solidVoxels();

        std::printf("  %+3d  %7.4f  %4d^3  %11ld  %7ld  %7ld  %8ld  %7ld  %5.1fx\n",
                    lod, vsize, n, r.voxels(), st.exposedFaces, st.sealedCulled,
                    st.backfaceCulled, st.quads, st.faceReduction());

        // Orthographic isometric, identical framing at every level so the
        // panels are directly comparable.
        Raster raster(PW, PH);
        raster.clear(14, 15, 20);
        const double yaw = 0.7853981634, pitch = 0.5235987756;
        const double cy = std::cos(yaw), sy = std::sin(yaw);
        const double cp = std::cos(pitch), sp = std::sin(pitch);
        const double scale = PH * 0.42 / extent;
        auto project = [&](float vx, float vy, float vz, float o[3]) {
            const double x = vx, y = vy, z = vz;
            const double rx = x * cy + z * sy;
            const double rz = -x * sy + z * cy;
            const double ry = y * cp - rz * sp;
            o[0] = float(PW * 0.5 + rx * scale);
            o[1] = float(PH * 0.50 - ry * scale);
            o[2] = float(y * sp + rz * cp);
        };
        for (size_t k = 0; k + 2 < mesh.indices.size(); k += 3) {
            const Vertex& a = mesh.vertices[mesh.indices[k]];
            const Vertex& b = mesh.vertices[mesh.indices[k + 1]];
            const Vertex& c = mesh.vertices[mesh.indices[k + 2]];
            float pa[3], pb[3], pc[3];
            project(a.x, a.y, a.z, pa);
            project(b.x, b.y, b.z, pb);
            project(c.x, c.y, c.z, pc);
            uint8_t ca[3], cb[3], cc[3];
            shadeVertex(a, ca); shadeVertex(b, cb); shadeVertex(c, cc);
            raster.triangle(pa, pb, pc, ca, cb, cc);
        }
        blit(ladder, raster.image(), PW * i, 26);

        const char* sizeText = vsize < 0.1 ? "1/16" : vsize < 0.2 ? "1/8"
                             : vsize < 0.4 ? "1/4"  : vsize < 0.7 ? "1/2" : "1";
        char label[64];
        std::snprintf(label, sizeof(label), "lod %d  %s m", lod, sizeText);
        drawText(ladder, PW * i + 14, 7, label, 2, 214, 220, 232);
        char sub[64];
        std::snprintf(sub, sizeof(sub), "%dx%dx%d  %ld quads", n, n, n, st.quads);
        drawText(ladder, PW * i + 14, PH + 10, sub, 1, 132, 140, 158);
        for (int yy = 0; yy < PH + 26; ++yy) ladder.set(PW * i, yy, 34, 36, 44);
    }
    ladder.writePng(out + "/lod_ladder_" + std::to_string(seed) + ".png");

    // -----------------------------------------------------------------------
    // 2. Distance LOD across a strip of landscape.
    // -----------------------------------------------------------------------
    const double tile = 32.0;          // metres per tile
    const int tilesZ = 16, tilesX = 6; // 512 m deep, 192 m wide
    const double height = 96.0;        // vertical extent, metres
    const double camY = surface + 15.0, camZ = -14.0, camX = 0.0;

    std::printf("\ndistance LOD: %d x %d tiles of %.0f m, camera at z = %.0f m\n",
                tilesX, tilesZ, tile, camZ);

    std::vector<Mesh> meshes;
    MeshStats total;
    long lodVoxels = 0, fullVoxels = 0;
    int lodHistogram[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    auto tclock0 = std::chrono::steady_clock::now();

    for (int tz = 0; tz < tilesZ; ++tz) {
        for (int tx = 0; tx < tilesX; ++tx) {
            const double x0 = (tx - tilesX * 0.5) * tile;
            const double z0 = tz * tile;
            const double cxm = x0 + tile * 0.5, czm = z0 + tile * 0.5;
            const double dist = std::sqrt((cxm - camX) * (cxm - camX) +
                                          (czm - camZ) * (czm - camZ));
            // One level coarser per doubling of distance. Level 0 (one metre)
            // is the finest used out here: micro-voxels are for arm's reach,
            // and this proves the rule over the range that matters for a view
            // distance, not for a fingertip.
            const int lod = lodForDistance(dist, 40.0, 0, 4);
            const double vsize = lodVoxelSize(lod);
            const int n = int(tile / vsize + 0.5);
            const int ny = int(height / vsize + 0.5);
            ++lodHistogram[lod];

            Region r = generate(t, face, uC, vC, x0, surface - height * 0.5, z0,
                                n, ny, n, vsize, 1);
            lodVoxels += r.voxels();
            fullVoxels += long(tile) * long(height) * long(tile);

            // Inside the scene a tile joins its neighbour through the apron.
            // At the scene's own outer boundary there is no neighbour, so the
            // apron is suppressed and the tile closes itself off — otherwise
            // the outermost tiles would be meshed as if the terrain continued
            // and the viewer would look straight into the inside of the
            // ground through a wall that was never drawn.
            const double sceneX0 = -tilesX * 0.5 * tile, sceneX1 = -sceneX0;
            const double sceneZ0 = 0.0, sceneZ1 = tilesZ * tile;
            const double sceneY0 = surface - height * 0.5, sceneY1 = sceneY0 + height;
            auto samp = [&, x0, z0, vsize](int x, int y, int z) -> Material {
                const double wx = x0 + (x + 0.5) * vsize;
                const double wz = z0 + (z + 0.5) * vsize;
                const double wy = sceneY0 + (y + 0.5) * vsize;
                if (wx < sceneX0 || wx > sceneX1 || wz < sceneZ0 || wz > sceneZ1 ||
                    wy < sceneY0 || wy > sceneY1)
                    return MAT_AIR;
                return r.at(x, y, z);
            };
            VisibilityMask vis(n, ny, n, samp);

            MeshOptions opt;
            opt.voxelSize = vsize;
            opt.ox = x0; opt.oy = surface - height * 0.5; opt.oz = z0;
            opt.visibility = &vis;
            opt.cullBackFaces = true;
            opt.viewX = 0.0; opt.viewY = -0.35; opt.viewZ = 1.0;
            MeshStats st;
            meshes.push_back(greedyMesh(n, ny, n, samp, opt, &st));
            total.exposedFaces += st.exposedFaces;
            total.sealedCulled += st.sealedCulled;
            total.backfaceCulled += st.backfaceCulled;
            total.emittedFaces += st.emittedFaces;
            total.quads += st.quads;
            total.solidVoxels += vis.solidVoxels();
        }
    }
    auto tclock1 = std::chrono::steady_clock::now();
    const double buildMs =
        std::chrono::duration<double, std::milli>(tclock1 - tclock0).count();

    std::printf("  tiles per level: ");
    for (int l = 0; l <= 4; ++l)
        if (lodHistogram[l]) std::printf("lod %d x%d  ", l, lodHistogram[l]);
    std::printf("\n");
    std::printf("  voxels generated %11ld   at uniform lod 0 it would be %11ld"
                "   (%.1fx less)\n",
                lodVoxels, fullVoxels, double(fullVoxels) / double(lodVoxels));
    std::printf("  build            %8.0f ms for %d tiles\n", buildMs,
                tilesX * tilesZ);
    std::printf("  faces  exposed %9ld\n", total.exposedFaces);
    std::printf("         sealed  %9ld  (%.1f%%, cavities with no path out)\n",
                total.sealedCulled,
                100.0 * total.sealedCulled / double(total.exposedFaces));
    std::printf("         backface%9ld  (%.1f%%, normal away from camera)\n",
                total.backfaceCulled,
                100.0 * total.backfaceCulled / double(total.exposedFaces));
    std::printf("         drawn   %9ld  quads after merging (%.1fx fewer than"
                " exposed faces)\n",
                total.quads, double(total.exposedFaces) / double(total.quads));

    // Perspective render of the strip. LOD is meant to be invisible; what this
    // image checks is that it very nearly is, and where it is not.
    const int RW = 1100, RH = 620;
    Raster strip(RW, RH);
    strip.clear(18, 20, 28);
    const double focal = RH * 0.9;
    const double tilt = 0.16;                   // camera looks slightly down
    const double ct = std::cos(tilt), stt = std::sin(tilt);
    long drawn = 0, clipped = 0;
    for (const Mesh& m : meshes) {
        for (size_t k = 0; k + 2 < m.indices.size(); k += 3) {
            const Vertex* vs[3] = {&m.vertices[m.indices[k]],
                                   &m.vertices[m.indices[k + 1]],
                                   &m.vertices[m.indices[k + 2]]};
            float pr[3][3];
            uint8_t cols[3][3];
            bool ok = true;
            for (int j = 0; j < 3; ++j) {
                const double dx = vs[j]->x - camX;
                const double dy = vs[j]->y - camY;
                const double dz = vs[j]->z - camZ;
                const double ry = dy * ct + dz * stt;
                const double rz = -dy * stt + dz * ct;
                if (rz < 1.0) { ok = false; break; }
                pr[j][0] = float(RW * 0.5 + dx * focal / rz);
                pr[j][1] = float(RH * 0.5 - ry * focal / rz);
                pr[j][2] = float(rz);
                shadeVertex(*vs[j], cols[j]);
                // Distance haze, so the LOD gradient is legible rather than
                // hidden: the eye should read depth, not resolution.
                const double haze = clampd(rz / 520.0, 0.0, 0.82);
                for (int c = 0; c < 3; ++c)
                    cols[j][c] = uint8_t(cols[j][c] * (1 - haze) +
                                         (c == 0 ? 78 : c == 1 ? 92 : 118) * haze);
            }
            if (!ok) { ++clipped; continue; }
            strip.triangle(pr[0], pr[1], pr[2], cols[0], cols[1], cols[2]);
            ++drawn;
        }
    }
    std::printf("  render  %ld triangles (%ld clipped at the near plane)\n",
                drawn, clipped);
    strip.image().writePng(out + "/lod_distance_" + std::to_string(seed) + ".png");

    // -----------------------------------------------------------------------
    // 3. Where sealed-cavity culling actually pays: underground.
    //
    // On the surface every pocket of air joins the sky, so the cull finds
    // almost nothing — which is the honest result and is why it is reported
    // rather than assumed. Two hundred metres down, in the middle of the cave
    // band, the picture is different, and that is where a player mines.
    // -----------------------------------------------------------------------
    {
        const int N = 64;
        const double depth = 200.0;
        Region r = generate(t, face, uC, vC, -N * 0.5, surface - depth - N * 0.5,
                            -N * 0.5, N, N, N, 1.0, 1);
        auto samp = [&](int x, int y, int z) { return r.at(x, y, z); };
        VisibilityMask vis(N, N, N, samp);

        MeshStats plain, culled;
        MeshOptions none;
        greedyMesh(N, N, N, samp, none, &plain);
        MeshOptions opt;
        opt.visibility = &vis;
        greedyMesh(N, N, N, samp, opt, &culled);

        std::printf("\nsealed cavities: %d^3 m of rock centred %.0f m below the"
                    " surface\n", N, depth + N * 0.5);
        std::printf("  air     %9ld voxels, %ld of them sealed off (%.1f%%)\n",
                    vis.airVoxels(), vis.sealedAir(),
                    100.0 * vis.sealedAir() /
                        double(vis.airVoxels() ? vis.airVoxels() : 1));
        std::printf("  faces   %9ld exposed, %ld culled as unreachable (%.1f%%)\n",
                    plain.exposedFaces, culled.sealedCulled,
                    100.0 * culled.sealedCulled /
                        double(plain.exposedFaces ? plain.exposedFaces : 1));
        std::printf("  quads   %9ld -> %ld after culling\n",
                    plain.quads, culled.quads);
        if (vis.sealedAir() == 0)
            std::printf("  reading: this generator's caves are one connected"
                        " network, so almost\n"
                        "           nothing is sealed and this cull earns"
                        " nothing here. Kept\n"
                        "           because it costs one pass and will earn its"
                        " keep on sealed\n"
                        "           player rooms and structure interiors — but"
                        " it is not what\n"
                        "           makes the frame budget. LOD and greedy"
                        " merging are.\n");
    }

    std::printf("\n  note: tiles at different levels do not share vertices, so"
                " hairline\n"
                "        cracks are visible where two levels meet. Skirts at"
                " tile edges\n"
                "        are the standard fix and are not implemented yet.\n");
    return 0;
}
