#include "terrain.h"
#include <algorithm>
#include <cmath>

namespace ely {

namespace {

// Climate field wavelengths in metres (spec §4.2). Sampling is in metres of
// arc on the nominal sphere, so a wavelength means the same thing on a small
// planet as on a large one — which is what stops a 1.2 km world from being one
// continent and a 6.4 km world from being noise.
constexpr double kWaveCont  = 4000.0;
constexpr double kWaveEros  = 2600.0;
constexpr double kWaveRidge = 1400.0;
constexpr double kWaveTemp  = 3200.0;
constexpr double kWaveHumid = 2200.0;
constexpr double kWaveOdd   = 5000.0;

constexpr double kBiomeUnknown = -9999.0;

}  // namespace

const char* biomeName(Biome b) {
    switch (b) {
        case BIOME_OCEAN: return "ocean";
        case BIOME_BEACH: return "beach";
        case BIOME_PLAINS: return "plains";
        case BIOME_FOREST: return "forest";
        case BIOME_BOREAL: return "boreal forest";
        case BIOME_RAINFOREST: return "rainforest";
        case BIOME_SAVANNA: return "savanna";
        case BIOME_DESERT: return "desert";
        case BIOME_BADLANDS: return "badlands";
        case BIOME_TUNDRA: return "tundra";
        case BIOME_HIGHLANDS: return "highlands";
        case BIOME_WETLAND: return "wetland";
        case BIOME_KARST: return "karst";
        case BIOME_REGOLITH: return "regolith plain";
        case BIOME_IMPACT_BASIN: return "impact basin";
        case BIOME_BASALT: return "basalt plain";
        case BIOME_MAGMA_FIELD: return "magma field";
        case BIOME_ICE_SHEET: return "ice sheet";
        case BIOME_SUBGLACIAL: return "sub-glacial ocean";
        case BIOME_SPORE_PLAIN: return "spore plain";
        case BIOME_SPORE_DEEPS: return "spore deeps";
        case BIOME_GLASS_SEA: return "glass sea";
        case BIOME_DEAD_CITY: return "dead city";
        case BIOME_OPEN_OCEAN: return "open ocean";
        case BIOME_ABYSSAL_TRENCH: return "abyssal trench";
        default: return "?";
    }
}

void biomeColour(Biome b, uint8_t& r, uint8_t& g, uint8_t& bl) {
    struct C { uint8_t r, g, b; };
    static const C k[BIOME_COUNT] = {
        { 42,  82, 148}, {214, 200, 152}, {124, 168,  92}, { 70, 128,  62},
        { 58,  96,  76}, { 44, 122,  60}, {176, 168,  86}, {214, 190, 128},
        {178, 116,  76}, {224, 230, 234}, {150, 150, 154}, { 86, 122, 104},
        {158, 154, 138},
        {142, 136, 128}, {108, 104,  98},
        { 62,  60,  66}, {198,  92,  40},
        {216, 232, 244}, { 70, 118, 158},
        {124, 140,  92}, { 96,  76, 124},
        {170, 178, 190}, {110, 112, 122},
        { 34,  70, 132}, { 16,  34,  76},
    };
    r = k[b].r; g = k[b].g; bl = k[b].b;
}

Terrain::Terrain(const PlanetParams& params)
    : p_(params),
      sphere_(params.radius),
      nCont_ (mix(params.seed ^ labelHash("continentalness"))),
      nEros_ (mix(params.seed ^ labelHash("erosion"))),
      nRidge_(mix(params.seed ^ labelHash("ridges"))),
      nTemp_ (mix(params.seed ^ labelHash("temperature"))),
      nHumid_(mix(params.seed ^ labelHash("humidity"))),
      nOdd_  (mix(params.seed ^ labelHash("oddity"))),
      nLarge_(mix(params.seed ^ labelHash("large"))),
      nSmall_(mix(params.seed ^ labelHash("small"))),
      nCaveA_(mix(params.seed ^ labelHash("caveA"))),
      nCaveB_(mix(params.seed ^ labelHash("caveB"))),
      nChamber_(mix(params.seed ^ labelHash("chamber"))),
      nFissure_(mix(params.seed ^ labelHash("fissure"))),
      nOre_  (mix(params.seed ^ labelHash("ore"))),
      nAquifer_(mix(params.seed ^ labelHash("aquifer")))
{
    // Terrain height range, scaled per planet so two worlds of a class differ.
    reliefLo_ = -400.0 * p_.reliefScale;
    reliefHi_ =  240.0 * p_.reliefScale;
    reliefLo_ = std::max(reliefLo_, PlanetParams::kMinAltitude + 40.0);
    reliefHi_ = std::min(reliefHi_, PlanetParams::kMaxAltitude - 8.0);

    seaLevel_ = reliefLo_ + (reliefHi_ - reliefLo_) * p_.seaLevelFrac;

    oreConst_.resize(size_t(kOreCount));
    for (int i = 0; i < kOreCount; ++i) {
        const OreDef& o = kOres[i];
        oreConst_[size_t(i)].veinDensity = veinDensity(o);
        oreConst_[size_t(i)].abundance = oreAbundance(p_.cls, o.material);
        oreConst_[size_t(i)].maxReach = std::cbrt(double(o.veinMax) * 0.2387324146);
        oreConst_[size_t(i)].indexMix = mix(uint64_t(i) * 0x9E3779B97F4A7C15ull);
    }
}

Climate Terrain::climate(const Vec3& dir) const {
    // Climate is a map on the sea-level shell: it does not vary with depth.
    const Vec3 fp = dir * p_.radius;
    Climate c;
    c.continentalness = clampd(
        nCont_.fbm(fp / kWaveCont, 4) * 1.55 + p_.oceanBias, -1.0, 1.0);
    c.erosion   = clampd(nEros_.fbm(fp / kWaveEros, 3) * 1.7, -1.0, 1.0);
    c.ridges    = clampd(nRidge_.ridged(fp / kWaveRidge, 4), -1.0, 1.0);
    c.humidity  = clampd(nHumid_.fbm(fp / kWaveHumid, 3) * 1.6, -1.0, 1.0);
    c.oddity    = clampd(nOdd_.fbm(fp / kWaveOdd, 2) * 1.4, -1.0, 1.0);

    // Temperature is noise plus a latitude term scaled by axial tilt. A planet
    // with zero tilt has banded climate; a heavily tilted one is patchy.
    const double lat = std::fabs(fp.normalised().y);       // 0 equator .. 1 pole
    const double band = 1.0 - lat * 2.0;
    const double tiltMix = 0.35 + 0.5 * std::cos(p_.axialTilt);
    c.temperature = clampd(
        band * tiltMix + nTemp_.fbm(fp / kWaveTemp, 3) * (1.0 - tiltMix) * 1.8,
        -1.0, 1.0);
    return c;
}

double Terrain::terrainHeight(const Climate& c) const {
    // Continentalness is the primary land/sea split, shaped rather than linear
    // so that shelf, coast and inland each get their own band instead of the
    // whole range being one ramp.
    //
    // The bands are expressed as fractions of the available headroom above and
    // depth below sea level, never as absolute metres. The first version used
    // absolutes (sea level + 24 m, and so on), which works while sea level sits
    // in the middle of the relief range and breaks completely when it does not:
    // on an ocean world with sea level at 90% of relief, "sea level + 24" is
    // above the ceiling, so every column came out as land and a global ocean
    // rendered as a beach.
    const double head = std::max(12.0, reliefHi_ - seaLevel_);
    const double deep = std::max(12.0, seaLevel_ - reliefLo_);

    // The continentalness value at which land begins.
    //
    // This is what makes seaLevelFrac mean "how much of the world is ocean"
    // rather than merely "where the waterline sits in the height range". With
    // a fixed split, an ocean world with a high waterline still had land
    // wherever continentalness was above zero, and rendered as a beach planet:
    // 70% beach, 21% ocean. Moving the *threshold* with the fraction fixes it
    // at the distribution level, where the problem actually is.
    //
    // Continentalness is roughly normal with a standard deviation near 0.28,
    // so the range below spans about 80% land at one end to 6% at the other.
    const double kSea = lerpd(-0.35, 0.45, p_.seaLevelFrac);

    double base;
    const double k = c.continentalness;
    if (k < kSea) {
        // Below the waterline: abyss -> shelf -> coast.
        const double tt = (k + 1.0) / std::max(0.05, kSea + 1.0);   // 0..1
        if (tt < 0.45)      base = lerpd(seaLevel_ - deep, seaLevel_ - deep * 0.42, tt / 0.45);
        else if (tt < 0.85) base = lerpd(seaLevel_ - deep * 0.42, seaLevel_ - deep * 0.03,
                                         (tt - 0.45) / 0.40);
        else                base = lerpd(seaLevel_ - deep * 0.03, seaLevel_, (tt - 0.85) / 0.15);
    } else {
        // Above it: coast -> inland -> upland.
        const double tt = (k - kSea) / std::max(0.05, 1.0 - kSea);  // 0..1
        base = lerpd(seaLevel_, reliefHi_, tt * tt * 0.85 + tt * 0.15);
    }

    // Erosion flattens. High erosion pulls toward the local base level; low
    // erosion lets the ridge term express itself.
    const double flat = (c.erosion + 1.0) * 0.5;             // 0..1
    base += c.ridges * (1.0 - flat) * head * 0.55;

    // Above sea level, erosion also damps absolute relief, so an eroded
    // continent is plains rather than eroded mountains.
    if (base > seaLevel_) base = seaLevel_ + (base - seaLevel_) * (1.0 - flat * 0.55);
    return clampd(base, PlanetParams::kMinAltitude + 24.0,
                        PlanetParams::kMaxAltitude - 4.0);
}

double Terrain::caveCarve(const Vec3& dir, double altitude) const {
    // Returns 1.0 where rock is carved away, 0.0 where it is untouched.
    //
    // Caves are a *mask* applied after the density test, not a term inside it.
    // The spec describes them as a subtraction, and the first implementation
    // did that literally — and produced a planet with no caves anywhere, for
    // a reason worth recording: the density term is (surfaceHeight - altitude)
    // / squash, which grows without bound with depth. Two hundred metres down
    // it is of order 130, and no plausible carve value subtracts against that.
    // A cave system that only works in the top ten metres is not a cave system.
    //
    // As a mask the two concerns separate cleanly: density decides where rock
    // is, this decides where it has been removed, and neither has to be
    // calibrated against the other's magnitude.
    if (altitude > 140.0 || altitude < PlanetParams::kMinAltitude + 14.0) return 0.0;
    const Vec3 fp = volumePos(dir, altitude);

    // Tunnels. Two independent fields, carved where BOTH are near zero — the
    // intersection of two near-zero isosurfaces is a curve, which is exactly
    // the long, thin, winding, connected shape wanted. Plain fbm, not ridged:
    // ridged noise is biased away from zero, so the intersection almost never
    // happens, which is the other half of why the first version was empty.
    // Slightly wider with depth: the deeps should feel more open than the
    // shallows, and it gives a reason to keep descending.
    const double widen = 1.0 + clampd((-altitude) / 420.0, 0.0, 1.0) * 0.45;
    const double tunnelR = 0.042 * widen;
    const double tunnelR2 = tunnelR * tunnelR;

    // a*a + b*b < r*r cannot hold if a*a already exceeds r*r, so the second
    // field — three more noise samples — is only evaluated when the first has
    // not already ruled the point out. The tunnel radius is about 0.06 against
    // a field range near 0.7, so this skips the second sample for the large
    // majority of voxels. Exact: the branch changes nothing about which points
    // are carved.
    static thread_local NoiseCell cA[3], cB[3], cChamber[3], cFissure[2];
    const double a = nCaveA_.fbm(fp / 78.0, 3, cA);
    if (a * a < tunnelR2) {
        const double b = nCaveB_.fbm(fp / 78.0, 3, cB);
        if (a * a + b * b < tunnelR2) return 1.0;
    }

    // Chambers. Low-frequency noise above a threshold, weighted to the middle
    // deeps where they read as landmarks rather than as noise.
    const double depthBias =
        clampd(1.0 - std::fabs(altitude + 210.0) / 260.0, 0.0, 1.0);
    if (depthBias > 0.0) {
        const double ch = nChamber_.fbm(fp / 165.0, 3, cChamber);
        if (ch > 0.50 - 0.12 * depthBias) return 1.0;
    }

    // Fissures. Near-vertical slots that break the surface, sampled with a
    // heavily squashed radial axis so they run down rather than across. These
    // are the way in that a player did not have to dig.
    if (altitude > -200.0) {
        const Vec3 sq{fp.x, fp.y * 0.14, fp.z};
        const double fz = nFissure_.fbm(sq / 62.0, 2, cFissure);
        if (std::fabs(fz) < 0.020) return 1.0;
    }
    return 0.0;
}

double Terrain::density(const Vec3& dir, double altitude, const Climate& c) const {
    return density(dir, altitude, terrainHeight(c), columnSquash(c));
}

// The same field, with the two per-column constants passed in.
//
// terrainHeight() is a function of the climate alone, so it is the same value
// for every voxel in a column and for every step of the surface scan — and it
// was being recomputed for all of them. Hoisting it is not a micro-optimisation
// at this scale: a 32 m tile is 98,304 voxels and the scan adds tens of
// thousands of calls on top.
double Terrain::density(const Vec3& dir, double altitude, double base,
                        double squash) const {
    const Vec3 fp = volumePos(dir, altitude);

    double d = (base - altitude) / squash;
    // The 3D terms are what produce overhangs, arches and crags. They are
    // scaled by how close we are to the surface: applied at full strength
    // hundreds of metres down they would do nothing (the depth term dwarfs
    // them) and applied unscaled near the surface they shred it.
    const double nearSurface = clampd(1.0 - std::fabs(base - altitude) / 40.0, 0.0, 1.0);
    // Guarded, not merely multiplied by zero. Beyond 40 m from the terrain
    // height these two terms contribute exactly nothing, and evaluating them
    // anyway was five noise samples — forty gradient hashes — per voxel, for
    // every voxel in open sky and every voxel deeper than 40 m. That is most of
    // a tile. The result is bit-identical; the arithmetic that was being thrown
    // away is simply not done.
    if (nearSurface > 0.0) {
        // thread_local, not member state: Noise stays const and the four worker
        // threads that build tiles share the Terrain with no locking. Each
        // thread walks its own column, so each gets its own cells and the hit
        // rate is whatever vertical coherence the field has.
        static thread_local NoiseCell cLarge[3], cSmall[2];
        d += nLarge_.fbm(fp / 180.0, 3, cLarge) * 2.2 * nearSurface;
        d += nSmall_.fbm(fp / 45.0, 2, cSmall) * 0.9 * nearSurface;
    }

    // floorBias: rises steeply below -440 m so the deep column is reliably
    // solid. Deep mining should be tunnelling, not falling into voids.
    if (altitude < -440.0) d += (-440.0 - altitude) * 0.06;
    return d;
}

bool Terrain::solidAt(const Vec3& dir, double altitude, const Climate& c) const {
    return solidAt(dir, altitude, terrainHeight(c), columnSquash(c));
}

bool Terrain::solidAt(const Vec3& dir, double altitude, double base,
                      double squash) const {
    if (density(dir, altitude, base, squash) <= 0.0) return false;
    return caveCarve(dir, altitude) <= 0.0;
}

double Terrain::surfaceAltitude(const Vec3& dir, const Climate& c) const {
    // Walk down until solid: 4 m steps to bracket, then bisection to land on
    // the surface.
    //
    // WHERE THE WALK STARTS is the whole cost of this function, and it used to
    // start at the top of the planet's relief range. For a column whose terrain
    // height is near sea level that is several hundred metres of empty sky,
    // tested four metres at a time, every one of them a full density
    // evaluation. Measured: the scan was 96% of the cost of a column, and a
    // column is most of the cost of a coarse tile.
    //
    // It can start far lower, and provably. Rock exists where
    //
    //     (base - altitude) / squash + lift  >  0
    //
    // so the highest rock in a column is at most `lift * squash` above the
    // terrain height. `lift` is bounded by the two 3D terms, 2.2 and 0.9 times
    // an fbm; genbench measures the worst |fbm| over 400,000 samples at 0.71
    // and 0.75, giving a worst lift of 2.24 and, at the maximum squash of 3.2,
    // 7.2 m of rock above the terrain height. kSurfaceCeiling is 16 m: more
    // than double the measured worst case, because being wrong here does not
    // degrade gracefully — it puts holes in the sky.
    //
    // The start is then SNAPPED BACK ONTO THE OLD 4 m GRID. Starting at an
    // arbitrary altitude would test a different sequence of points and land the
    // bisection on a slightly different surface, which is a different world.
    // Snapping keeps the sampled sequence a suffix of the original one, so the
    // result is bit-identical and the golden fingerprints in the world tests
    // still hold.
    constexpr double kSurfaceCeiling = 16.0;
    const double base = terrainHeight(c);
    const double squash = columnSquash(c);

    const double top = reliefHi_ + 8.0;
    double a = top;
    const double wanted = base + kSurfaceCeiling;
    if (wanted < top) {
        const double steps = std::ceil((top - wanted) / 4.0);
        a = top - steps * 4.0;
    }
    while (a > PlanetParams::kMinAltitude) {
        if (solidAt(dir, a, base, squash)) break;
        a -= 4.0;
    }
    double hi = a + 4.0;
    for (int i = 0; i < 8; ++i) {
        double mid = (a + hi) * 0.5;
        if (solidAt(dir, mid, base, squash)) a = mid; else hi = mid;
    }
    return a;
}

double Terrain::surfaceAltitude(Face f, double u, double v) const {
    const Vec3 dir = CubeSphere::direction(f, u, v);
    return surfaceAltitude(dir, climate(dir));
}

Biome Terrain::biomeAt(const Climate& c, double surfaceAlt) const {
    const double t = c.temperature, h = c.humidity;
    const double submerged = seaLevel_ - surfaceAlt;

    switch (p_.cls) {
        case PlanetClass::Barren:
            return (c.continentalness < -0.35 || c.erosion > 0.5)
                 ? BIOME_IMPACT_BASIN : BIOME_REGOLITH;

        case PlanetClass::Scorched:
            // Magma fields sit in the low, unerodedbasins — the biome you are
            // looking for, and the one that costs you to stand in.
            return (c.continentalness < -0.15 && c.erosion < 0.0)
                 ? BIOME_MAGMA_FIELD : BIOME_BASALT;

        case PlanetClass::Frozen:
            return submerged > 4.0 ? BIOME_SUBGLACIAL : BIOME_ICE_SHEET;

        case PlanetClass::Toxic:
            return (c.humidity > 0.25 && surfaceAlt < seaLevel_ + 30.0)
                 ? BIOME_SPORE_DEEPS : BIOME_SPORE_PLAIN;

        case PlanetClass::Irradiated:
            return (c.oddity > 0.35) ? BIOME_DEAD_CITY : BIOME_GLASS_SEA;

        case PlanetClass::Oceanic:
            if (submerged > 120.0) return BIOME_ABYSSAL_TRENCH;
            if (submerged > 2.0)   return BIOME_OPEN_OCEAN;
            return BIOME_BEACH;

        case PlanetClass::Anomalous:
        case PlanetClass::Temperate:
        default:
            break;
    }

    // Temperate (and Anomalous, which borrows the table): the full 5D lookup.
    if (submerged > 3.0) return BIOME_OCEAN;
    if (submerged > -3.0) return BIOME_BEACH;

    const bool mountainous = surfaceAlt > seaLevel_ + (reliefHi_ - seaLevel_) * 0.58;
    if (mountainous) return t < -0.35 ? BIOME_TUNDRA : BIOME_HIGHLANDS;

    if (t < -0.45) return BIOME_TUNDRA;
    if (t < -0.10) return h > 0.05 ? BIOME_BOREAL : BIOME_TUNDRA;

    if (t > 0.45) {
        if (h < -0.45) return BIOME_DESERT;
        if (h < -0.10) return (c.oddity > 0.30) ? BIOME_BADLANDS : BIOME_SAVANNA;
        return BIOME_RAINFOREST;
    }
    // Temperate band
    if (h > 0.55 && c.erosion > 0.25) return BIOME_WETLAND;
    if (c.oddity > 0.55) return BIOME_KARST;
    if (h > 0.15) return BIOME_FOREST;
    return BIOME_PLAINS;
}

Material Terrain::baseStone(double altitude) const {
    if (altitude <= PlanetParams::kMinAltitude + 2.0) return MAT_MANTLE;
    if (altitude < -256.0) return MAT_DEEP_STONE;
    switch (p_.cls) {
        case PlanetClass::Scorched:   return MAT_BASALT;
        case PlanetClass::Barren:     return MAT_STONE;
        case PlanetClass::Irradiated: return MAT_STONE;
        default:                      return MAT_STONE;
    }
}

// Surface cover.
//
// Two different quantities were conflated here, and the bug they produced was
// visible as contour banding across a hillside: alternating rings of grass and
// bare dirt following the height contours, on ground that should have been
// uniformly grassed.
//
//   * "the topmost solid voxel" is a *grid* fact. Grass is a skin one voxel
//     thick, whatever a voxel currently is.
//   * "four metres of dirt beneath it" is a *physical* fact, and must not
//     change when the same ground is sampled at 1/16 m or at 4 m.
//
// The old code wrote both as one integer depth rounded from a metric distance,
// so at one metre resolution the top voxel came out as depth 0 or depth 1
// depending on where in the metre the true surface happened to fall — and half
// the columns on a slope lost their grass. Now `top` carries the grid fact and
// `dm` carries the physical one, and the layer thicknesses below are metres.
void Terrain::surfaceCover(Biome b, double altitude, bool top, double dm,
                           Material& out) const {
    switch (b) {
        case BIOME_PLAINS: case BIOME_FOREST: case BIOME_RAINFOREST:
        case BIOME_SAVANNA: case BIOME_KARST:
            out = (top) ? MAT_GRASS : (dm < 4.0 ? MAT_DIRT : out); break;
        case BIOME_BOREAL:
            out = (top) ? MAT_GRASS : (dm < 3.0 ? MAT_DIRT : out); break;
        case BIOME_WETLAND:
            out = (top) ? MAT_GRASS : (dm < 3.0 ? MAT_CLAY : out); break;
        case BIOME_BEACH:
            out = (dm < 4.0) ? MAT_SAND : (dm < 7.0 ? MAT_SANDSTONE : out); break;
        case BIOME_DESERT:
            out = (dm < 5.0) ? MAT_SAND : (dm < 12.0 ? MAT_SANDSTONE : out); break;
        case BIOME_BADLANDS:
            out = (dm < 9.0) ? MAT_SANDSTONE : out; break;
        case BIOME_TUNDRA:
            out = (top) ? MAT_SNOW : (dm < 4.0 ? MAT_DIRT : out); break;
        case BIOME_HIGHLANDS:
            out = (top && altitude > 150.0) ? MAT_SNOW : out; break;
        case BIOME_OCEAN: case BIOME_OPEN_OCEAN:
            out = (dm < 3.0) ? MAT_SAND : (dm < 6.0 ? MAT_CLAY : out); break;
        case BIOME_ABYSSAL_TRENCH:
            out = (dm < 3.0) ? MAT_MUDSTONE : out; break;
        case BIOME_REGOLITH: case BIOME_IMPACT_BASIN:
            out = (dm < 3.0) ? MAT_REGOLITH : out; break;
        case BIOME_BASALT:
            out = (dm < 6.0) ? MAT_BASALT : out; break;
        case BIOME_MAGMA_FIELD:
            out = (top) ? MAT_ASH : (dm < 6.0 ? MAT_BASALT : out); break;
        case BIOME_ICE_SHEET:
            out = (dm < 3.0) ? MAT_SNOW : (dm < 9.0 ? MAT_ICE : out); break;
        case BIOME_SUBGLACIAL:
            out = (dm < 6.0) ? MAT_ICE : out; break;
        case BIOME_SPORE_PLAIN: case BIOME_SPORE_DEEPS:
            out = (top) ? MAT_FUNGAL_MAT : (dm < 4.0 ? MAT_MUDSTONE : out); break;
        case BIOME_GLASS_SEA:
            out = (dm < 2.0) ? MAT_FUSED_GLASS : out; break;
        case BIOME_DEAD_CITY:
            out = (dm < 2.0) ? MAT_GRAVEL : out; break;
        default: break;
    }
}

Material Terrain::oreAt(const Vec3& dir, double altitude, Material host) const {
    const Vec3 fp = volumePos(dir, altitude);
    if (host != MAT_STONE && host != MAT_DEEP_STONE && host != MAT_BASALT)
        return host;

    // Vein placement by hashed scatter over a lattice of candidate sites.
    //
    // The first implementation thresholded an fbm field. It generated no ore at
    // all, and the reason is worth recording: gradient noise has a range of
    // roughly +/-0.7, while the threshold solved for a realistic vein density
    // came out at 0.87. The field could never reach it, silently, and the only
    // symptom was an empty planet. That is exactly the class of failure the
    // first edition's principle E7 is about — a silent failure is the worse of
    // two bugs — and it is why this test existed before the feature did.
    //
    // What is here instead is the spec's own algorithm (§4.5), made pure per
    // voxel: space is divided into candidate cells; each cell hashes to a
    // decision, a centre and a radius; a voxel is ore if it falls inside a
    // vein sphere. Deterministic, connected blobs, and no distribution
    // assumptions to get wrong.
    constexpr double kCell = 8.0;                       // metres
    constexpr double kCellVolume = kCell * kCell * kCell;
    const bool deep = (host == MAT_DEEP_STONE);

    const int64_t bx = int64_t(std::floor(fp.x / kCell));
    const int64_t by = int64_t(std::floor(fp.y / kCell));
    const int64_t bz = int64_t(std::floor(fp.z / kCell));

    // Walked in table order so that two ores overlapping in depth resolve the
    // same way every time (S2). Shallow ores win ties, which is what makes a
    // deep ore feel like a reward for depth rather than a coin flip.
    for (int i = 0; i < kOreCount; ++i) {
        const OreDef& o = kOres[i];
        if (o.deepStoneOnly && !deep) continue;

        const double w = depthWeight(o, altitude);
        if (w <= 0.0) continue;

        const OreConstants& oc = oreConst_[size_t(i)];
        const double abundance = oc.abundance;
        // veinsPerChunk is per 32^3 = 32768 voxels. The expression is unchanged
        // from when both factors were computed inline — only the recomputation
        // is gone, so every product associates exactly as it did.
        const double pSite = oc.veinDensity * w * abundance * kCellVolume / 32768.0;
        if (pSite <= 0.0) continue;

        // Only visit neighbouring cells a vein could actually reach into.
        // The largest vein in the table is 20 voxels, a sphere of radius 1.7 m,
        // against an 8 m cell — so the 27-cell sweep was checking 24 cells that
        // could not possibly contain a hit. Bounding it is exact, not an
        // approximation: a site outside this range cannot reach this voxel.
        const double maxR = oc.maxReach;
        const double lx = fp.x - double(bx) * kCell, hx = kCell - lx;
        const double ly = fp.y - double(by) * kCell, hy = kCell - ly;
        const double lz = fp.z - double(bz) * kCell, hz = kCell - lz;
        const int dxLo = (lx < maxR) ? -1 : 0, dxHi = (hx < maxR) ? 1 : 0;
        const int dyLo = (ly < maxR) ? -1 : 0, dyHi = (hy < maxR) ? 1 : 0;
        const int dzLo = (lz < maxR) ? -1 : 0, dzHi = (hz < maxR) ? 1 : 0;

        for (int dz = dzLo; dz <= dzHi; ++dz)
        for (int dy = dyLo; dy <= dyHi; ++dy)
        for (int dx = dxLo; dx <= dxHi; ++dx) {
            const int64_t cx = bx + dx, cy = by + dy, cz = bz + dz;
            Rng rng(mix(p_.seed ^ labelHash("ore")
                        ^ oc.indexMix
                        ^ hash3(cx, cy, cz)));
            if (!rng.chance(float(pSite))) continue;

            const Vec3 centre{
                (double(cx) + rng.unit()) * kCell,
                (double(cy) + rng.unit()) * kCell,
                (double(cz) + rng.unit()) * kCell};
            const int voxels = o.veinMin +
                int(rng.below(uint32_t(o.veinMax - o.veinMin + 1)));
            // Radius of a sphere holding that many 1 m voxels.
            const double radius = std::cbrt(double(voxels) * 0.2387324146);
            const Vec3 d = fp - centre;
            if (d.dot(d) <= radius * radius) return o.material;
        }
    }
    return host;
}

Terrain::Column Terrain::columnAt(Face f, double u, double v) const {
    Column col;
    col.dir = CubeSphere::direction(f, u, v);
    col.climate = climate(col.dir);
    col.base = terrainHeight(col.climate);
    col.squash = columnSquash(col.climate);
    col.surface = surfaceAltitude(col.dir, col.climate);
    col.biome = biomeAt(col.climate, col.surface);
    const Vec3 ap = col.dir * p_.radius;
    col.aquifer = -60.0 + nAquifer_.fbm(ap / 900.0, 2) * 240.0;
    return col;
}

Material Terrain::materialInColumn(const Column& col, double altitude,
                                   double voxelSize) const {
    if (altitude <= PlanetParams::kMinAltitude + 1.0) return MAT_MANTLE;

    if (!solidAt(col.dir, altitude, col.base, col.squash)) {
        // See the note in the first implementation: what fills a void depends
        // on whether this column is sea floor, deep enough for lava, or inside
        // an aquifer. Sea level alone floods every cave under every mountain.
        if (p_.seaLevelFrac > 0.0 && altitude < seaLevel_ && col.surface < seaLevel_)
            return MAT_WATER;
        if (altitude < -400.0) {
            const double lv = nChamber_.fbm(volumePos(col.dir, altitude) / 260.0, 2);
            if (lv > 0.55) return MAT_LAVA;
        }
        if (p_.cls == PlanetClass::Scorched && col.surface < seaLevel_ - 30.0
            && altitude < seaLevel_ - 30.0)
            return MAT_LAVA;
        if (p_.seaLevelFrac > 0.0 && altitude < -20.0 && altitude > -380.0
            && altitude < col.aquifer)
            return MAT_WATER;
        return MAT_AIR;
    }

    Material m = baseStone(altitude);
    // Depth below the surface in metres, and whether this is the topmost solid
    // voxel of the column at whatever resolution the caller is sampling.
    const double dm = col.surface - altitude;
    // The skin is one voxel thick, or 35 cm, whichever is more.
    //
    // "One voxel" alone is right at block resolution and wrong below it. A
    // column's skin voxel is the topmost one in that column, so on a slope the
    // skin is a single-voxel step per column — and at 1/16 m those steps are
    // 62.5 mm apart and read as green speckles scattered across bare dirt
    // rather than as a grassed hillside. Giving the skin a physical minimum
    // thickness makes it a continuous layer at any resolution; at one metre
    // and coarser the voxel size wins and behaviour is unchanged.
    constexpr double kSurfaceSkin = 0.35;
    const double skin = voxelSize > kSurfaceSkin ? voxelSize : kSurfaceSkin;
    const bool top = dm >= 0.0 && dm < skin;
    if (dm >= 0.0 && dm < 14.0) surfaceCover(col.biome, altitude, top, dm, m);
    if (m == MAT_STONE || m == MAT_DEEP_STONE || m == MAT_BASALT)
        m = oreAt(col.dir, altitude, m);
    return m;
}

Material Terrain::materialAt(Face f, double u, double v, double altitude) const {
    return materialInColumn(columnAt(f, u, v), altitude);
}

}  // namespace ely
