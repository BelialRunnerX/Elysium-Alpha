// Survey generated star systems from a galaxy seed — the galaxy layer's
// equivalent of planetmap, and a fast way to sanity-check class weights,
// planet-count distribution and the radial gradient without a UI.
//
//   systemsurvey <galaxySeed> [systemCount]
#include "world/terrain.h"
#include <cstdio>
#include <cstdlib>
#include <map>

using namespace ely;

int main(int argc, char** argv) {
    const uint64_t galaxy = argc > 1 ? std::strtoull(argv[1], nullptr, 0) : 1;
    const int count = argc > 2 ? std::atoi(argv[2]) : 12;

    std::printf("galaxy %llu — first %d systems\n\n",
                (unsigned long long)galaxy, count);

    std::map<PlanetClass, int> classCounts;
    int totalPlanets = 0, claimable = 0, temperate = 0;

    for (int s = 0; s < count; ++s) {
        const uint64_t sys = systemSeed(galaxy, uint32_t(s));
        Rng rng(mix(sys ^ labelHash("layout")));
        // Planets per system: 8/22/27/22/14/7 % for 1..6 (spec §3.1).
        const int table[] = {8, 22, 27, 22, 14, 7};
        int roll = int(rng.below(100)), n = 1;
        for (int i = 0; i < 6; ++i) { if (roll < table[i]) { n = i + 1; break; } roll -= table[i]; }

        std::printf("system %-4d  %d planet%s\n", s, n, n == 1 ? "" : "s");
        for (int i = 0; i < n; ++i) {
            PlanetParams p = generatePlanet(sys, uint32_t(i));
            ++classCounts[p.cls];
            ++totalPlanets;
            if (p.claimable) ++claimable;
            if (p.cls == PlanetClass::Temperate) ++temperate;
            std::printf("   %d  %-11s r=%4.0fm  g=%.2f  %-13s haz %d  %s%s\n",
                        i, planetClassName(p.cls), p.radius, p.gravity,
                        hazardName(p.hazard()), p.hazardIntensity,
                        p.dayLength == 0.0 ? "tide-locked" : "rotating",
                        p.claimable ? "" : "  [unclaimable]");
        }
    }

    std::printf("\n%d planets across %d systems\n", totalPlanets, count);
    for (auto& kv : classCounts)
        std::printf("  %-11s %3d  %5.1f%%\n", planetClassName(kv.first), kv.second,
                    100.0 * kv.second / totalPlanets);
    std::printf("  claimable   %3d  %5.1f%%\n", claimable, 100.0 * claimable / totalPlanets);
    std::printf("  temperate   %3d  %5.1f%%  (the ones worth living on)\n",
                temperate, 100.0 * temperate / totalPlanets);
    return 0;
}
