#include "material.h"
#include <algorithm>
#include <cmath>

namespace ely {

namespace {
const MaterialInfo kInfo[MAT_COUNT] = {
    {"air",           0,   0,   0,   false, 0},
    {"stone",         128, 128, 132, true,  1},
    {"deep stone",    72,  72,  80,  true,  3},
    {"dirt",          110, 84,  58,  true,  0},
    {"grass",         96,  148, 74,  true,  0},
    {"sand",          214, 200, 152, true,  0},
    {"gravel",        132, 128, 124, true,  0},
    {"clay",          158, 152, 160, true,  0},
    {"snow",          238, 242, 248, true,  0},
    {"ice",           178, 208, 232, true,  0},
    {"sandstone",     206, 186, 140, true,  1},
    {"mudstone",      112, 96,  84,  true,  1},
    {"regolith",      142, 136, 128, true,  0},
    {"basalt",        62,  60,  66,  true,  1},
    {"ash",           96,  90,  88,  true,  0},
    {"obsidian",      28,  24,  38,  true,  4},
    {"fused glass",   170, 178, 190, true,  1},
    {"fungal mat",    124, 96,  140, true,  0},
    {"mantle",        44,  30,  30,  true,  6},   // tier 6 == unmineable
    {"water",         54,  104, 176, false, 0},
    {"lava",          214, 104, 38,  false, 0},

    {"coal ore",      54,  52,  56,  true,  1},
    {"copper ore",    182, 122, 84,  true,  1},
    {"tin ore",       186, 186, 196, true,  1},
    {"zinc ore",      164, 172, 176, true,  1},
    {"iron ore",      178, 148, 122, true,  2},
    {"bauxite",       190, 140, 104, true,  2},
    {"resonant dust", 190, 74,  92,  true,  2},
    {"lead ore",      104, 104, 122, true,  2},
    {"silver ore",    206, 208, 218, true,  3},
    {"nickel ore",    170, 176, 152, true,  3},
    {"gold ore",      222, 186, 88,  true,  3},
    {"cobalt ore",    76,  102, 178, true,  3},
    {"emerald ore",   74,  190, 122, true,  3},
    {"titanium ore",  164, 168, 180, true,  4},
    {"tungsten ore",  90,  92,  98,  true,  4},
    {"platinum ore",  208, 212, 220, true,  4},
    {"diamond ore",   110, 214, 214, true,  4},
    {"osmium ore",    126, 134, 158, true,  4},
    {"uranium ore",   118, 176, 86,  true,  4},
    {"voidglass",     168, 110, 240, true,  3},
    {"aetherium",     127, 212, 232, true,  4},
    {"neutronium",    74,  74,  74,  true,  5},
};
}  // namespace

const MaterialInfo& materialInfo(Material m) {
    return kInfo[m < MAT_COUNT ? m : MAT_AIR];
}

// The ore table, from spec §5.2.
//
// kVeinDensityScale is a calibration the spec did not have. The document's
// per-chunk vein counts, taken literally, put coal at 0.03% of stone by
// volume — a player would tunnel two hundred metres without seeing any. The
// reference the whole table was shaped against sits an order of magnitude
// higher: coal around 0.25% of stone in its band, diamond around 0.008% in
// its. One scale factor brings the whole table onto that curve without
// disturbing the *relative* rarities the design chose, which are the part
// that carries the progression.
//
// Kept as one auditable number rather than folded into 22 literals, so that
// re-tuning overall ore abundance is a single decision. The spec table should
// be corrected to match.
constexpr double kVeinDensityScale = 8.0;
const OreDef kOres[] = {
  //  material            name            peak   spread vmin vmax  /chunk tier element             deep   lobe   lobeSpread
    { MAT_ORE_COAL,      "coal",           40.0,  220.0,  4, 18,  0.90,  1, Element::None,        false,   0.0,   0.0 },
    { MAT_ORE_COPPER,    "copper",        -20.0,  140.0,  6, 20,  0.55,  1, Element::Plasma,      false,   0.0,   0.0 },
    { MAT_ORE_TIN,       "tin",           -30.0,  120.0,  3,  9,  0.34,  1, Element::Neural,      false,   0.0,   0.0 },
    { MAT_ORE_ZINC,      "zinc",          -40.0,  120.0,  3,  9,  0.30,  1, Element::Plasma,      false,   0.0,   0.0 },
    { MAT_ORE_IRON,      "iron",          -60.0,  180.0,  4, 14,  0.62,  2, Element::Kinetic,     false, 180.0,  90.0 },
    { MAT_ORE_BAUXITE,   "bauxite",        20.0,  100.0,  5, 16,  0.30,  2, Element::Dimensional, false,   0.0,   0.0 },
    { MAT_ORE_RESONANT,  "resonant dust", -300.0,  200.0,  6, 20,  0.42,  2, Element::Neural,      false,   0.0,   0.0 },
    { MAT_ORE_LEAD,      "lead",          -90.0,  120.0,  3, 10,  0.28,  2, Element::Kinetic,     false,   0.0,   0.0 },
    { MAT_ORE_SILVER,    "silver",       -110.0,  110.0,  2,  7,  0.20,  3, Element::Neural,      false,   0.0,   0.0 },
    { MAT_ORE_NICKEL,    "nickel",       -130.0,  110.0,  2,  8,  0.22,  3, Element::Kinetic,     false,   0.0,   0.0 },
    { MAT_ORE_GOLD,      "gold",         -160.0,  150.0,  2,  8,  0.20,  3, Element::Neural,      false,   0.0,   0.0 },
    { MAT_ORE_COBALT,    "cobalt",       -200.0,  100.0,  2,  6,  0.16,  3, Element::Dimensional, false,   0.0,   0.0 },
    { MAT_ORE_EMERALD,   "emerald",       200.0,  120.0,  1,  2,  0.10,  3, Element::Neural,      false,   0.0,   0.0 },
    { MAT_ORE_TITANIUM,  "titanium",     -230.0,  110.0,  2,  6,  0.16,  4, Element::Dimensional, true,    0.0,   0.0 },
    { MAT_ORE_TUNGSTEN,  "tungsten",     -280.0,  100.0,  2,  5,  0.14,  4, Element::Kinetic,     true,    0.0,   0.0 },
    { MAT_ORE_PLATINUM,  "platinum",     -300.0,   90.0,  1,  4,  0.12,  4, Element::Neural,      true,    0.0,   0.0 },
    { MAT_ORE_DIAMOND,   "diamond",      -380.0,  130.0,  1,  4,  0.13,  4, Element::Dimensional, true,    0.0,   0.0 },
    { MAT_ORE_OSMIUM,    "osmium",       -400.0,   90.0,  1,  4,  0.10,  4, Element::Void,        true,    0.0,   0.0 },
    { MAT_ORE_URANIUM,   "uranium",      -420.0,  100.0,  1,  3,  0.09,  4, Element::Void,        true,    0.0,   0.0 },
    { MAT_ORE_VOIDGLASS, "voidglass",    -150.0,  150.0,  3,  8,  0.26,  3, Element::Void,        false,   0.0,   0.0 },
    { MAT_ORE_AETHERIUM, "aetherium",    -300.0,  130.0,  2,  6,  0.15,  4, Element::Dimensional, true,    0.0,   0.0 },
    { MAT_ORE_NEUTRONIUM,"neutronium",   -470.0,   60.0,  1,  3,  0.10,  5, Element::Kinetic,     true,    0.0,   0.0 },
};
const int kOreCount = int(sizeof(kOres) / sizeof(kOres[0]));

double veinDensity(const OreDef& o) { return o.veinsPerChunk * kVeinDensityScale; }

double depthWeight(const OreDef& o, double altitude) {
    double w = std::max(0.0, 1.0 - std::fabs(altitude - o.peak) / o.spread);
    if (o.secondLobeSpread > 0.0) {
        // The second lobe is deliberately weaker: mountains are worth climbing
        // for iron, but they are not the main iron band.
        double w2 = std::max(0.0,
            1.0 - std::fabs(altitude - o.secondLobePeak) / o.secondLobeSpread);
        w = std::max(w, w2 * 0.55);
    }
    return w;
}

// Mechanism 4. Boosted ores are the class's own element plus a few that fit
// the fiction; suppressed ores are the ones the class has no process to make.
double oreAbundance(PlanetClass cls, Material ore) {
    auto boost = [](std::initializer_list<Material> list, Material m, double v) {
        for (Material x : list) if (x == m) return v;
        return 1.0;
    };
    switch (cls) {
        case PlanetClass::Temperate:
            // Nothing boosted — but everything is present, which is what makes
            // a Temperate world worth claiming (spec §5.2).
            return boost({MAT_ORE_URANIUM, MAT_ORE_OSMIUM}, ore, 0.3);
        case PlanetClass::Barren: {
            double v = boost({MAT_ORE_IRON, MAT_ORE_NICKEL, MAT_ORE_TITANIUM,
                              MAT_ORE_PLATINUM}, ore, 2.2);
            return v != 1.0 ? v : boost({MAT_ORE_COAL, MAT_ORE_BAUXITE}, ore, 0.3);
        }
        case PlanetClass::Scorched: {
            double v = boost({MAT_ORE_COPPER, MAT_ORE_ZINC, MAT_ORE_TUNGSTEN,
                              MAT_ORE_GOLD}, ore, 2.4);
            return v != 1.0 ? v : boost({MAT_ORE_COAL, MAT_ORE_RESONANT}, ore, 0.3);
        }
        case PlanetClass::Frozen: {
            double v = boost({MAT_ORE_IRON, MAT_ORE_LEAD, MAT_ORE_NICKEL,
                              MAT_ORE_NEUTRONIUM}, ore, 2.4);
            return v != 1.0 ? v : boost({MAT_ORE_BAUXITE, MAT_ORE_EMERALD}, ore, 0.3);
        }
        case PlanetClass::Toxic: {
            double v = boost({MAT_ORE_TIN, MAT_ORE_SILVER, MAT_ORE_GOLD,
                              MAT_ORE_PLATINUM, MAT_ORE_RESONANT}, ore, 2.4);
            return v != 1.0 ? v : boost({MAT_ORE_TITANIUM, MAT_ORE_TUNGSTEN}, ore, 0.3);
        }
        case PlanetClass::Irradiated: {
            if (ore == MAT_ORE_URANIUM) return 4.0;
            if (ore == MAT_ORE_NEUTRONIUM) return 3.0;
            if (ore == MAT_ORE_VOIDGLASS) return 2.0;
            double v = boost({MAT_ORE_OSMIUM}, ore, 2.6);
            return v != 1.0 ? v : boost({MAT_ORE_COAL}, ore, 0.3);
        }
        case PlanetClass::Oceanic: {
            double v = boost({MAT_ORE_BAUXITE, MAT_ORE_COBALT, MAT_ORE_TITANIUM,
                              MAT_ORE_DIAMOND}, ore, 2.4);
            return v != 1.0 ? v : boost({MAT_ORE_COAL, MAT_ORE_GOLD}, ore, 0.3);
        }
        case PlanetClass::Anomalous: {
            if (ore == MAT_ORE_AETHERIUM) return 4.0;
            // Tier 1-2 suppressed: an Anomalous world is not where you go for
            // copper.
            const OreDef* def = nullptr;
            for (int i = 0; i < kOreCount; ++i)
                if (kOres[i].material == ore) { def = &kOres[i]; break; }
            if (def && def->toolTier <= 2) return 0.3;
            return 1.0;
        }
    }
    return 1.0;
}

}  // namespace ely
