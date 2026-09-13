// The voxel material table. Specification §5.2 and §5.3.
//
// Ids are permanent once anything has been saved: the edit journal stores
// material ids, so renumbering breaks every save. Append only.
#pragma once
#include "planet.h"
#include <cstdint>

namespace ely {

enum Material : uint16_t {
    MAT_AIR = 0,
    // Bulk
    MAT_STONE, MAT_DEEP_STONE, MAT_DIRT, MAT_GRASS, MAT_SAND, MAT_GRAVEL,
    MAT_CLAY, MAT_SNOW, MAT_ICE, MAT_SANDSTONE, MAT_MUDSTONE, MAT_REGOLITH,
    MAT_BASALT, MAT_ASH, MAT_OBSIDIAN, MAT_FUSED_GLASS, MAT_FUNGAL_MAT,
    MAT_MANTLE,
    // Fluids
    MAT_WATER, MAT_LAVA,
    // Ores — order matches the spec's table, shallow to deep
    MAT_ORE_COAL, MAT_ORE_COPPER, MAT_ORE_TIN, MAT_ORE_ZINC, MAT_ORE_IRON,
    MAT_ORE_BAUXITE, MAT_ORE_RESONANT, MAT_ORE_LEAD, MAT_ORE_SILVER,
    MAT_ORE_NICKEL, MAT_ORE_GOLD, MAT_ORE_COBALT, MAT_ORE_EMERALD,
    MAT_ORE_TITANIUM, MAT_ORE_TUNGSTEN, MAT_ORE_PLATINUM, MAT_ORE_DIAMOND,
    MAT_ORE_OSMIUM, MAT_ORE_URANIUM,
    // The Empire's three
    MAT_ORE_VOIDGLASS, MAT_ORE_AETHERIUM, MAT_ORE_NEUTRONIUM,
    MAT_COUNT
};

struct MaterialInfo {
    const char* name;
    uint8_t r, g, b;      // display colour for headless rendering
    bool solid;
    uint8_t toolTier;     // 0..5, spec §5.1
};

const MaterialInfo& materialInfo(Material m);
inline bool isSolid(Material m) { return materialInfo(m).solid; }
inline bool isOre(Material m) { return m >= MAT_ORE_COAL && m <= MAT_ORE_NEUTRONIUM; }

// ---------------------------------------------------------------------------
// Ore definitions — the table from spec §5.2, in code so that the generator
// and any tooling read one source of truth (principle E5).
// ---------------------------------------------------------------------------
struct OreDef {
    Material material;
    const char* name;
    double peak;          // altitude of maximum probability, metres
    double spread;        // falls to zero this far either side
    int    veinMin, veinMax;
    double veinsPerChunk; // before class abundance
    uint8_t toolTier;
    Element element;
    bool   deepStoneOnly;
    double secondLobePeak;    // 0 == none; iron's mountain band
    double secondLobeSpread;
};

extern const OreDef kOres[];
extern const int kOreCount;

// Class abundance multiplier — Mechanism 4 (spec §5.2).
double oreAbundance(PlanetClass cls, Material ore);

// Vein sites per 32^3 chunk, after the global density calibration. Generators
// must use this rather than OreDef::veinsPerChunk directly.
double veinDensity(const OreDef& ore);

// Triangular depth weighting (spec §4.5).
double depthWeight(const OreDef& ore, double altitude);

}  // namespace ely
