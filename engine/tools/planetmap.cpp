// Render a whole planet as maps, without launching anything.
//
// Principle S6: every generation stage must be inspectable outside the game.
// This renders the climate fields, the height field, the biome assignment and
// a shaded globe, for any seed, in a couple of seconds.
//
//   planetmap <seed> [outdir]
#include "image/image.h"
#include "world/terrain.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

using namespace ely;

namespace {

// Equirectangular projection: longitude across, latitude down. Every pixel is
// a direction, which is exactly what the generator wants (see the seam note in
// cubesphere.h), so there is no face handling here at all.
Vec3 dirFromLatLon(double lat, double lon) {
    const double cl = std::cos(lat);
    return Vec3{cl * std::sin(lon), std::sin(lat), cl * std::cos(lon)};
}

void shade(uint8_t& r, uint8_t& g, uint8_t& b, double f) {
    r = uint8_t(clampd(r * f, 0, 255));
    g = uint8_t(clampd(g * f, 0, 255));
    b = uint8_t(clampd(b * f, 0, 255));
}

}  // namespace

int main(int argc, char** argv) {
    const uint64_t seed = argc > 1 ? std::strtoull(argv[1], nullptr, 0) : 12345;
    const std::string out = argc > 2 ? argv[2] : ".";

    PlanetParams p = generatePlanetFromSeed(seed);
    Terrain t(p);

    std::printf("planet %llu\n", (unsigned long long)seed);
    std::printf("  class       %s  (%s hazard, %s aligned)\n",
                planetClassName(p.cls), hazardName(p.hazard()),
                elementName(p.element()));
    std::printf("  radius      %.0f m\n", p.radius);
    std::printf("  gravity     %.2f g\n", p.gravity);
    if (p.dayLength == 0.0) std::printf("  day         tide-locked\n");
    else                    std::printf("  day         %.0f min\n", p.dayLength);
    std::printf("  sea level   %+.0f m  (%.0f%% of relief)\n",
                t.seaLevel(), p.seaLevelFrac * 100);
    std::printf("  hazard      intensity %d\n", p.hazardIntensity);
    std::printf("  flora/fauna %d / %d\n", p.floraDensity, p.faunaDensity);
    std::printf("  claimable   %s\n", p.claimable ? "yes" : "no (Anomalous)");

    const int W = 720, H = 360;
    Image height(W, H), biome(W, H), climateImg(W, H);
    std::map<Biome, int> biomeCounts;

    std::vector<double> h(size_t(W) * H);
    std::vector<Biome> bio(size_t(W) * H);
    for (int y = 0; y < H; ++y) {
        const double lat = (0.5 - (y + 0.5) / H) * 3.14159265358979;
        for (int x = 0; x < W; ++x) {
            const double lon = ((x + 0.5) / W - 0.5) * 2 * 3.14159265358979;
            const Vec3 d = dirFromLatLon(lat, lon);
            const Climate c = t.climate(d);
            const double a = t.surfaceAltitude(d, c);
            h[size_t(y) * W + x] = a;
            bio[size_t(y) * W + x] = t.biomeAt(c, a);
            ++biomeCounts[bio[size_t(y) * W + x]];

            // Climate as false colour: red temperature, green humidity, blue
            // continentalness. Three fields in one image, which is enough to
            // see whether any of them has gone flat or banded.
            climateImg.set(x, y,
                uint8_t((c.temperature * 0.5 + 0.5) * 255),
                uint8_t((c.humidity * 0.5 + 0.5) * 255),
                uint8_t((c.continentalness * 0.5 + 0.5) * 255));
        }
    }

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            const double a = h[size_t(y) * W + x];
            const int xr = (x + 1) % W, yl = y > 0 ? y - 1 : y;
            const double dx = h[size_t(y) * W + xr] - a;
            const double dy = h[size_t(yl) * W + x] - a;
            const double slope = clampd(1.0 - (dx * 0.5 + dy * 0.5) * 0.03, 0.55, 1.45);

            uint8_t r, g, b;
            if (a < t.seaLevel() && p.seaLevelFrac > 0.0) {
                const double dep = clampd((t.seaLevel() - a) / 220.0, 0, 1);
                r = uint8_t(30 + (1 - dep) * 40);
                g = uint8_t(70 + (1 - dep) * 70);
                b = uint8_t(120 + (1 - dep) * 90);
            } else {
                const double up = clampd((a - t.seaLevel()) / 260.0, 0, 1);
                r = uint8_t(70 + up * 165);
                g = uint8_t(95 + up * 140);
                b = uint8_t(60 + up * 150);
                shade(r, g, b, slope);
            }
            height.set(x, y, r, g, b);

            uint8_t br, bg, bb;
            biomeColour(bio[size_t(y) * W + x], br, bg, bb);
            shade(br, bg, bb, clampd(slope, 0.7, 1.25));
            biome.set(x, y, br, bg, bb);
        }
    }

    // An orthographic globe. This is the image that confirms the world really
    // is a sphere and that no cube edge is visible anywhere on it.
    const int G = 420;
    Image globe(G, G);
    globe.fill(12, 12, 16);
    const Vec3 light = Vec3{-0.4, 0.5, 0.75}.normalised();
    for (int y = 0; y < G; ++y) {
        for (int x = 0; x < G; ++x) {
            const double sx = (x + 0.5) / G * 2 - 1;
            const double sy = 1 - (y + 0.5) / G * 2;
            const double r2 = sx * sx + sy * sy;
            if (r2 > 1.0) continue;
            const double sz = std::sqrt(1.0 - r2);
            // Tilted so a pole and a cube corner are both in view.
            const double ty = sy * 0.9 - sz * 0.436;
            const double tz = sy * 0.436 + sz * 0.9;
            const Vec3 d = Vec3{sx, ty, tz}.normalised();
            const Climate c = t.climate(d);
            const double a = t.surfaceAltitude(d, c);
            uint8_t br, bg, bb;
            biomeColour(t.biomeAt(c, a), br, bg, bb);
            const double lam = clampd(d.dot(light) * 0.85 + 0.35, 0.18, 1.25);
            shade(br, bg, bb, lam);
            globe.set(x, y, br, bg, bb);
        }
    }

    const std::string tag = out + "/planet_" + std::to_string(seed);
    height.writePng(tag + "_height.png");
    biome.writePng(tag + "_biome.png");
    climateImg.writePng(tag + "_climate.png");
    globe.writePng(tag + "_globe.png");

    std::printf("  biomes:\n");
    for (auto& kv : biomeCounts)
        std::printf("    %-20s %5.1f%%\n", biomeName(kv.first),
                    100.0 * kv.second / double(W * H));
    std::printf("  wrote 4 images\n");
    return 0;
}
