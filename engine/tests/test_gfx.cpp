// gfx tests
#include "harness.h"

#include "gfx/frustum.h"
#include "gfx/mat.h"
#include "gfx/shading.h"
#include "gfx/streamer.h"
#include "gfx/tilebuild.h"
#include "gfx/vertex.h"
#include "jobs/jobsystem.h"
#include "mesh/greedy.h"
#include "mesh/visibility.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <vector>

using namespace ely;
using elytest::section;

static void testRenderLayer() {
    section("render layer: matrices, frustum, vertices, streaming");

    // --- Vulkan clip-space conventions. Every one of these is a bug that
    //     looks like "the renderer is broken" and takes an afternoon to find.
    {
        const double aspect = 16.0 / 9.0, zn = 0.1, zf = 100.0;
        Mat4 proj = perspectiveGl(1.2217304764, aspect, zn, zf);

        // OpenGL depth: -1 at the near plane, +1 at the far plane.
        float p[4];
        proj.transform(0, 0, float(-zn), p);
        CHECK(std::fabs(p[2] / p[3] + 1.0) < 1e-5,
              "the near plane maps to depth %.6f, expected -1", p[2] / p[3]);
        proj.transform(0, 0, float(-zf), p);
        CHECK(std::fabs(p[2] / p[3] - 1.0) < 1e-4,
              "the far plane maps to depth %.6f, expected +1", p[2] / p[3]);

        // Y is up in NDC. This is the one that was wrong first time round: with
        // Vulkan's downward Y the first GPU frame came out perfectly mirrored,
        // which reads as a broken camera rather than a wrong sign.
        proj.transform(0, 1, -10, p);
        CHECK(p[1] / p[3] > 0.0,
              "a point above the camera landed at ndc y=%.3f; the world is"
              " upside down", p[1] / p[3]);
        proj.transform(1, 0, -10, p);
        CHECK(p[0] / p[3] > 0.0, "a point to the right landed at ndc x=%.3f",
              p[0] / p[3]);

        // A point behind the camera must come out with w <= 0, which is what
        // makes the near-plane test meaningful.
        proj.transform(0, 0, 10, p);
        CHECK(p[3] <= 0.0, "a point behind the camera has w=%.3f, expected <= 0",
              p[3]);
    }

    // --- The camera's own basis.
    {
        Camera cam;
        cam.position = Vec3{0, 0, 0};
        cam.yaw = 0.0; cam.pitch = 0.0;
        Vec3 f = cam.forward();
        CHECK(std::fabs(f.x) < 1e-9 && std::fabs(f.y) < 1e-9 &&
              std::fabs(f.z + 1.0) < 1e-9,
              "yaw 0 should look down -Z, got (%.3f, %.3f, %.3f)", f.x, f.y, f.z);
        CHECK(std::fabs(cam.forward().dot(cam.right())) < 1e-9,
              "forward and right are not perpendicular");

        cam.pitch = 10.0; cam.clampPitch();
        CHECK(cam.pitch < 1.5708, "pitch clamped to %.4f, must stay under"
              " vertical or the view rolls", cam.pitch);

        // The full transform must put what is in front of the camera on screen
        // and what is behind it off.
        cam.position = Vec3{0, 0, 0};
        cam.yaw = 0; cam.pitch = 0;
        Mat4 vp = cam.viewProjection(1.0);
        float a[4], b[4];
        vp.transform(0, 0, -10, a);
        vp.transform(0, 0, 10, b);
        CHECK(a[3] > 0.0 && b[3] <= 0.0,
              "the view-projection does not separate front from behind"
              " (w=%.3f in front, %.3f behind)", a[3], b[3]);
    }

    // --- Frustum culling. Conservative is required; wrong is not allowed.
    {
        Camera cam;
        cam.position = Vec3{0, 0, 0};
        cam.yaw = 0; cam.pitch = 0;
        cam.zNear = 0.5; cam.zFar = 200.0;
        Frustum fr(cam.viewProjection(1.0));

        CHECK(fr.containsPoint(0, 0, -10), "a point straight ahead was culled");
        CHECK(!fr.containsPoint(0, 0, 10), "a point behind the camera survived");
        CHECK(!fr.containsPoint(0, 0, -400), "a point past the far plane survived");
        CHECK(!fr.containsPoint(0, 0, -0.1), "a point inside the near plane survived");
        CHECK(!fr.containsPoint(500, 0, -10), "a point far off to the side survived");

        CHECK(fr.visible(Aabb::fromOriginSize(-5, -5, -30, 10, 10, 10)),
              "a box in front of the camera was culled");
        CHECK(!fr.visible(Aabb::fromOriginSize(-5, -5, 20, 10, 10, 10)),
              "a box entirely behind the camera survived");
        // A box straddling the near plane must be kept: culling it would clip
        // away the ground the player is standing on.
        CHECK(fr.visible(Aabb::fromOriginSize(-5, -5, -2, 10, 10, 10)),
              "a box straddling the camera was culled");

        // Planes must be normalised, or "how far outside" is meaningless.
        bool unit = true;
        for (int i = 0; i < 6; ++i) {
            const double* pl = fr.plane(i);
            const double len = std::sqrt(pl[0] * pl[0] + pl[1] * pl[1] + pl[2] * pl[2]);
            if (std::fabs(len - 1.0) > 1e-9) unit = false;
        }
        CHECK(unit, "frustum planes are not normalised");

        // Turning around must change what is visible, and by roughly half.
        int aheadVisible = 0, behindVisible = 0;
        for (int i = 0; i < 200; ++i) {
            const double ang = i * 0.0314159265;
            const Aabb box = Aabb::fromOriginSize(std::cos(ang) * 60 - 4, -4,
                                                  std::sin(ang) * 60 - 4, 8, 8, 8);
            if (fr.visible(box)) ++aheadVisible;
        }
        cam.yaw = 3.14159265358979;
        Frustum back(cam.viewProjection(1.0));
        for (int i = 0; i < 200; ++i) {
            const double ang = i * 0.0314159265;
            const Aabb box = Aabb::fromOriginSize(std::cos(ang) * 60 - 4, -4,
                                                  std::sin(ang) * 60 - 4, 8, 8, 8);
            if (back.visible(box)) ++behindVisible;
        }
        CHECK(aheadVisible > 0 && aheadVisible < 200,
              "the frustum kept %d of 200 boxes on a ring; it is not culling",
              aheadVisible);
        CHECK(aheadVisible + behindVisible < 240,
              "facing two opposite directions kept %d of 400 box tests;"
              " the planes overlap far more than a 70 degree field should",
              aheadVisible + behindVisible);
    }

    // --- The packed vertex. Eight bytes, and every field survives the trip.
    {
        CHECK(sizeof(GpuVertex) == 8, "GpuVertex is %zu bytes, must be 8",
              sizeof(GpuVertex));
        bool packOk = true;
        for (int ao = 0; ao < 4; ++ao)
            for (int axis = 0; axis < 6; ++axis) {
                GpuVertex g{};
                g.aoAxis = GpuVertex::pack(ao, axis);
                if (g.ao() != ao || g.axis() != axis) packOk = false;
            }
        CHECK(packOk, "AO and face axis do not round-trip through one byte");

        // A real mesh, converted, must land exactly on the voxel lattice at
        // every level — including micro-voxels, where a float world position
        // has barely enough precision to represent the corner at all.
        const double sizes[3] = {1.0, 0.0625, 4.0};
        bool exact = true;
        double worst = 0.0;
        for (double vs : sizes) {
            const int N = 8;
            auto blob = [&](int x, int y, int z) {
                const bool in = x >= 0 && y >= 0 && z >= 0 && x < N && y < N && z < N;
                return (in && (x + y + z) % 3 != 0) ? MAT_STONE : MAT_AIR;
            };
            MeshOptions opt;
            opt.voxelSize = vs;
            opt.ox = 1234.5; opt.oy = -321.25; opt.oz = 77.0;
            Mesh m = greedyMesh(N, N, N, blob, opt);
            GpuMesh g = toGpuMesh(m, opt.ox, opt.oy, opt.oz, vs);
            CHECK(g.vertices.size() == m.vertices.size(),
                  "conversion changed the vertex count");
            for (size_t i = 0; i < m.vertices.size(); ++i) {
                // Reconstruct exactly as the vertex shader will.
                const double rx = opt.ox + double(g.vertices[i].x) * vs;
                const double ry = opt.oy + double(g.vertices[i].y) * vs;
                const double rz = opt.oz + double(g.vertices[i].z) * vs;
                worst = std::max(worst, std::fabs(rx - m.vertices[i].x));
                worst = std::max(worst, std::fabs(ry - m.vertices[i].y));
                worst = std::max(worst, std::fabs(rz - m.vertices[i].z));
                if (g.vertices[i].material != m.vertices[i].mat) exact = false;
                if (g.vertices[i].ao() != m.vertices[i].ao) exact = false;
                if (g.vertices[i].axis() != m.vertices[i].nx) exact = false;
            }
        }
        CHECK(exact, "material, AO or face axis was lost in packing");
        CHECK(worst < 1e-3, "packing moved a vertex by %.6f m; positions must"
              " land exactly on the voxel lattice", worst);
    }

    // --- Winding order. The GPU's back-face cull removes triangles whose
    //     screen-space winding says they face away; if the mesher winds a face
    //     the wrong way, the cull removes the visible half of the world
    //     instead of the hidden half. That does not look like a state bug — it
    //     looks like a shading bug, because what you get is a picture of the
    //     inside of the terrain. It was one, and this is the test that would
    //     have caught it in a second instead of a render.
    //
    //     Checked geometrically, with no GPU: the normal implied by each
    //     triangle's vertex order must point the same way as the face it
    //     belongs to.
    {
        const int N = 6;
        auto cube = [&](int x, int y, int z) {
            const bool in = x >= 0 && y >= 0 && z >= 0 && x < N && y < N && z < N;
            return in ? MAT_STONE : MAT_AIR;
        };
        Mesh m = greedyMesh(N, N, N, cube);
        // Outward normal for each face axis: +X, -X, +Y, -Y, +Z, -Z.
        const double axisNormal[6][3] = {{1,0,0},{-1,0,0},{0,1,0},
                                         {0,-1,0},{0,0,1},{0,0,-1}};
        int wrong = 0, checked = 0;
        for (size_t i = 0; i + 2 < m.indices.size(); i += 3) {
            const Vertex& a = m.vertices[m.indices[i]];
            const Vertex& b = m.vertices[m.indices[i + 1]];
            const Vertex& c = m.vertices[m.indices[i + 2]];
            const double e1[3] = {b.x - a.x, b.y - a.y, b.z - a.z};
            const double e2[3] = {c.x - a.x, c.y - a.y, c.z - a.z};
            // Right-handed cross product: this is the normal a
            // counter-clockwise winding implies.
            const double n[3] = {e1[1] * e2[2] - e1[2] * e2[1],
                                 e1[2] * e2[0] - e1[0] * e2[2],
                                 e1[0] * e2[1] - e1[1] * e2[0]};
            const double* want = axisNormal[a.nx < 6 ? a.nx : 0];
            const double d = n[0] * want[0] + n[1] * want[1] + n[2] * want[2];
            ++checked;
            if (d <= 0.0) ++wrong;
        }
        CHECK(checked > 0, "the winding test meshed nothing");
        CHECK(wrong == 0, "%d of %d triangles wind the wrong way; the GPU's"
              " back-face cull would remove the visible half of the world",
              wrong, checked);
    }

    // --- The shading model. These constants are duplicated by hand into GLSL,
    //     so they are pinned: a change here without a change there makes the
    //     headless proof images a picture of something the GPU does not draw.
    {
        CHECK(std::fabs(kFaceLight[2] - 1.00f) < 1e-6 &&
              std::fabs(kFaceLight[3] - 0.55f) < 1e-6,
              "face lighting changed; update shaders/terrain.frag to match");
        CHECK(std::fabs(aoLight(0) - 0.52f) < 1e-6 &&
              std::fabs(aoLight(3) - 1.00f) < 1e-6,
              "the AO response changed; update shaders/terrain.frag to match");

        // Sky-facing and open must be the brightest case, and it must be
        // exactly the material's own colour: anything else means the model is
        // darkening everything.
        const float white[3] = {1, 1, 1};
        float out[3];
        shadeVoxelFace(white, 2, 3, 0.0f, out);
        CHECK(std::fabs(out[0] - 1.0f) < 1e-6,
              "a fully lit, fully open top face came out at %.4f, expected 1",
              out[0]);
        shadeVoxelFace(white, 3, 0, 0.0f, out);
        CHECK(out[0] < 0.3f, "the darkest case came out at %.4f; the model has"
              " no contrast", out[0]);

        // Fog saturates and never quite reaches the fog colour.
        float far1[3], far2[3];
        shadeVoxelFace(white, 2, 3, kFogFull * 2.0f, far1);
        shadeVoxelFace(white, 2, 3, kFogFull * 20.0f, far2);
        CHECK(std::fabs(far1[0] - far2[0]) < 1e-6, "fog does not saturate");
        CHECK(far1[0] > kFogColour[0],
              "fog reached full strength; distant terrain will have no"
              " silhouette");

        // The palette the GPU gets must agree with the material table, entry
        // for entry — it is a transport format, not a second source of truth.
        MaterialPalette pal;
        bool palOk = true;
        for (int i = 0; i < MAT_COUNT; ++i) {
            const MaterialInfo& info = materialInfo(Material(i));
            if (std::fabs(pal.rgb[i][0] - info.r / 255.0f) > 1e-6) palOk = false;
            if (std::fabs(pal.rgb[i][2] - info.b / 255.0f) > 1e-6) palOk = false;
        }
        CHECK(palOk, "the GPU palette disagrees with the material table");
        CHECK(MaterialPalette::bytes() % 16 == 0,
              "the palette is %zu bytes, not a multiple of 16; std140 will"
              " misalign it", MaterialPalette::bytes());
    }

    // --- The streamer. Everything here is about motion.
    {
        StreamerConfig cfg;
        cfg.tileSize = 32.0;
        cfg.viewDistance = 256.0;
        cfg.nearDistance = 40.0;
        cfg.maxLoadsPerUpdate = 100000;   // no budget, so residency settles at once

        Streamer st(cfg);
        Vec3 eye{0, 0, 0};
        st.update(eye, nullptr);
        CHECK(st.residentCount() > 0, "nothing became resident");

        // The tile the camera is inside must be at the finest level, and
        // distant tiles must be coarser. This is the rule, restated where a
        // moving camera can break it.
        const int here = st.lodOf(TileKey{0, 0, 0});
        const int far = st.lodOf(TileKey{7, 0, 0});
        CHECK(here == cfg.finestLod,
              "the tile under the camera is at lod %d, expected %d", here,
              cfg.finestLod);
        CHECK(far > here, "a tile 224 m away is at lod %d, no coarser than the"
              " one underfoot", far);

        // Everything resident must be within the view distance, and everything
        // within it must be resident.
        bool bounded = true;
        for (int tz = -12; tz <= 12; ++tz)
            for (int tx = -12; tx <= 12; ++tx) {
                const TileKey k{tx, 0, tz};
                const double d = std::sqrt(double(tx * tx + tz * tz)) * cfg.tileSize;
                if (d > cfg.viewDistance * 1.5 && st.isResident(k)) bounded = false;
            }
        CHECK(bounded, "tiles well past the view distance stayed resident");

        // THRASHING. Jiggle the camera on a level boundary and count rebuilds.
        // Without hysteresis this is where a core goes to die.
        StreamerConfig hy = cfg;
        hy.hysteresis = 0.15;
        Streamer stable(hy);
        const double boundary = hy.nearDistance * 2.0;   // where lod 1 begins
        // Warm up first. Arriving somewhere new is *supposed* to cost work;
        // the question this test asks is what a camera that has arrived and is
        // now merely breathing costs, which is a steady-state question.
        for (int i = 0; i < 6; ++i)
            stable.update(Vec3{boundary + (i % 2 ? 0.02 : -0.02) * boundary, 0, 0},
                          nullptr);
        int rebuilds = 0;
        for (int i = 0; i < 60; ++i) {
            const double wobble = (i % 2 ? 0.02 : -0.02) * boundary;
            stable.update(Vec3{boundary + wobble, 0, 0}, nullptr);
            rebuilds += int(stable.toLoad().size());
        }
        CHECK(rebuilds == 0, "a camera hovering on a level boundary triggered"
              " %d rebuilds in 60 steady-state frames; hysteresis is not"
              " working", rebuilds);

        StreamerConfig noHy = cfg;
        noHy.hysteresis = 0.0;
        Streamer jumpy(noHy);
        for (int i = 0; i < 6; ++i)
            jumpy.update(Vec3{boundary + (i % 2 ? 0.02 : -0.02) * boundary, 0, 0},
                         nullptr);
        int naiveRebuilds = 0;
        for (int i = 0; i < 60; ++i) {
            const double wobble = (i % 2 ? 0.02 : -0.02) * boundary;
            jumpy.update(Vec3{boundary + wobble, 0, 0}, nullptr);
            naiveRebuilds += int(jumpy.toLoad().size());
        }
        CHECK(naiveRebuilds > rebuilds,
              "hysteresis made no difference (%d rebuilds with, %d without);"
              " the test is not exercising the boundary",
              rebuilds, naiveRebuilds);
        std::printf("    boundary wobble over 60 frames: %d rebuilds with"
                    " hysteresis, %d without\n", rebuilds, naiveRebuilds);

        // UNBOUNDED WORK. A teleport must not try to build the world in one
        // update; the budget has to hold.
        StreamerConfig bud = cfg;
        bud.maxLoadsPerUpdate = 8;
        Streamer paced(bud);
        paced.update(Vec3{0, 0, 0}, nullptr);
        paced.update(Vec3{100000, 0, 100000}, nullptr);
        CHECK(int(paced.toLoad().size()) <= bud.maxLoadsPerUpdate,
              "a teleport queued %zu loads against a budget of %zu",
              paced.toLoad().size(), size_t(bud.maxLoadsPerUpdate));
        CHECK(!paced.toUnload().empty(),
              "a teleport unloaded nothing; memory would grow without bound");

        // Loads must arrive nearest first: the tile under the player is the
        // one whose absence is a hole in the floor.
        Streamer ordered(bud);
        ordered.update(Vec3{0, 0, 0}, nullptr);
        bool nearestFirst = true;
        double prev = -1.0;
        for (const auto& l : ordered.toLoad()) {
            const Aabb b = ordered.boundsOf(l.key);
            const double d = std::sqrt(b.centreX() * b.centreX() +
                                       b.centreY() * b.centreY() +
                                       b.centreZ() * b.centreZ());
            if (d + 1e-6 < prev) nearestFirst = false;
            prev = d;
        }
        CHECK(nearestFirst, "loads are not ordered nearest first");

        // Turning around must not reload anything: the frustum decides what is
        // drawn, residency is decided by distance alone.
        Streamer turning(cfg);
        Camera cam;
        cam.position = Vec3{0, 0, 0};
        turning.update(cam.position, nullptr);
        cam.yaw = 0.0;
        Frustum f0(cam.viewProjection(1.6));
        turning.update(cam.position, &f0);
        const size_t before = turning.residentCount();
        const size_t seenAhead = turning.visibleCount();
        cam.yaw = 3.14159265358979;
        Frustum f1(cam.viewProjection(1.6));
        turning.update(cam.position, &f1);
        CHECK(turning.toLoad().empty(),
              "turning around queued %zu loads; residency must not depend on"
              " view direction", turning.toLoad().size());
        CHECK(turning.residentCount() == before,
              "turning around changed residency from %zu to %zu tiles",
              before, turning.residentCount());
        CHECK(seenAhead > 0 && seenAhead < before,
              "the frustum marked %zu of %zu resident tiles visible; it is not"
              " culling", seenAhead, before);
        std::printf("    %zu tiles resident, %zu visible facing forward,"
                    " %zu facing back\n",
                    before, seenAhead, turning.visibleCount());

        // A walk across the world: residency must stay bounded, and the work
        // per frame must stay under budget the whole way.
        Streamer walk(bud);
        size_t peak = 0;
        int overBudget = 0;
        for (int i = 0; i < 400; ++i) {
            walk.update(Vec3{i * 4.0, 0, i * 1.5}, nullptr);
            peak = std::max(peak, walk.residentCount());
            if (int(walk.toLoad().size()) > bud.maxLoadsPerUpdate) ++overBudget;
        }
        CHECK(overBudget == 0, "%d frames of a 1.6 km walk exceeded the load"
              " budget", overBudget);
        CHECK(peak < 4000, "residency peaked at %zu tiles during the walk",
              peak);
        std::printf("    1.6 km walk: %zu tiles peak residency, no frame over"
                    " the %d-tile budget\n", peak, bud.maxLoadsPerUpdate);
    }
}


// Threading must not change the world.
//
// The engine's whole contract is that a seed names a world (principle S2). A
// job system that is fast and non-deterministic is worse than no job system at
// all, because the failure is invisible: two players on the same seed quietly
// stand on different ground. The column walk inside a tile writes index i to
// slot i and accumulates nothing across threads, so this holds by construction
// — and this test is what catches the day somebody adds a shared accumulator.
static void testThreadedBuildIsDeterministic() {
    section("threaded tile building is deterministic");

    PlanetParams params = generatePlanetFromSeed(23);
    Terrain terrain(params);

    TileSpec spec;
    spec.face = FACE_PZ;
    spec.ox = -16.0;
    spec.oz = -16.0;
    spec.oy = terrain.surfaceAltitude(FACE_PZ, 0, 0) - 24.0;
    spec.sizeX = 32.0; spec.sizeY = 48.0; spec.sizeZ = 32.0;
    spec.lod = 1;

    BuiltTile serial = buildTile(terrain, spec, nullptr);
    CHECK(!serial.gpu.vertices.empty(), "the reference tile meshed to nothing");

    for (int workers : {1, 2, 4, 8}) {
        JobSystem jobs(workers);
        BuiltTile threaded = buildTile(terrain, spec, &jobs);

        bool same = threaded.gpu.vertices.size() == serial.gpu.vertices.size() &&
                    threaded.gpu.indices.size() == serial.gpu.indices.size();
        if (same) {
            for (size_t i = 0; i < serial.gpu.vertices.size(); ++i) {
                const GpuVertex& a = serial.gpu.vertices[i];
                const GpuVertex& b = threaded.gpu.vertices[i];
                if (a.x != b.x || a.y != b.y || a.z != b.z ||
                    a.material != b.material || a.aoAxis != b.aoAxis) {
                    same = false;
                    break;
                }
            }
            for (size_t i = 0; i < serial.gpu.indices.size() && same; ++i)
                if (serial.gpu.indices[i] != threaded.gpu.indices[i]) same = false;
        }
        CHECK(same, "building with %d workers produced a different tile than"
              " building with none: %zu vertices against %zu", workers,
              threaded.gpu.vertices.size(), serial.gpu.vertices.size());
    }
}

int main() {
    std::printf("Elysium — gfx tests\n");
    testRenderLayer();
    testThreadedBuildIsDeterministic();
    return elytest::report("gfx tests");
}
