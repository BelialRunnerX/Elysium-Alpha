// world tests
#include "harness.h"

#include "core/seed.h"
#include "world/chunk.h"
#include "world/lod.h"
#include "world/terrain.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <vector>

using namespace ely;
using elytest::section;

// ---------------------------------------------------------------------------
// 1. Determinism — principle S2. Regenerate a fixed set of voxels across a
//    fixed set of seeds and compare a hash. This is the regression test that
//    stands between a generator change and every player's base being buried.
// ---------------------------------------------------------------------------
static uint64_t worldFingerprint(uint64_t planetSeed) {
    PlanetParams p = generatePlanetFromSeed(planetSeed);
    Terrain t(p);
    uint64_t h = 0xCBF29CE484222325ull;
    for (int f = 0; f < 6; ++f) {
        for (int i = 0; i < 7; ++i) {
            const double u = -0.86 + i * 0.29;
            for (int j = 0; j < 7; ++j) {
                const double v = -0.86 + j * 0.29;
                for (int k = 0; k < 9; ++k) {
                    const double alt = 200.0 - k * 78.0;
                    Material m = t.materialAt(Face(f), u, v, alt);
                    h = (h ^ uint64_t(m)) * 0x100000001B3ull;
                }
            }
        }
    }
    return h;
}


static void testDeterminism() {
    section("determinism (S2)");
    const uint64_t seeds[] = {1, 42, 0xDEADBEEF, 0x5EED1234, 987654321};
    for (uint64_t s : seeds) {
        const uint64_t a = worldFingerprint(s);
        const uint64_t b = worldFingerprint(s);
        CHECK(a == b, "seed %llu regenerated differently: %llx vs %llx",
              (unsigned long long)s, (unsigned long long)a, (unsigned long long)b);
    }
    // GOLDEN FINGERPRINTS.
    //
    // Self-consistency proves the generator is a pure function; it does not
    // prove the world has not moved. These constants were captured before the
    // generator was optimised, and every optimisation since has been required
    // to reproduce them exactly — which is what made it safe to hoist terms out
    // of the density field, start the surface scan a hundred metres lower, and
    // cache the per-ore constants. A change here is not necessarily a bug, but
    // it IS a new world: every saved base is in the wrong place. Changing these
    // numbers must be a deliberate act with a version bump beside it.
    const uint64_t golden[] = {
        0x9ff58a3b49db9919ull,  // seed 1
        0xedb4787a88f115e4ull,  // seed 42
        0xe66f76d986afe010ull,  // seed 3735928559
        0xc45451f0b3dd2a47ull,  // seed 1592594996
        0x9ee7ae1bc9a5b6bdull,  // seed 987654321
    };
    for (size_t i = 0; i < sizeof(golden) / sizeof(golden[0]); ++i) {
        const uint64_t got = worldFingerprint(seeds[i]);
        CHECK(got == golden[i],
              "seed %llu now generates a DIFFERENT WORLD: %016llx, was %016llx."
              " Every saved base on that planet is in the wrong place.",
              (unsigned long long)seeds[i], (unsigned long long)got,
              (unsigned long long)golden[i]);
    }

    // Different seeds must give different worlds, or the seed is not wired in.
    CHECK(worldFingerprint(1) != worldFingerprint(2),
          "two seeds produced identical worlds");

    // Planet parameters are equally a pure function of the seed.
    PlanetParams p1 = generatePlanetFromSeed(777);
    PlanetParams p2 = generatePlanetFromSeed(777);
    CHECK(p1.cls == p2.cls && p1.radius == p2.radius && p1.gravity == p2.gravity,
          "planet parameters are not deterministic");
}

// ---------------------------------------------------------------------------
// 2. The seam — spec §4.1, the project's highest technical risk.
//
//    Walk all 12 cube edges. At each sample, take the two faces that meet
//    there and confirm they agree about the world. They must, because every
//    field is sampled in direction space; this test is what proves it, and
//    what will catch the first time somebody samples a field in face-local
//    (u, v) instead.
// ---------------------------------------------------------------------------


static void testSeam() {
    section("cube-sphere seam (spec 4.1)");
    PlanetParams p = generatePlanetFromSeed(31337);
    Terrain t(p);

    int checked = 0;
    double worstHeight = 0.0;
    int materialMismatches = 0;

    // For each face, walk just inside each of its four edges. Project the
    // resulting direction back to find which face owns it; where that is a
    // *different* face we are looking across a seam, and both descriptions of
    // the same point must agree.
    for (int f = 0; f < 6; ++f) {
        for (int edge = 0; edge < 4; ++edge) {
            for (int s = 1; s < 40; ++s) {
                const double tpar = -1.0 + 2.0 * (double(s) / 40.0);
                double u, v;
                const double e = 0.999999;
                switch (edge) {
                    case 0: u = -e;   v = tpar; break;
                    case 1: u =  e;   v = tpar; break;
                    case 2: u = tpar; v = -e;   break;
                    default:u = tpar; v =  e;   break;
                }
                const Vec3 dir = CubeSphere::direction(Face(f), u, v);
                const FaceCoord fc = CubeSphere::project(dir);

                // Both descriptions must map to the same direction.
                const Vec3 back = CubeSphere::direction(fc.face, fc.u, fc.v);
                const double angle = (dir - back).length();
                CHECK(angle < 1e-9, "projection round-trip drifted by %g", angle);

                // And must agree about the terrain, which is the property that
                // actually matters.
                const double h1 = t.surfaceAltitude(Face(f), u, v);
                const double h2 = t.surfaceAltitude(fc.face, fc.u, fc.v);
                worstHeight = std::max(worstHeight, std::fabs(h1 - h2));

                const Material m1 = t.materialAt(Face(f), u, v, h1 - 3.0);
                const Material m2 = t.materialAt(fc.face, fc.u, fc.v, h1 - 3.0);
                if (m1 != m2) ++materialMismatches;
                ++checked;
            }
        }
    }
    std::printf("  %d edge samples, worst height disagreement %.6f m, "
                "%d material mismatches\n", checked, worstHeight, materialMismatches);
    CHECK(worstHeight < 0.01,
          "faces disagree about terrain height across a seam by %.4f m", worstHeight);
    CHECK(materialMismatches == 0,
          "%d material mismatches across seams", materialMismatches);
}

// ---------------------------------------------------------------------------
// 3. Chunk storage — the homogeneous fast path and palette round-trip.
// ---------------------------------------------------------------------------


static void testChunk() {
    section("chunk storage (spec 12.1)");
    Chunk solid;
    solid.fill([](int, int, int) { return MAT_STONE; });
    CHECK(solid.homogeneous(), "a uniform chunk did not take the homogeneous path");
    CHECK(solid.get(5, 9, 31) == MAT_STONE, "uniform chunk read back wrong");
    CHECK(solid.bytes() < 200, "a homogeneous chunk allocated %zu bytes", solid.bytes());

    // Palette round-trip with more than 16 materials, forcing a widen().
    Chunk mixed;
    mixed.fill([](int x, int y, int z) {
        return Material(MAT_STONE + ((x + y * 3 + z * 7) % 20));
    });
    CHECK(!mixed.homogeneous(), "a mixed chunk took the homogeneous path");
    bool ok = true;
    for (int y = 0; y < kChunkSize && ok; ++y)
        for (int z = 0; z < kChunkSize && ok; ++z)
            for (int x = 0; x < kChunkSize; ++x) {
                Material want = Material(MAT_STONE + ((x + y * 3 + z * 7) % 20));
                if (mixed.get(x, y, z) != want) { ok = false; break; }
            }
    CHECK(ok, "palette chunk did not round-trip after widening");
    CHECK(mixed.paletteSize() == 20, "palette holds %d entries, expected 20",
          mixed.paletteSize());

    // A single edit to a homogeneous chunk must expand it correctly.
    Chunk edited;
    edited.fill([](int, int, int) { return MAT_STONE; });
    edited.set(4, 4, 4, MAT_AIR);
    CHECK(edited.get(4, 4, 4) == MAT_AIR, "edit was lost");
    CHECK(edited.get(4, 4, 5) == MAT_STONE, "edit clobbered a neighbour");
}

// ---------------------------------------------------------------------------
// 4. Greedy meshing — correctness, and that it actually merges.
// ---------------------------------------------------------------------------


static void testOres() {
    section("ore placement (spec 4.5, 5.2)");

    // Triangular weighting: peak at the stated depth, zero outside the spread.
    const OreDef& iron = kOres[4];
    CHECK(std::string(iron.name) == "iron", "ore table order changed");
    CHECK(depthWeight(iron, iron.peak) > 0.99, "iron weight at its own peak is not 1");
    CHECK(depthWeight(iron, iron.peak - iron.spread - 1) == 0.0,
          "iron has weight outside its spread");
    // The second lobe exists so mountains are worth climbing.
    CHECK(depthWeight(iron, 180.0) > 0.4, "iron's mountain lobe is missing");

    const OreDef& diamond = kOres[16];
    CHECK(std::string(diamond.name) == "diamond", "ore table order changed");
    CHECK(depthWeight(diamond, -380.0) > 0.99, "diamond peak is wrong");
    CHECK(depthWeight(diamond, 0.0) == 0.0, "diamond appears at sea level");

    // Mechanism 4: class abundance must actually differ between classes.
    CHECK(oreAbundance(PlanetClass::Irradiated, MAT_ORE_URANIUM) > 3.0,
          "irradiated worlds do not boost uranium");
    CHECK(oreAbundance(PlanetClass::Temperate, MAT_ORE_URANIUM) < 0.5,
          "temperate worlds do not suppress uranium");
    CHECK(oreAbundance(PlanetClass::Scorched, MAT_ORE_TUNGSTEN) > 2.0,
          "scorched worlds do not boost tungsten");

    // And ore must actually be generated, densely sampled so the result is a
    // measurement rather than a lottery. A scattered sample was the first
    // version of this test and it reported zero while the generator was
    // working — sampling 100k points 250 m apart finds nothing at realistic
    // ore densities.
    PlanetParams p = generatePlanetFromSeed(0xABCDEF);
    p.cls = PlanetClass::Temperate;
    Terrain t(p);
    std::map<Material, int> found;
    std::map<Material, double> deepest, shallowest;
    long stoneVoxels = 0;
    const double du = 1.0 / (t.sphere().resolution() * 0.5);   // ~1 m in u
    for (int i = 0; i < 26; ++i) {
        const double u = 0.10 + i * du;
        for (int j = 0; j < 26; ++j) {
            const double v = -0.22 + j * du;
            for (double alt = 220.0; alt > -500.0; alt -= 1.0) {
                Material m = t.materialAt(FACE_PZ, u, v, alt);
                if (m == MAT_STONE || m == MAT_DEEP_STONE) ++stoneVoxels;
                if (!isOre(m)) continue;
                ++found[m];
                ++stoneVoxels;
                if (!deepest.count(m) || alt < deepest[m]) deepest[m] = alt;
                if (!shallowest.count(m) || alt > shallowest[m]) shallowest[m] = alt;
            }
        }
    }
    long oreVoxels = 0;
    for (auto& kv : found) oreVoxels += kv.second;
    std::printf("  %ld stone voxels sampled, %ld ore (%.3f%%), %zu distinct types\n",
                stoneVoxels, oreVoxels, 100.0 * oreVoxels / double(stoneVoxels),
                found.size());
    for (auto& kv : found)
        std::printf("    %-16s %5d voxels  %.4f%%  band %+.0f .. %+.0f m\n",
                    materialInfo(kv.first).name, kv.second,
                    100.0 * kv.second / double(stoneVoxels),
                    deepest[kv.first], shallowest[kv.first]);

    CHECK(found.size() >= 10, "only %zu ore types generated", found.size());
    CHECK(oreVoxels * 1000 > stoneVoxels,
          "ore is %.4f%% of stone — too sparse to find while mining",
          100.0 * oreVoxels / double(stoneVoxels));
    CHECK(oreVoxels * 20 < stoneVoxels,
          "ore is %.2f%% of stone — too common to be worth finding",
          100.0 * oreVoxels / double(stoneVoxels));

    // Every ore found must lie inside the band its table entry claims. This is
    // the check that catches a depth-weighting change silently moving diamond
    // to the surface.
    for (auto& kv : found) {
        const OreDef* def = nullptr;
        for (int i = 0; i < kOreCount; ++i)
            if (kOres[i].material == kv.first) { def = &kOres[i]; break; }
        if (!def) continue;
        const double lo = std::min(def->peak - def->spread,
            def->secondLobeSpread > 0 ? def->secondLobePeak - def->secondLobeSpread : 1e9);
        const double hi = std::max(def->peak + def->spread,
            def->secondLobeSpread > 0 ? def->secondLobePeak + def->secondLobeSpread : -1e9);
        CHECK(deepest[kv.first] >= lo - 2.0 && shallowest[kv.first] <= hi + 2.0,
              "%s generated at %+.0f..%+.0f m, outside its band %+.0f..%+.0f m",
              def->name, deepest[kv.first], shallowest[kv.first], lo, hi);
    }
}

// ---------------------------------------------------------------------------
// 6. Planet generation — class weights and invariants.
// ---------------------------------------------------------------------------


static void testPlanets() {
    section("planet generation (spec 3.4)");
    std::map<PlanetClass, int> counts;
    const int N = 20000;
    for (int i = 0; i < N; ++i)
        counts[generatePlanetFromSeed(mix(uint64_t(i) * 7919)).cls]++;

    // Weights from the spec, within sampling tolerance.
    struct Expect { PlanetClass c; double pct; };
    const Expect want[] = {
        {PlanetClass::Barren, 24}, {PlanetClass::Scorched, 14},
        {PlanetClass::Frozen, 14}, {PlanetClass::Toxic, 12},
        {PlanetClass::Oceanic, 11}, {PlanetClass::Irradiated, 10},
        {PlanetClass::Temperate, 9}, {PlanetClass::Anomalous, 6},
    };
    for (const auto& w : want) {
        const double got = 100.0 * counts[w.c] / N;
        std::printf("  %-11s %5.2f%%  (spec %2.0f%%)\n",
                    planetClassName(w.c), got, w.pct);
        CHECK(std::fabs(got - w.pct) < 1.5,
              "%s at %.2f%%, spec says %.0f%%", planetClassName(w.c), got, w.pct);
    }

    // Mechanism 3: every hazardous class must map to an element, and every
    // element must be reachable. A gap here would silently disable the
    // exploration half of the elemental system.
    bool seen[6] = {false, false, false, false, false, false};
    for (int i = 0; i < 8; ++i)
        seen[int(classElement(PlanetClass(i)))] = true;
    for (int e = 1; e < 6; ++e)
        CHECK(seen[e], "element %s is not the hazard of any planet class",
              elementName(Element(e)));

    // Anomalous worlds are never claimable (the register does not know them).
    for (int i = 0; i < 400; ++i) {
        PlanetParams p = generatePlanetFromSeed(mix(uint64_t(i) * 104729));
        if (p.cls == PlanetClass::Anomalous)
            CHECK(!p.claimable, "an Anomalous world was claimable");
    }
}


// ---------------------------------------------------------------------------
// 7. Micro-voxels, level of detail, and visibility culling.
//
//    The properties that must hold for these to be worth having at all:
//      * a block that has not been carved costs nothing extra;
//      * a carved block that is filled back in costs nothing extra again;
//      * one LOD step is exactly one doubling of voxel size and one eighth of
//        the voxel count, at every level, with no drift;
//      * a sealed cavity contributes no faces, and an open one contributes all
//        of them — culling must never remove something reachable.
// ---------------------------------------------------------------------------


static void testMicroAndLod() {
    section("micro-voxels, LOD and culling");

    CHECK(kMicro == 16, "a block subdivides into %d, expected 16", kMicro);
    CHECK(kMicroVolume == 4096, "a block holds %d subunits, expected 4096",
          kMicroVolume);
    CHECK(std::fabs(kMicroSize - 0.0625) < 1e-12,
          "micro-voxels are %.6f m, expected 0.0625", kMicroSize);
    CHECK(std::fabs(lodVoxelSize(kFinestLod) - kMicroSize) < 1e-12,
          "the finest LOD is not the micro-voxel size");

    // --- Sparse detail: no storage until something is carved, and none again
    //     once it is filled back in.
    {
        Chunk c;
        c.fill([](int, int, int) { return MAT_STONE; });
        const size_t plain = c.bytes();
        CHECK(c.detailBlocks() == 0, "a fresh chunk carries %d detail volumes",
              c.detailBlocks());
        CHECK(c.micro(3, 4, 5, 0, 0, 0) == MAT_STONE,
              "a block without detail did not answer with its own material");

        c.setMicro(3, 4, 5, 1, 2, 3, MAT_AIR);
        CHECK(c.detailBlocks() == 1, "carving one block made %d detail volumes",
              c.detailBlocks());
        CHECK(c.micro(3, 4, 5, 1, 2, 3) == MAT_AIR, "the carve was lost");
        CHECK(c.micro(3, 4, 5, 1, 2, 4) == MAT_STONE,
              "the carve spread to a neighbouring subunit");
        CHECK(c.get(3, 4, 5) == MAT_STONE,
              "carving a subunit changed the whole block");
        CHECK(c.bytes() > plain, "a carved chunk did not grow at all");
        CHECK(c.detailBlocks() * kMicroVolume < kChunkVolume * kMicroVolume,
              "detail is not sparse");

        // Fill it back in: the detail volume must be released, not kept.
        c.setMicro(3, 4, 5, 1, 2, 3, MAT_STONE);
        CHECK(c.detailBlocks() == 0,
              "a block restored to uniform kept %d detail volumes",
              c.detailBlocks());
        CHECK(c.bytes() == plain, "restored chunk is %zu bytes, was %zu",
              c.bytes(), plain);
    }

    // --- A detail fill that comes out uniform must not allocate either. This
    //     is the common case underground and the reason sampling the generator
    //     at micro resolution does not explode storage.
    {
        Chunk c;
        c.fill([](int, int, int) { return MAT_AIR; });
        c.fillDetail(1, 1, 1, [](int, int, int) { return MAT_STONE; });
        CHECK(c.detailBlocks() == 0, "a uniform detail fill allocated a volume");
        CHECK(c.get(1, 1, 1) == MAT_STONE,
              "a uniform detail fill did not become the block material");

        c.fillDetail(2, 2, 2, [](int mx, int, int) {
            return mx < 8 ? MAT_STONE : MAT_AIR;
        });
        CHECK(c.detailBlocks() == 1, "a genuinely mixed detail fill was dropped");
        CHECK(c.micro(2, 2, 2, 0, 0, 0) == MAT_STONE, "detail fill lost a subunit");
        CHECK(c.micro(2, 2, 2, 15, 0, 0) == MAT_AIR, "detail fill lost a subunit");

        // Writing the whole block must discard the carving underneath it.
        c.set(2, 2, 2, MAT_DIRT);
        CHECK(c.detailBlocks() == 0, "a whole-block write left detail behind");
        CHECK(c.micro(2, 2, 2, 0, 0, 0) == MAT_DIRT, "block write did not take");
    }

    // --- The LOD rule itself: one step per doubling, exactly.
    {
        bool ladder = true;
        for (int lod = kFinestLod; lod <= 6; ++lod) {
            if (std::fabs(lodVoxelSize(lod + 1) / lodVoxelSize(lod) - 2.0) > 1e-12)
                ladder = false;
            if (std::fabs(lodVoxelDensity(lod) / lodVoxelDensity(lod + 1) - 8.0) > 1e-9)
                ladder = false;
        }
        CHECK(ladder, "the LOD ladder is not exactly one doubling per step");

        const double near = 24.0;
        CHECK(lodForDistance(near * 0.5) == kFinestLod,
              "inside the near distance the finest level was not chosen");
        bool doubling = true;
        for (int step = 0; step <= 6; ++step) {
            // Just past each doubling of the near distance, the level must
            // have advanced by exactly that many steps.
            const double d = near * std::exp2(double(step)) * 1.01;
            const int want = kFinestLod + step > 6 ? 6 : kFinestLod + step;
            if (lodForDistance(d, near) != want) doubling = false;
        }
        CHECK(doubling, "LOD does not advance one step per doubling of distance");
        CHECK(lodForDistance(1e9, near) == 6, "LOD is not clamped at the far end");

        // Half the linear density at twice the distance, stated as the design
        // states it: 2x distance -> 2x voxel edge -> 1/8 the voxels.
        CHECK(std::fabs(lodVoxelSize(lodForDistance(96.0, 24.0)) /
                        lodVoxelSize(lodForDistance(48.0, 24.0)) - 2.0) < 1e-12,
              "doubling the distance did not double the voxel size");
    }

    // --- Sampling the same ground at two levels must agree about where the
    //     ground is. Coarse levels are allowed to *lose* features thinner than
    //     a voxel — that is what LOD is — but they must not systematically
    //     move the surface, because a bias would show as the whole landscape
    //     rising or sinking as the player walks toward it.
    {
        PlanetParams p = generatePlanetFromSeed(23);
        Terrain t(p);
        const double mpu = p.radius * 3.14159265358979 / 4.0;
        double bias = 0.0, worst = 0.0;
        int agree = 0, total = 0;
        for (int i = 0; i < 256; ++i) {
            const double u = (i % 16 - 8) * 5.0 / mpu;
            const double v = (i / 16 - 8) * 5.0 / mpu;
            const Terrain::Column col = t.columnAt(FACE_PZ, u, v);
            double surf[2] = {0, 0};
            const double sizes[2] = {1.0, 0.0625};
            for (int k = 0; k < 2; ++k) {
                const double s = sizes[k];
                for (int step = 0; step < int(160.0 / s); ++step) {
                    const double a = col.surface + 40.0 - (step + 0.5) * s;
                    if (isSolid(t.materialInColumn(col, a, s))) { surf[k] = a; break; }
                }
            }
            const double diff = surf[1] - surf[0];
            bias += diff;
            worst = std::max(worst, std::fabs(diff));
            if (std::fabs(diff) <= 1.0 + 1e-9) ++agree;
            ++total;
        }
        bias /= double(total);
        // There *is* an expected offset, and it is worth pinning down rather
        // than wishing away. A voxel is solid if its centre is below the true
        // surface, so the coarse surface sits on average half a coarse voxel
        // low and the fine one half a fine voxel low: the fine ground should
        // read about +0.47 m higher, never lower, and never a whole voxel
        // higher. This is why ground appears to rise slightly as a player
        // walks toward it and the level refines — a real artifact, bounded
        // here so that it can never grow into a step.
        CHECK(bias > 0.25 && bias < 0.75,
              "fine sampling puts the surface %+.3f m from coarse sampling on"
              " average; expected about +0.47 m (half a coarse voxel)", bias);
        // And the disagreements must be the exception, not the rule.
        CHECK(agree * 100 >= total * 88,
              "only %d of %d columns agreed within one coarse voxel"
              " (worst case %.2f m)", agree, total, worst);
        std::printf("    surface agreement 1 m vs 1/16 m: %d/%d within one"
                    " coarse voxel, bias %+.3f m, worst %.2f m\n",
                    agree, total, bias, worst);
    }
}


// ---------------------------------------------------------------------------
// 8. The render layer: matrices, culling, vertex packing, shading, streaming.
//
//    None of this needs a GPU, and that is the point. The Vulkan backend is a
//    few hundred lines of API calls around these; everything that can actually
//    be wrong in an interesting way — a sign in a projection, a plane pointing
//    the wrong way, a vertex quantised off by one, a tile thrashing between
//    levels — is decided here and tested here.
// ---------------------------------------------------------------------------


int main() {
    std::printf("Elysium — world tests\n");
    testDeterminism();
    testSeam();
    testChunk();
    testOres();
    testPlanets();
    testMicroAndLod();
    return elytest::report("world tests");
}
