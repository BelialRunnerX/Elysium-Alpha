// mesh tests
#include "harness.h"

#include "mesh/greedy.h"
#include "mesh/visibility.h"
#include "world/chunk.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <vector>

using namespace ely;
using elytest::section;

static void testMesh() {
    section("greedy meshing (spec 12.2)");

    // A single voxel: 6 faces, 6 quads, no merging possible.
    {
        Mesh m = greedyMesh(3, 3, 3, [](int x, int y, int z) {
            return (x == 1 && y == 1 && z == 1) ? MAT_STONE : MAT_AIR;
        });
        CHECK(m.quads() == 6, "one voxel produced %zu quads, expected 6", m.quads());
    }
    // A solid 16^3 block: 6 merged faces total, if merging works at all.
    {
        // The sampler is the contract: it must answer for the ring of voxels
        // just outside the region too, or the mesher cannot know whether a
        // boundary face is exposed. Answering air there means "this region
        // stands alone"; answering the neighbour's rock means "join to it".
        auto solid = [](int x, int y, int z) {
            const bool in = x >= 0 && y >= 0 && z >= 0 && x < 16 && y < 16 && z < 16;
            return in ? MAT_STONE : MAT_AIR;
        };
        Mesh m = greedyMesh(16, 16, 16, solid);
        CHECK(m.quads() == 6, "a solid cube produced %zu quads, expected 6 "
              "(greedy merging is not working)", m.quads());
    }
    // A flat slab: top and bottom merge to one quad each, sides to one each.
    {
        Mesh m = greedyMesh(16, 4, 16, [](int x, int y, int z) {
            const bool in = x >= 0 && z >= 0 && x < 16 && z < 16;
            return (in && y == 0) ? MAT_STONE : MAT_AIR;
        });
        CHECK(m.quads() == 6, "a flat slab produced %zu quads, expected 6", m.quads());
    }
    // Two different materials must not merge.
    {
        Mesh m = greedyMesh(2, 1, 1, [](int x, int y, int z) {
            if (y != 0 || z != 0) return MAT_AIR;
            if (x == 0) return MAT_STONE;
            if (x == 1) return MAT_DIRT;
            return MAT_AIR;
        });
        // 2 side faces each on 4 axes for the pair, plus 2 ends: 10 quads.
        CHECK(m.quads() == 10, "two adjacent materials produced %zu quads, expected 10",
              m.quads());
    }
    // Every index must address a real vertex.
    {
        Mesh m = greedyMesh(8, 8, 8, [](int x, int y, int z) {
            const bool in = x >= 0 && y >= 0 && z >= 0 && x < 8 && y < 8 && z < 8;
            return (in && (x * 7 + y * 13 + z * 29) % 5 == 0) ? MAT_STONE : MAT_AIR;
        });
        bool inRange = true;
        for (uint32_t i : m.indices)
            if (i >= m.vertices.size()) { inRange = false; break; }
        CHECK(inRange, "mesh contains an out-of-range index");
        CHECK(m.indices.size() % 6 == 0, "index count is not a whole number of quads");
    }
}

// ---------------------------------------------------------------------------
// 5. Ore distribution — that the depth bands from spec §5.2 actually appear
//    where the table says, and that class abundance (Mechanism 4) bites.
// ---------------------------------------------------------------------------



// Culling: what can be seen, and what provably cannot.
//
// These live here rather than with the LOD tests because they are questions
// about the mesher. The module split made that obvious: they would not link
// against ely_world alone.
static void testCulling() {
    elytest::section("visibility and backface culling");
    // --- Culling. A hollow shell: the cavity inside cannot be seen, so its
    //     walls must not be meshed. Open the shell and they must come back.
    {
        const int N = 12;
        auto shell = [&](int x, int y, int z) {
            const bool in = x >= 0 && y >= 0 && z >= 0 && x < N && y < N && z < N;
            if (!in) return MAT_AIR;
            const bool inner = x >= 3 && y >= 3 && z >= 3 &&
                               x < N - 3 && y < N - 3 && z < N - 3;
            return inner ? MAT_AIR : MAT_STONE;
        };
        VisibilityMask sealed(N, N, N, shell);
        CHECK(sealed.sealedAir() == 6 * 6 * 6,
              "%ld sealed air voxels in a 6^3 cavity, expected 216",
              sealed.sealedAir());
        CHECK(!sealed.exteriorAir(6, 6, 6), "the sealed cavity was reachable");
        CHECK(sealed.exteriorAir(-1, 6, 6),
              "outside the region must always count as reachable");

        MeshOptions plain;
        MeshStats sPlain;
        greedyMesh(N, N, N, shell, plain, &sPlain);

        MeshOptions culled;
        culled.visibility = &sealed;
        MeshStats sCulled;
        greedyMesh(N, N, N, shell, culled, &sCulled);

        CHECK(sCulled.sealedCulled == 6 * 6 * 6,
              "culled %ld faces around the cavity, expected 216",
              sCulled.sealedCulled);
        CHECK(sCulled.emittedFaces + sCulled.sealedCulled == sPlain.exposedFaces,
              "culling did not account for every face it removed");
        CHECK(sCulled.quads < sPlain.quads, "culling the cavity saved no quads");

        // Now punch a hole through to the outside. Nothing may be culled: the
        // cavity is reachable, and culling something reachable is a hole in
        // the world, which is far worse than drawing something hidden.
        auto opened = [&](int x, int y, int z) {
            if (y >= 0 && y < 3 && x == 5 && z == 5) return MAT_AIR;
            return shell(x, y, z);
        };
        VisibilityMask open(N, N, N, opened);
        CHECK(open.sealedAir() == 0, "%ld voxels sealed in a cavity with a way out",
              open.sealedAir());
        MeshOptions o2;
        o2.visibility = &open;
        MeshStats s2;
        greedyMesh(N, N, N, opened, o2, &s2);
        CHECK(s2.sealedCulled == 0, "culled %ld faces of a reachable cavity",
              s2.sealedCulled);
    }

    // --- Backface culling removes exactly the three axis directions that face
    //     away, and never anything else.
    {
        const int N = 8;
        auto cube = [&](int x, int y, int z) {
            const bool in = x >= 0 && y >= 0 && z >= 0 && x < N && y < N && z < N;
            return in ? MAT_STONE : MAT_AIR;
        };
        MeshStats all, half;
        MeshOptions none;
        greedyMesh(N, N, N, cube, none, &all);
        MeshOptions back;
        back.cullBackFaces = true;
        back.viewX = 0.4; back.viewY = -0.6; back.viewZ = 0.7;
        greedyMesh(N, N, N, cube, back, &half);
        CHECK(all.exposedFaces == 6 * N * N, "a solid cube exposed %ld faces",
              all.exposedFaces);
        CHECK(half.backfaceCulled == 3 * N * N,
              "backface culling removed %ld of %ld faces, expected half",
              half.backfaceCulled, all.exposedFaces);
        CHECK(half.quads == 3, "a cube seen from one corner needs %ld quads,"
              " expected 3", half.quads);
    }

}

int main() {
    std::printf("Elysium — mesh tests\n");
    testMesh();
    testCulling();
    return elytest::report("mesh tests");
}
