// Where does generation time actually go?
//
// The engine has exactly one performance problem and this tool is how it gets
// argued about with numbers. It measures the three things that matter
// separately, because they have completely different fixes:
//
//   * per-column work   (climate, and the descent scan for the surface)
//   * per-voxel work    (density, caves, materials, ore)
//   * whole-tile build  (what the streamer actually waits on)
//
// It also empirically bounds the noise range, which is not curiosity: the
// surface-finding scan can only be started near the terrain height rather than
// at the top of the world if there is a proven bound on how far the 3D terms
// can lift rock above it. Guessing that bound would put holes in the sky.
//
//   genbench [seed] [samples]
#include "core/noise.h"
#include "gfx/tilebuild.h"
#include "jobs/jobsystem.h"
#include "world/lod.h"
#include "world/terrain.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <thread>
#include <vector>

using namespace ely;

namespace {
constexpr double kPi = 3.14159265358979;

double now() {
    using clock = std::chrono::steady_clock;
    return std::chrono::duration<double, std::milli>(
        clock::now().time_since_epoch()).count();
}
}  // namespace

int main(int argc, char** argv) {
    const uint64_t seed = argc > 1 ? std::strtoull(argv[1], nullptr, 0) : 23;
    const int samples = argc > 2 ? std::atoi(argv[2]) : 400000;

    PlanetParams p = generatePlanetFromSeed(seed);
    Terrain t(p);
    const double mpu = p.radius * kPi / 4.0;

    std::printf("planet %llu (%s), radius %.0f m\n",
                (unsigned long long)seed, planetClassName(p.cls), p.radius);

    // --- 1. How large can a single fbm actually get?
    //
    // The theoretical bound on this gradient construction is loose. What the
    // surface scan needs is the real one, measured over the frequencies and
    // octave counts the density field actually uses.
    {
        Noise n(0x9E3779B97F4A7C15ull);
        double worst3 = 0, worst2 = 0;
        Rng rng(12345);
        for (int i = 0; i < samples; ++i) {
            const Vec3 v(rng.range(-4000.0f, 4000.0f), rng.range(-4000.0f, 4000.0f),
                         rng.range(-4000.0f, 4000.0f));
            worst3 = std::max(worst3, std::fabs(n.fbm(v / 180.0, 3)));
            worst2 = std::max(worst2, std::fabs(n.fbm(v / 45.0, 2)));
        }
        // density() adds nLarge.fbm(.,3) * 2.2 and nSmall.fbm(.,2) * 0.9, both
        // scaled by a near-surface weight that is at most 1.
        const double lift = worst3 * 2.2 + worst2 * 0.9;
        std::printf("\nnoise range over %d samples\n", samples);
        std::printf("  |fbm 3 octaves|  max %.4f\n", worst3);
        std::printf("  |fbm 2 octaves|  max %.4f\n", worst2);
        std::printf("  worst lift of the density field above its base: %.3f\n", lift);
        std::printf("  x max squash (3.2) = %.2f m of rock possible above the"
                    " terrain height\n", lift * 3.2);
    }

    // --- 2. Per-column cost, and how much of it is the surface scan.
    {
        const int N = 64;
        const double t0 = now();
        double sink = 0;
        for (int j = 0; j < N; ++j)
            for (int i = 0; i < N; ++i) {
                const double u = (i - N * 0.5) * 1.0 / mpu;
                const double v = (j - N * 0.5) * 1.0 / mpu;
                sink += t.climate(CubeSphere::direction(FACE_PZ, u, v)).temperature;
            }
        const double t1 = now();
        for (int j = 0; j < N; ++j)
            for (int i = 0; i < N; ++i) {
                const double u = (i - N * 0.5) * 1.0 / mpu;
                const double v = (j - N * 0.5) * 1.0 / mpu;
                sink += t.columnAt(FACE_PZ, u, v).surface;
            }
        const double t2 = now();

        const double columns = double(N) * N;
        std::printf("\nper column (%d columns)\n", int(columns));
        std::printf("  climate only        %7.2f us\n", (t1 - t0) * 1000 / columns);
        std::printf("  climate + surface   %7.2f us\n", (t2 - t1) * 1000 / columns);
        std::printf("  the surface scan is %.0f%% of a column\n",
                    100.0 * ((t2 - t1) - (t1 - t0)) / (t2 - t1));
        if (sink == 12345.6789) std::printf(" ");  // keep the work alive
    }

    // --- 3. Per-voxel cost, split by where the voxel is.
    //
    // These are wildly different and averaging them hides the fix: a voxel in
    // open sky, a voxel near the surface and a voxel deep in rock do very
    // different amounts of work.
    {
        const Terrain::Column col = t.columnAt(FACE_PZ, 0, 0);
        struct Band { const char* name; double lo, hi; };
        const Band bands[] = {
            {"open sky   (surface + 30..90 m)", col.surface + 30, col.surface + 90},
            {"at surface (surface -10..+10 m)", col.surface - 10, col.surface + 10},
            {"shallow    (surface -90..-30 m)", col.surface - 90, col.surface - 30},
            {"deep       (-300..-240 m)",       -300, -240},
        };
        std::printf("\nper voxel, by band (%d samples each)\n", samples / 4);
        for (const Band& b : bands) {
            const double t0 = now();
            int solid = 0;
            for (int i = 0; i < samples / 4; ++i) {
                const double a = b.lo + (b.hi - b.lo) * (i % 1024) / 1024.0;
                if (isSolid(t.materialInColumn(col, a, 1.0))) ++solid;
            }
            const double us = (now() - t0) * 1000 / (samples / 4);
            std::printf("  %-34s %6.3f us   %d%% solid\n", b.name, us,
                        solid * 400 / samples);
        }
    }

    // --- 4. What the streamer waits on: a whole tile.
    {
        std::printf("\nwhole tile, 32 x 96 x 32 m\n");
        for (int lod = 0; lod <= 3; ++lod) {
            const double vs = lodVoxelSize(lod);
            const int nx = int(32.0 / vs), ny = int(96.0 / vs), nz = int(32.0 / vs);
            const double t0 = now();
            long solid = 0;
            for (int z = 0; z < nz; ++z)
                for (int x = 0; x < nx; ++x) {
                    const double u = ((x + 0.5) * vs - 16.0) / mpu;
                    const double v = ((z + 0.5) * vs - 16.0) / mpu;
                    const Terrain::Column col = t.columnAt(FACE_PZ, u, v);
                    for (int y = 0; y < ny; ++y)
                        if (isSolid(t.materialInColumn(
                                col, col.surface - 48.0 + (y + 0.5) * vs, vs)))
                            ++solid;
                }
            const double ms = now() - t0;
            std::printf("  lod %d  %4.2f m voxels  %3dx%3dx%3d = %8ld voxels"
                        "  %8.1f ms  %6.3f us/voxel\n",
                        lod, vs, nx, ny, nz, long(nx) * ny * nz, ms,
                        ms * 1000.0 / (double(nx) * ny * nz));
            (void)solid;
        }
    }
    // --- 5. Does the pool actually scale, and does threading change the world?
    //
    // Two questions in one measurement, and the second matters more. A job
    // system that is fast and non-deterministic is worse than no job system:
    // this engine's whole contract is that a seed names a world (S2), and
    // threading is the easiest way to break it.
    {
        std::printf("\nparallel tile build (32 x 96 x 32 m at 1 m), %u cores"
                    " available\n", std::thread::hardware_concurrency());

        TileSpec spec;
        spec.face = FACE_PZ;
        spec.uCentre = 0.0;
        spec.vCentre = 0.0;
        spec.ox = -16.0;
        spec.oz = -16.0;
        spec.oy = t.surfaceAltitude(FACE_PZ, 0, 0) - 48.0;
        spec.sizeX = 32.0; spec.sizeY = 96.0; spec.sizeZ = 32.0;
        spec.lod = 0;

        std::vector<GpuVertex> reference;
        double serialMs = 0;
        for (int workers : {0, 1, 2, 4, 8}) {
            JobSystem jobs(workers);
            const double t0 = now();
            BuiltTile built = buildTile(t, spec, &jobs);
            const double ms = now() - t0;
            if (workers == 0) {
                serialMs = ms;
                reference = built.gpu.vertices;
            }
            const bool identical = built.gpu.vertices.size() == reference.size() &&
                std::equal(built.gpu.vertices.begin(), built.gpu.vertices.end(),
                           reference.begin(),
                           [](const GpuVertex& a, const GpuVertex& b) {
                               return a.x == b.x && a.y == b.y && a.z == b.z &&
                                      a.material == b.material &&
                                      a.aoAxis == b.aoAxis;
                           });
            std::printf("  %2d workers  %7.1f ms  %5.2fx  %s\n",
                        workers, ms, serialMs / ms,
                        identical ? "identical output"
                                  : "*** OUTPUT DIFFERS — DETERMINISM BROKEN ***");
        }
        std::printf("  (speedup is against 0 workers, which runs the column walk"
                    " on the calling thread.\n   The ceiling is the core count;"
                    " adding workers past it buys nothing.)\n");
    }

    return 0;
}
