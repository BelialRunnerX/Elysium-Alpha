// A vertical slice through a planet's crust: terrain, caves, ore, fluids.
//
// This is the view that makes the underground legible. A height map cannot
// show a cave; this can, and it is how you tell at a glance whether the cave
// systems connect, whether ore is sitting in its stated band, and whether the
// deep column is solid enough to tunnel through rather than fall into.
//
//   crosssection <seed> [outdir] [width_m] [--face N] [--u U]
#include "image/image.h"
#include "world/terrain.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

using namespace ely;

int main(int argc, char** argv) {
    const uint64_t seed = argc > 1 ? std::strtoull(argv[1], nullptr, 0) : 12345;
    const std::string out = argc > 2 ? argv[2] : ".";
    double widthMetres = argc > 3 ? std::atof(argv[3]) : 512.0;
    int face = FACE_PZ;
    double uFixed = 0.0;
    for (int i = 4; i < argc - 1; ++i) {
        if (!std::strcmp(argv[i], "--face")) face = std::atoi(argv[i + 1]);
        if (!std::strcmp(argv[i], "--u"))    uFixed = std::atof(argv[i + 1]);
    }

    PlanetParams p = generatePlanetFromSeed(seed);
    Terrain t(p);

    // One pixel per metre, both axes: the slice is to scale, which matters
    // because "is that cave big enough to walk through" is a real question.
    const int H = int(PlanetParams::kMaxAltitude - PlanetParams::kMinAltitude);
    const int W = int(widthMetres);
    Image img(W, H);
    img.fill(10, 10, 14);

    // Metres per unit of v at this radius.
    const double metresPerV = p.radius * 3.14159265358979 / 4.0;
    std::map<Material, int> counts;

    for (int x = 0; x < W; ++x) {
        const double v = (double(x) - W * 0.5) / metresPerV;
        for (int y = 0; y < H; ++y) {
            const double alt = PlanetParams::kMaxAltitude - y;
            const Material m = t.materialAt(Face(face), uFixed, v, alt);
            ++counts[m];
            const MaterialInfo& mi = materialInfo(m);
            uint8_t r = mi.r, g = mi.g, b = mi.b;
            if (m == MAT_AIR) {
                // Tint air above and below the surface differently so the
                // cave systems read as caves rather than as holes.
                const bool sky = alt > t.surfaceAltitude(Face(face), uFixed, v);
                r = sky ? 40 : 18; g = sky ? 48 : 18; b = sky ? 62 : 24;
            } else if (isOre(m)) {
                // Ore is drawn at full brightness with a halo so a two-voxel
                // vein is still visible at this scale.
                for (int dy = -1; dy <= 1; ++dy)
                    for (int dx = -1; dx <= 1; ++dx)
                        img.set(x + dx, y + dy,
                                uint8_t(r * 0.55), uint8_t(g * 0.55), uint8_t(b * 0.55));
            }
            img.set(x, y, r, g, b);
        }
        // Sea level and the deep-stone boundary as faint rules.
        img.set(x, int(PlanetParams::kMaxAltitude - t.seaLevel()), 90, 130, 190);
        img.set(x, int(PlanetParams::kMaxAltitude + 256.0), 70, 70, 78);
    }

    img.writePng(out + "/section_" + std::to_string(seed) + ".png");

    long solid = 0, air = 0, ore = 0;
    for (auto& kv : counts) {
        if (kv.first == MAT_AIR) air += kv.second;
        else if (isSolid(kv.first)) solid += kv.second;
        if (isOre(kv.first)) ore += kv.second;
    }
    std::printf("section of %s planet %llu: %d x %d m\n",
                planetClassName(p.cls), (unsigned long long)seed, W, H);
    std::printf("  solid %ld  air %ld (%.1f%% void)  ore %ld (%.3f%% of solid)\n",
                solid, air, 100.0 * air / double(solid + air),
                ore, 100.0 * ore / double(solid));
    for (auto& kv : counts)
        if (isOre(kv.first))
            std::printf("    %-16s %d\n", materialInfo(kv.first).name, kv.second);
    return 0;
}
