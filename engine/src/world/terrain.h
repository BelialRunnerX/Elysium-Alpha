// Terrain generation. Specification §4.2 through §4.5.
//
// The whole pipeline is a pure function of (planet seed, position). It is
// evaluated per voxel during chunk generation and per column for maps and
// tooling. Nothing caches across chunks except the noise objects themselves,
// which are stateless.
#pragma once
#include "../core/noise.h"
#include "cubesphere.h"
#include "material.h"
#include "planet.h"

#include <vector>

namespace ely {

enum Biome : uint8_t {
    // Temperate — the sandbox world (spec §4.3)
    BIOME_OCEAN = 0, BIOME_BEACH, BIOME_PLAINS, BIOME_FOREST, BIOME_BOREAL,
    BIOME_RAINFOREST, BIOME_SAVANNA, BIOME_DESERT, BIOME_BADLANDS,
    BIOME_TUNDRA, BIOME_HIGHLANDS, BIOME_WETLAND, BIOME_KARST,
    // Hostile classes — one "the biome you are looking for" each
    BIOME_REGOLITH, BIOME_IMPACT_BASIN,
    BIOME_BASALT, BIOME_MAGMA_FIELD,
    BIOME_ICE_SHEET, BIOME_SUBGLACIAL,
    BIOME_SPORE_PLAIN, BIOME_SPORE_DEEPS,
    BIOME_GLASS_SEA, BIOME_DEAD_CITY,
    BIOME_OPEN_OCEAN, BIOME_ABYSSAL_TRENCH,
    BIOME_COUNT
};

const char* biomeName(Biome b);
void biomeColour(Biome b, uint8_t& r, uint8_t& g, uint8_t& bl);

// The six climate fields (spec §4.2). All in [-1, 1].
struct Climate {
    double continentalness = 0;
    double erosion = 0;
    double ridges = 0;
    double temperature = 0;
    double humidity = 0;
    double oddity = 0;
};

class Terrain {
public:
    Terrain(const PlanetParams& params);

    const PlanetParams& planet() const { return p_; }
    const CubeSphere& sphere() const { return sphere_; }

    // Sea level as an altitude in metres, from the planet's sea-level fraction.
    double seaLevel() const { return seaLevel_; }

    // --- The pipeline, stage by stage. Each is separately inspectable, which
    // is principle S6 and the reason the tools in tools/ can render any one of
    // them as an image without launching a game.

    // All of these take a *unit direction* from the planet centre, never a
    // face-local (u, v). That is the seam fix (see CubeSphere::fieldPos): two
    // faces meeting at a cube edge ask about the same direction and therefore
    // get the same answer, so every field agrees across every seam by
    // construction.
    //
    // Note the deliberate split between two sampling positions:
    //
    //   climate  is sampled at  dir * radius            — a 2D map on a shell
    //   volume   is sampled at  dir * (radius + alt)    — genuinely 3D
    //
    // Climate is a map: temperature does not vary with depth. Density, caves
    // and ore are volumetric and must vary with depth. Conflating the two was
    // a real bug here: with the volume fields sampled on the shell, every 3D
    // noise term was constant down a column, which silently disabled caves,
    // overhangs and the entire ore system while leaving the terrain surface
    // looking perfectly plausible.
    Climate climate(const Vec3& dir) const;
    double  terrainHeight(const Climate& c) const;
    double  density(const Vec3& dir, double altitude, const Climate& c) const;
    // The same field with the per-column constants hoisted out. Identical
    // results; the convenience form simply computes them and calls this.
    double  density(const Vec3& dir, double altitude, double base,
                    double squash) const;

    // How much the depth term is stretched, from erosion. A property of the
    // column, like the terrain height, and hoisted for the same reason.
    double  columnSquash(const Climate& c) const {
        return lerpd(0.6, 3.2, (c.erosion + 1.0) * 0.5);
    }
    double  caveCarve(const Vec3& dir, double altitude) const;
    // Rock is present here: dense enough, and not carved away by a cave.
    // Everything downstream asks this rather than density() directly.
    bool    solidAt(const Vec3& dir, double altitude, const Climate& c) const;
    bool    solidAt(const Vec3& dir, double altitude, double base,
                    double squash) const;
    Biome   biomeAt(const Climate& c, double surfaceAltitude) const;

    // The full per-voxel answer, including surface materials, fluids and ore.
    //
    // Convenience only — for tools, tests and single queries. It recomputes
    // the column's climate and surface altitude every call, which is fine for
    // one voxel and ruinous for a chunk. Chunk generation must use
    // generateColumn instead.
    Material materialAt(Face f, double u, double v, double altitude) const;

    // Everything a column needs, computed once.
    //
    // This exists because the per-voxel path was 517 ms per 32^3 chunk against
    // a 2 ms budget — 250x over. The cost was surfaceAltitude(), a descent
    // scan of roughly 180 density evaluations, being run for every voxel when
    // it is a property of the *column*. Hoisting it is a two-order-of-magnitude
    // win and it is the reason chunk generation is a column walk rather than a
    // triple loop over materialAt.
    struct Column {
        Vec3 dir;
        Climate climate;
        double base;      // terrain height from the climate, hoisted
        double squash;    // depth-term stretch from erosion, hoisted
        double surface;   // altitude of the topmost solid voxel
        Biome biome;
        double aquifer;
    };
    Column columnAt(Face f, double u, double v) const;

    // voxelSize is the edge length of the voxel whose centre `altitude` is, in
    // metres. It exists so that "the topmost solid voxel gets the grass" stays
    // true at every level of the resolution ladder — at 1/16 m the grass skin
    // is 62.5 mm thick, at 4 m it is four metres, and in both cases it is
    // exactly the surface voxel and nothing below it. Layer *thicknesses*
    // below the surface are metres and do not scale.
    Material materialInColumn(const Column& col, double altitude,
                              double voxelSize = 1.0) const;

    // Column helpers, for maps and for the mesher's height queries.
    double surfaceAltitude(Face f, double u, double v) const;
    double surfaceAltitude(const Vec3& dir, const Climate& c) const;

private:
    Material baseStone(double altitude) const;
    Material oreAt(const Vec3& dir, double altitude, Material host) const;
    Vec3 volumePos(const Vec3& dir, double altitude) const {
        return dir * (p_.radius + altitude);
    }
    void surfaceCover(Biome b, double altitude, bool topVoxel,
                      double depthMetres, Material& out) const;

    // Per-ore constants, computed once in the constructor.
    //
    // oreAt loops all 22 ores for every stone voxel, and every iteration was
    // recomputing things that depend only on (planet class, ore): a class
    // abundance lookup that walks initialiser lists, a global density scale,
    // and a cube root. Twenty-two cube roots per voxel is not a rounding error.
    //
    // These are cached as VALUES, not as folded products, so the arithmetic in
    // oreAt is character-for-character the expression it always was. Folding
    // `veinDensity * abundance` into one constant would reassociate the
    // multiply, change the last bit of pSite, flip a vein's coin somewhere on
    // some planet, and silently produce a different world.
    struct OreConstants {
        double veinDensity;   // veinDensity(o)
        double abundance;     // oreAbundance(class, o.material)
        double maxReach;      // cbrt(veinMax * 3/(4 pi)) — how far a vein spans
        uint64_t indexMix;    // mix(i * golden), the per-ore hash salt
    };
    std::vector<OreConstants> oreConst_;

    PlanetParams p_;
    CubeSphere sphere_;
    double seaLevel_;
    double reliefLo_, reliefHi_;

    // One noise instance per field, each with its own derived seed so that
    // changing one field's octave count cannot shift another (S2).
    Noise nCont_, nEros_, nRidge_, nTemp_, nHumid_, nOdd_;
    Noise nLarge_, nSmall_, nCaveA_, nCaveB_, nChamber_, nFissure_;
    Noise nOre_, nAquifer_;
};

}  // namespace ely
