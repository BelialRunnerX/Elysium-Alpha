// Intended function: imported content implementation for ContentCatalogue; preserves the agent-authored subsystem contract for later integration/debugging.
#include "content/ContentCatalogue.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <cmath>
#include <set>
#include <map>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace elysium::content {
namespace {

using EA = ElementAffinity;
using DB = DensityBand;
using SB = StrengthBand;
using TB = ThermalBehavior;
using CB = CorrosionBehavior;
using RB = ShieldingBand;
using KB = ConductivityBand;
using MD = MachineDomain;
using RS = RecipeStatus;
using TK = TrinketKind;
using DR = DungeonRoomKind;
using TA = ThreatAxis;

constexpr std::array kMaterials{
    MaterialDefinition{"elysium:material/vacuum", "Vacuum", "environment", DB::VeryLight, SB::Fragile, TB::Ordinary, CB::Inert, RB::None, KB::Insulator, EA::None, false, true, false, false, "invisible"},
    MaterialDefinition{"elysium:material/soil", "Soil", "natural", DB::Medium, SB::Soft, TB::Insulating, CB::Ordinary, RB::Low, KB::Low, EA::None, false, true, true, true, "earth"},
    MaterialDefinition{"elysium:material/grass_soil", "Grass Soil", "natural", DB::Medium, SB::Soft, TB::Insulating, CB::Ordinary, RB::Low, KB::Low, EA::None, false, true, true, true, "grass-earth"},
    MaterialDefinition{"elysium:material/stone", "Stone", "natural", DB::Heavy, SB::Hard, TB::Ordinary, CB::Resistant, RB::Medium, KB::Low, EA::None, true, false, false, true, "stratified-stone"},
    MaterialDefinition{"elysium:material/regolith", "Regolith", "natural", DB::Medium, SB::Soft, TB::Insulating, CB::Inert, RB::Low, KB::Low, EA::None, false, true, false, true, "powdered-rock"},
    MaterialDefinition{"elysium:material/basalt", "Basalt", "natural", DB::Heavy, SB::Hard, TB::HeatResistant, CB::Resistant, RB::Medium, KB::Low, EA::Plasma, true, false, false, true, "dark-volcanic"},
    MaterialDefinition{"elysium:material/timber", "Timber", "organic", DB::Light, SB::Medium, TB::Insulating, CB::Vulnerable, RB::None, KB::Insulator, EA::None, true, false, true, true, "timber-grain"},
    MaterialDefinition{"elysium:material/steel", "Steel", "metal", DB::Heavy, SB::Hard, TB::Conductive, CB::Resistant, RB::Medium, KB::High, EA::Kinetic, true, false, false, true, "worn-metal-panel"},
    MaterialDefinition{"elysium:material/magma", "Magma", "fluid", DB::Heavy, SB::Fragile, TB::Refractory, CB::Inert, RB::None, KB::Medium, EA::Plasma, false, true, false, false, "emissive-molten"},
    MaterialDefinition{"elysium:material/machine_composite", "Machine Composite", "engineered", DB::Heavy, SB::Hard, TB::HeatResistant, CB::Resistant, RB::Medium, KB::High, EA::None, true, false, false, true, "frontier-machine"},
    MaterialDefinition{"elysium:material/glass", "Glass", "engineered", DB::Medium, SB::Medium, TB::Ordinary, CB::Resistant, RB::Low, KB::Insulator, EA::None, true, false, false, true, "transparent-glass"},
    MaterialDefinition{"elysium:material/armored_glass", "Armored Glass", "engineered", DB::Heavy, SB::Hard, TB::HeatResistant, CB::Resistant, RB::Medium, KB::Insulator, EA::None, true, false, false, true, "reinforced-glass"},
    MaterialDefinition{"elysium:material/concrete", "Concrete", "engineered", DB::Heavy, SB::Hard, TB::Insulating, CB::Resistant, RB::Medium, KB::Insulator, EA::None, true, false, false, true, "cast-composite"},
    MaterialDefinition{"elysium:material/ceramic", "Ceramic", "engineered", DB::Medium, SB::Hard, TB::Refractory, CB::Inert, RB::Low, KB::Insulator, EA::None, true, false, false, true, "ceramic"},
    MaterialDefinition{"elysium:material/composite_panel", "Composite Panel", "engineered", DB::Light, SB::Hard, TB::Insulating, CB::Resistant, RB::Low, KB::Low, EA::None, true, false, false, true, "layered-composite"},
    MaterialDefinition{"elysium:material/titanium", "Titanium", "metal", DB::Medium, SB::Hard, TB::HeatResistant, CB::Resistant, RB::Low, KB::Medium, EA::Dimensional, true, false, false, true, "light-hull-metal"},
    MaterialDefinition{"elysium:material/tungsten", "Tungsten", "metal", DB::Extreme, SB::Extreme, TB::Refractory, CB::Resistant, RB::High, KB::High, EA::Kinetic, true, false, false, true, "dense-thermal-metal"},
    MaterialDefinition{"elysium:material/lead", "Lead", "metal", DB::Extreme, SB::Medium, TB::Conductive, CB::Ordinary, RB::Extreme, KB::Medium, EA::Kinetic, true, false, false, true, "radiation-liner"},
    MaterialDefinition{"elysium:material/copper", "Copper", "metal", DB::Heavy, SB::Medium, TB::Conductive, CB::Ordinary, RB::Low, KB::High, EA::Plasma, true, false, false, true, "copper-metal"},
    MaterialDefinition{"elysium:material/tin", "Tin", "metal", DB::Heavy, SB::Soft, TB::Conductive, CB::Ordinary, RB::Low, KB::Medium, EA::Neural, true, false, false, true, "tin-metal"},
    MaterialDefinition{"elysium:material/zinc", "Zinc", "metal", DB::Heavy, SB::Soft, TB::Conductive, CB::Ordinary, RB::Low, KB::Medium, EA::Plasma, true, false, false, true, "zinc-metal"},
    MaterialDefinition{"elysium:material/iron", "Iron", "metal", DB::Heavy, SB::Hard, TB::Conductive, CB::Vulnerable, RB::Medium, KB::High, EA::Kinetic, true, false, false, true, "iron-metal"},
    MaterialDefinition{"elysium:material/bauxite", "Bauxite", "ore", DB::Medium, SB::Medium, TB::Ordinary, CB::Ordinary, RB::Low, KB::Low, EA::Dimensional, false, true, false, true, "red-ore"},
    MaterialDefinition{"elysium:material/resonant_dust", "Resonant Dust", "ore", DB::Light, SB::Soft, TB::Ordinary, CB::Ordinary, RB::Low, KB::Resonant, EA::Neural, false, true, false, true, "resonant-ore"},
    MaterialDefinition{"elysium:material/silver", "Silver", "metal", DB::Heavy, SB::Medium, TB::Conductive, CB::Resistant, RB::Low, KB::High, EA::Neural, true, false, false, true, "silver-metal"},
    MaterialDefinition{"elysium:material/nickel", "Nickel", "metal", DB::Heavy, SB::Hard, TB::Conductive, CB::Resistant, RB::Medium, KB::High, EA::Kinetic, true, false, false, true, "nickel-metal"},
    MaterialDefinition{"elysium:material/gold", "Gold", "metal", DB::Extreme, SB::Soft, TB::Conductive, CB::Inert, RB::Medium, KB::High, EA::Neural, true, false, false, true, "gold-metal"},
    MaterialDefinition{"elysium:material/cobalt", "Cobalt", "metal", DB::Heavy, SB::Hard, TB::HeatResistant, CB::Resistant, RB::Medium, KB::Medium, EA::Dimensional, true, false, false, true, "cobalt-metal"},
    MaterialDefinition{"elysium:material/emerald", "Emerald", "crystal", DB::Medium, SB::Hard, TB::Ordinary, CB::Inert, RB::Low, KB::Resonant, EA::Neural, true, false, false, true, "emerald-crystal"},
    MaterialDefinition{"elysium:material/platinum", "Platinum", "metal", DB::Extreme, SB::Hard, TB::Conductive, CB::Inert, RB::Medium, KB::High, EA::Neural, true, false, false, true, "platinum-metal"},
    MaterialDefinition{"elysium:material/diamond", "Diamond", "crystal", DB::Medium, SB::Extreme, TB::HeatResistant, CB::Inert, RB::Low, KB::Insulator, EA::Dimensional, true, false, false, true, "diamond-crystal"},
    MaterialDefinition{"elysium:material/osmium", "Osmium", "metal", DB::Extreme, SB::Hard, TB::Conductive, CB::Inert, RB::High, KB::High, EA::Void, true, false, false, true, "osmium-metal"},
    MaterialDefinition{"elysium:material/uranium", "Uranium", "metal", DB::Extreme, SB::Medium, TB::Conductive, CB::Ordinary, RB::High, KB::Medium, EA::Void, true, false, false, true, "uranium-ore"},
    MaterialDefinition{"elysium:material/voidglass", "Voidglass", "exotic", DB::Heavy, SB::Hard, TB::HeatResistant, CB::Inert, RB::High, KB::Resonant, EA::Void, true, false, false, true, "voidglass"},
    MaterialDefinition{"elysium:material/aetherium", "Aetherium", "exotic", DB::Medium, SB::Hard, TB::Ordinary, CB::Inert, RB::Medium, KB::Resonant, EA::Dimensional, true, false, false, true, "aetherium"},
    MaterialDefinition{"elysium:material/neutronium", "Neutronium", "exotic", DB::Extreme, SB::Extreme, TB::Refractory, CB::Inert, RB::Extreme, KB::High, EA::Kinetic, true, false, false, true, "neutronium"},
};

constexpr std::array kBlocks{
    BlockDefinition{"elysium:block/air", "Air", "environment", "elysium:material/vacuum", "none", false, false, true, 0},
    BlockDefinition{"elysium:block/grass", "Grass", "terrain", "elysium:material/grass_soil", "soil", false, true, true, 1},
    BlockDefinition{"elysium:block/dirt", "Dirt", "terrain", "elysium:material/soil", "soil", false, true, true, 2},
    BlockDefinition{"elysium:block/stone", "Stone", "terrain", "elysium:material/stone", "stone", true, true, true, 3},
    BlockDefinition{"elysium:block/regolith", "Regolith", "terrain", "elysium:material/regolith", "soil", false, true, true, 4},
    BlockDefinition{"elysium:block/basalt", "Basalt", "terrain", "elysium:material/basalt", "stone", true, true, true, 5},
    BlockDefinition{"elysium:block/coal_ore", "Coal Ore", "ore", "elysium:material/stone", "stone", true, true, true, 6},
    BlockDefinition{"elysium:block/copper_ore", "Copper Ore", "ore", "elysium:material/copper", "stone", true, true, true, 7},
    BlockDefinition{"elysium:block/tin_ore", "Tin Ore", "ore", "elysium:material/tin", "stone", true, true, true, 8},
    BlockDefinition{"elysium:block/iron_ore", "Iron Ore", "ore", "elysium:material/iron", "stone", true, true, true, 9},
    BlockDefinition{"elysium:block/planks", "Planks", "construction", "elysium:material/timber", "frame", true, true, false, 10},
    BlockDefinition{"elysium:block/steel_plate", "Steel Plate", "construction", "elysium:material/steel", "frame", true, true, false, 11},
    BlockDefinition{"elysium:block/registry_beacon", "Registry Beacon", "command", "elysium:material/machine_composite", "machine", true, false, false, 12},
    BlockDefinition{"elysium:block/magma", "Magma", "fluid", "elysium:material/magma", "none", false, false, true, 13},
    BlockDefinition{"elysium:block/door_panel", "Door Panel", "opening", "elysium:material/steel", "frame", true, true, false, 14},
    BlockDefinition{"elysium:block/airlock_panel", "Airlock Panel", "opening", "elysium:material/steel", "frame", true, true, false, 15},
    BlockDefinition{"elysium:block/glass", "Glass", "construction", "elysium:material/glass", "panel", true, true, false, std::nullopt},
    BlockDefinition{"elysium:block/armored_glass", "Armored Glass", "construction", "elysium:material/armored_glass", "panel", true, true, false, std::nullopt},
    BlockDefinition{"elysium:block/concrete", "Concrete", "construction", "elysium:material/concrete", "stone", true, true, false, std::nullopt},
    BlockDefinition{"elysium:block/ceramic_tile", "Ceramic Tile", "construction", "elysium:material/ceramic", "panel", true, true, false, std::nullopt},
    BlockDefinition{"elysium:block/composite_panel", "Composite Panel", "construction", "elysium:material/composite_panel", "frame", true, true, false, std::nullopt},
    BlockDefinition{"elysium:block/titanium_plate", "Titanium Plate", "construction", "elysium:material/titanium", "frame", true, true, false, std::nullopt},
    BlockDefinition{"elysium:block/tungsten_liner", "Tungsten Liner", "hazard", "elysium:material/tungsten", "liner", true, true, false, std::nullopt},
    BlockDefinition{"elysium:block/lead_liner", "Lead Liner", "hazard", "elysium:material/lead", "liner", true, true, false, std::nullopt},
    BlockDefinition{"elysium:block/voidglass_panel", "Voidglass Panel", "exotic", "elysium:material/voidglass", "frame", true, true, false, std::nullopt},
    BlockDefinition{"elysium:block/neutronium_plate", "Neutronium Plate", "exotic", "elysium:material/neutronium", "frame", true, true, false, std::nullopt},
};

#define PIECE(fam, var, structural, seal, utility, interactive) \
    ConstructionPieceDefinition{"elysium:construction/" fam "/" var, fam, var, structural, seal, utility, interactive}
constexpr std::array kConstructionPieces{
    PIECE("wall", "full", true, true, false, false), PIECE("wall", "half", true, true, false, false),
    PIECE("wall", "quarter", true, true, false, false), PIECE("wall", "framed", true, true, true, false),
    PIECE("wall", "reinforced", true, true, false, false), PIECE("wall", "insulated", true, true, false, false),
    PIECE("wall", "sealed", true, true, false, false),
    PIECE("floor", "solid", true, true, false, false), PIECE("floor", "grate", true, false, true, false),
    PIECE("floor", "tile", true, true, false, false), PIECE("floor", "raised", true, true, true, false),
    PIECE("floor", "transparent", true, true, false, false), PIECE("floor", "hazard_resistant", true, true, false, false),
    PIECE("roof", "flat", true, true, false, false), PIECE("roof", "slope", true, true, false, false),
    PIECE("roof", "ridge", true, true, false, false), PIECE("roof", "hip", true, true, false, false),
    PIECE("roof", "glass", true, true, false, false), PIECE("roof", "solar_integrated", true, true, true, false),
    PIECE("support", "beam", true, false, false, false), PIECE("support", "column", true, false, false, false),
    PIECE("support", "arch", true, false, false, false), PIECE("support", "truss", true, false, false, false),
    PIECE("support", "buttress", true, false, false, false), PIECE("support", "cable_stay", true, false, false, false),
    PIECE("opening", "door", false, true, false, true), PIECE("opening", "double_door", false, true, false, true),
    PIECE("opening", "hatch", false, true, false, true), PIECE("opening", "gate", false, true, false, true),
    PIECE("opening", "airlock", false, true, true, true), PIECE("opening", "shutter", false, true, false, true),
    PIECE("window", "full_pane", false, true, false, false), PIECE("window", "slit", false, true, false, false),
    PIECE("window", "reinforced", false, true, false, false), PIECE("window", "curved_approximation", false, true, false, false),
    PIECE("window", "emissive", false, true, true, false),
    PIECE("access", "straight_stairs", true, false, false, false), PIECE("access", "corner_stairs", true, false, false, false),
    PIECE("access", "half_step", true, false, false, false), PIECE("access", "ladder", true, false, false, false),
    PIECE("access", "ramp", true, false, false, false), PIECE("access", "lift", true, false, true, true),
    PIECE("industrial", "catwalk", true, false, true, false), PIECE("industrial", "railing", false, false, false, false),
    PIECE("industrial", "pipe", false, false, true, false), PIECE("industrial", "duct", false, false, true, false),
    PIECE("industrial", "cable_tray", false, false, true, false), PIECE("industrial", "conveyor", false, false, true, true),
    PIECE("industrial", "gantry", true, false, true, false),
    PIECE("defense", "barricade", true, false, false, false), PIECE("defense", "firing_slit", true, true, false, false),
    PIECE("defense", "turret_plinth", true, false, true, false), PIECE("defense", "shield_socket", true, false, true, false),
    PIECE("defense", "blast_wall", true, true, false, false),
    PIECE("decoration", "cornice", false, false, false, false), PIECE("decoration", "trim", false, false, false, false),
    PIECE("decoration", "panel", false, false, false, false), PIECE("decoration", "sign", false, false, false, false),
    PIECE("decoration", "banner", false, false, false, false), PIECE("decoration", "light", false, false, true, true),
    PIECE("decoration", "planter", false, false, false, true), PIECE("decoration", "furniture", false, false, false, true),
};
#undef PIECE

constexpr std::array kPlanetClasses{
    PlanetClassDefinition{"elysium:planet_class/barren", "Barren", EA::None, "Vacuum", 24, true, "airless exposed geology; sealed habitats"},
    PlanetClassDefinition{"elysium:planet_class/temperate", "Temperate", EA::None, "None", 9, true, "breathable homestead; farming and broad resources"},
    PlanetClassDefinition{"elysium:planet_class/scorched", "Scorched", EA::Plasma, "Thermal", 14, true, "heat engineering and thermal industry"},
    PlanetClassDefinition{"elysium:planet_class/frozen", "Frozen", EA::Kinetic, "Cryogenic", 14, true, "insulation, ice logistics and subglacial mining"},
    PlanetClassDefinition{"elysium:planet_class/toxic", "Toxic", EA::Neural, "Corrosive", 12, true, "sealed construction, fungi and chemical hazards"},
    PlanetClassDefinition{"elysium:planet_class/irradiated", "Irradiated", EA::Void, "Radiological", 10, true, "shielded mining and exotic Void resources"},
    PlanetClassDefinition{"elysium:planet_class/oceanic", "Oceanic", EA::Dimensional, "Pressure", 11, true, "submerged construction and pressure engineering"},
    PlanetClassDefinition{"elysium:planet_class/anomalous", "Anomalous", EA::Mixed, "Two reduced hazards", 6, false, "Aetherium, research and impossible geometry"},
};

#define BIOME(cls, slug, label) BiomeDefinition{"elysium:biome/" cls "/" slug, "elysium:planet_class/" cls, label}
constexpr std::array kBiomes{
    BIOME("temperate","ocean","Ocean"), BIOME("temperate","beach","Beach"), BIOME("temperate","plains","Plains"),
    BIOME("temperate","forest","Forest"), BIOME("temperate","boreal_forest","Boreal Forest"), BIOME("temperate","rainforest","Rainforest"),
    BIOME("temperate","savanna","Savanna"), BIOME("temperate","desert","Desert"), BIOME("temperate","badlands","Badlands"),
    BIOME("temperate","tundra","Tundra"), BIOME("temperate","highlands","Highlands"), BIOME("temperate","wetland","Wetland"),
    BIOME("temperate","karst","Karst"), BIOME("temperate","alpine_meadow","Alpine Meadow"), BIOME("temperate","river_delta","River Delta"),
    BIOME("temperate","redwood_basin","Redwood Basin"),
    BIOME("barren","impact_basin","Impact Basin"), BIOME("barren","dust_sea","Dust Sea"), BIOME("barren","regolith_plain","Regolith Plain"),
    BIOME("barren","fracture_maze","Fracture Maze"), BIOME("barren","crater_highlands","Crater Highlands"), BIOME("barren","salt_flat","Salt Flat"),
    BIOME("barren","shadow_trench","Shadow Trench"), BIOME("barren","glass_ejecta","Glass Ejecta"),
    BIOME("scorched","basalt_plain","Basalt Plain"), BIOME("scorched","magma_field","Magma Field"), BIOME("scorched","ash_waste","Ash Waste"),
    BIOME("scorched","sulfur_ventland","Sulfur Ventland"), BIOME("scorched","obsidian_ridge","Obsidian Ridge"), BIOME("scorched","char_forest","Char Forest"),
    BIOME("scorched","lava_delta","Lava Delta"), BIOME("scorched","caldera_shelf","Caldera Shelf"), BIOME("scorched","fumarole_garden","Fumarole Garden"),
    BIOME("frozen","ice_sheet","Ice Sheet"), BIOME("frozen","subglacial_ocean","Sub-glacial Ocean"), BIOME("frozen","cryo_ridge","Cryo Ridge"),
    BIOME("frozen","snow_dune","Snow Dune"), BIOME("frozen","blue_ice_canyon","Blue-Ice Canyon"), BIOME("frozen","permafrost_plain","Permafrost Plain"),
    BIOME("frozen","glacier_forest","Glacier Forest"), BIOME("frozen","thermal_refuge","Thermal Refuge"), BIOME("frozen","particle_storm_steppe","Particle-Storm Steppe"),
    BIOME("toxic","spore_deeps","Spore Deeps"), BIOME("toxic","acid_bog","Acid Bog"), BIOME("toxic","fungal_canopy","Fungal Canopy"),
    BIOME("toxic","corrosive_fen","Corrosive Fen"), BIOME("toxic","miasma_forest","Miasma Forest"), BIOME("toxic","resin_flats","Resin Flats"),
    BIOME("toxic","rotwood","Rotwood"), BIOME("toxic","caustic_karst","Caustic Karst"), BIOME("toxic","bloom_basin","Bloom Basin"),
    BIOME("irradiated","glass_sea","Glass Sea"), BIOME("irradiated","dead_city","Dead City"), BIOME("irradiated","reactor_badlands","Reactor Badlands"),
    BIOME("irradiated","ashen_plain","Ashen Plain"), BIOME("irradiated","uranic_ridge","Uranic Ridge"), BIOME("irradiated","buried_complex","Buried Complex"),
    BIOME("irradiated","black_sand","Black Sand"), BIOME("irradiated","radiant_crater","Radiant Crater"),
    BIOME("oceanic","archipelago","Archipelago"), BIOME("oceanic","kelp_shelf","Kelp Shelf"), BIOME("oceanic","reef_garden","Reef Garden"),
    BIOME("oceanic","abyssal_trench","Abyssal Trench"), BIOME("oceanic","volcanic_island","Volcanic Island"), BIOME("oceanic","shallow_lagoon","Shallow Lagoon"),
    BIOME("oceanic","storm_coast","Storm Coast"), BIOME("oceanic","hydrothermal_field","Hydrothermal Field"),
    BIOME("anomalous","folded_mesa","Folded Mesa"), BIOME("anomalous","reverse_cavern","Reverse Cavern"), BIOME("anomalous","floating_shelf","Floating Shelf"),
    BIOME("anomalous","rift_garden","Rift Garden"), BIOME("anomalous","gravity_scar","Gravity Scar"), BIOME("anomalous","timeworn_plain","Timeworn Plain"),
    BIOME("anomalous","impossible_reef","Impossible Reef"), BIOME("anomalous","mirror_waste","Mirror Waste"),
};
#undef BIOME

#define WX(cls, slug, label, effect) WeatherDefinition{"elysium:weather/" cls "/" slug, "elysium:planet_class/" cls, label, effect}
constexpr std::array kWeather{
    WX("temperate","clear","Clear","baseline visibility and solar yield"), WX("temperate","rain","Rain","wet surfaces; reduced visibility and solar"),
    WX("temperate","fog","Fog","low visibility"), WX("temperate","thunderstorm","Thunderstorm","wind, lightning and reduced solar"),
    WX("barren","clear","Clear","vacuum baseline"), WX("barren","dust_plume","Dust Plume","reduced visibility and exposed-machine abrasion"),
    WX("barren","micrometeor","Micrometeor Event","exposed-structure impact risk"),
    WX("scorched","clear","Clear","thermal baseline"), WX("scorched","ash","Ash","reduced visibility and fouling"),
    WX("scorched","firestorm","Firestorm","severe thermal load"), WX("scorched","thermal_surge","Thermal Surge","solar/thermal spike"),
    WX("frozen","clear","Clear","cryogenic baseline"), WX("frozen","snowfall","Snowfall","snow accumulation and visibility loss"),
    WX("frozen","whiteout","Whiteout","severe visibility and movement penalty"), WX("frozen","particle_storm","Particle Storm","shielding and electronics stress"),
    WX("toxic","miasma","Miasma","corrosive/toxic exposure"), WX("toxic","acid_rain","Acid Rain","doubles corrosive pressure on exposed structures"),
    WX("toxic","spore_bloom","Spore Bloom","biological contamination"),
    WX("irradiated","clear","Clear","radiological baseline"), WX("irradiated","ion_storm","Ion Storm","radiation and sensor interference"),
    WX("irradiated","fallout_gust","Fallout Gust","surface contamination transport"),
    WX("oceanic","clear","Clear","marine baseline"), WX("oceanic","rain","Rain","wet weather and wave energy"),
    WX("oceanic","squall","Squall","high wind and wave pressure"), WX("oceanic","cyclone","Cyclone","extreme wind, surge and visibility loss"),
    WX("anomalous","mixed","Mixed Weather","inherits reduced hazards from selected class pair"),
    WX("anomalous","anomaly_state","Anomaly State","local gravity/signal/geometry disturbance"),
};
#undef WX

constexpr std::array kOres{
    OreDefinition{"elysium:ore/coal","Coal","elysium:material/stone",40,220,1,EA::None,"elysium:planet_class/temperate","fuel and carbon"},
    OreDefinition{"elysium:ore/copper","Copper","elysium:material/copper",-20,140,1,EA::Plasma,"elysium:planet_class/scorched","bronze, brass and conduction"},
    OreDefinition{"elysium:ore/tin","Tin","elysium:material/tin",-30,120,1,EA::Neural,"elysium:planet_class/toxic","bronze"},
    OreDefinition{"elysium:ore/zinc","Zinc","elysium:material/zinc",-40,120,1,EA::Plasma,"elysium:planet_class/scorched","brass and refining flux"},
    OreDefinition{"elysium:ore/iron","Iron","elysium:material/iron",-60,180,2,EA::Kinetic,"elysium:planet_class/frozen","steel and general industry"},
    OreDefinition{"elysium:ore/bauxite","Bauxite","elysium:material/bauxite",20,100,2,EA::Dimensional,"elysium:planet_class/oceanic","aluminium"},
    OreDefinition{"elysium:ore/resonant_dust","Resonant Dust","elysium:material/resonant_dust",-300,200,2,EA::Neural,"elysium:planet_class/toxic","power and signal medium"},
    OreDefinition{"elysium:ore/lead","Lead","elysium:material/lead",-90,120,2,EA::Kinetic,"elysium:planet_class/frozen","radiation shielding"},
    OreDefinition{"elysium:ore/silver","Silver","elysium:material/silver",-110,110,3,EA::Neural,"elysium:planet_class/toxic","electronics, scanners and electrum"},
    OreDefinition{"elysium:ore/nickel","Nickel","elysium:material/nickel",-130,110,3,EA::Kinetic,"elysium:planet_class/frozen","invar and constantan"},
    OreDefinition{"elysium:ore/gold","Gold","elysium:material/gold",-160,150,3,EA::Neural,"elysium:planet_class/scorched","trade, optics and electrum"},
    OreDefinition{"elysium:ore/cobalt","Cobalt","elysium:material/cobalt",-200,100,3,EA::Dimensional,"elysium:planet_class/oceanic","alloying and high-temperature tooling"},
    OreDefinition{"elysium:ore/emerald","Emerald","elysium:material/emerald",200,120,3,EA::Neural,"elysium:planet_class/temperate","trade currency and optics"},
    OreDefinition{"elysium:ore/titanium","Titanium","elysium:material/titanium",-230,110,4,EA::Dimensional,"elysium:planet_class/oceanic","hulls, pressure gear and tier-four tools"},
    OreDefinition{"elysium:ore/tungsten","Tungsten","elysium:material/tungsten",-280,100,4,EA::Kinetic,"elysium:planet_class/scorched","thermal shielding"},
    OreDefinition{"elysium:ore/platinum","Platinum","elysium:material/platinum",-300,90,4,EA::Neural,"elysium:planet_class/toxic","catalysts and warp components"},
    OreDefinition{"elysium:ore/diamond","Diamond","elysium:material/diamond",-380,130,4,EA::Dimensional,"elysium:planet_class/oceanic","tools, optics and pressure lattice"},
    OreDefinition{"elysium:ore/osmium","Osmium","elysium:material/osmium",-400,90,4,EA::Void,"elysium:planet_class/irradiated","dense ballast and radiation cores"},
    OreDefinition{"elysium:ore/uranium","Uranium","elysium:material/uranium",-420,100,4,EA::Void,"elysium:planet_class/irradiated","reactor fuel"},
    OreDefinition{"elysium:ore/voidglass","Voidglass","elysium:material/voidglass",-150,150,3,EA::Void,"elysium:planet_class/irradiated","rune material and exotic construction"},
    OreDefinition{"elysium:ore/aetherium","Aetherium","elysium:material/aetherium",-300,130,4,EA::Dimensional,"elysium:planet_class/anomalous","reforge, warp and ascension catalyst"},
    OreDefinition{"elysium:ore/neutronium","Neutronium","elysium:material/neutronium",-470,60,5,EA::Kinetic,"elysium:planet_class/irradiated","endgame tools and structures"},
};

constexpr std::array kItems{
    ItemDefinition{"elysium:item/copper_ingot","Copper Ingot","metal",3000,false}, ItemDefinition{"elysium:item/tin_ingot","Tin Ingot","metal",3001,false},
    ItemDefinition{"elysium:item/iron_ingot","Iron Ingot","metal",3002,false}, ItemDefinition{"elysium:item/carbon","Carbon","industrial",3003,false},
    ItemDefinition{"elysium:item/bronze_ingot","Bronze Ingot","alloy",3004,true}, ItemDefinition{"elysium:item/steel_ingot","Steel Ingot","alloy",3005,true},
    ItemDefinition{"elysium:item/stone_aggregate","Stone Aggregate","industrial",3006,false}, ItemDefinition{"elysium:item/copper_concentrate","Copper Concentrate","ore_concentrate",3007,false},
    ItemDefinition{"elysium:item/tin_concentrate","Tin Concentrate","ore_concentrate",3008,false}, ItemDefinition{"elysium:item/iron_concentrate","Iron Concentrate","ore_concentrate",3009,false},
    ItemDefinition{"elysium:item/sealant","Sealant","construction",3010,true}, ItemDefinition{"elysium:item/copper_wire","Copper Wire","component",3011,false},
    ItemDefinition{"elysium:item/steel_frame","Steel Frame","component",3012,false}, ItemDefinition{"elysium:item/actuator","Actuator","component",3013,false},
    ItemDefinition{"elysium:item/control_circuit","Control Circuit","component",3014,false}, ItemDefinition{"elysium:item/sensor_package","Sensor Package","component",3015,false},
    ItemDefinition{"elysium:item/turret_ammo","Turret Ammunition","defense",3016,false}, ItemDefinition{"elysium:item/repair_kit","Repair Kit","field",3017,true},
    ItemDefinition{"elysium:item/filter_cartridge","Filter Cartridge","habitat",3018,true}, ItemDefinition{"elysium:item/machine_casing","Machine Casing","component",3019,false},
    ItemDefinition{"elysium:item/composite_panel","Composite Panel","construction",3020,true},
    ItemDefinition{"elysium:item/zinc_ore","Zinc Ore","future_raw",std::nullopt,false}, ItemDefinition{"elysium:item/nickel_ore","Nickel Ore","future_raw",std::nullopt,false},
    ItemDefinition{"elysium:item/silver_ore","Silver Ore","future_raw",std::nullopt,false}, ItemDefinition{"elysium:item/gold_ore","Gold Ore","future_raw",std::nullopt,false},
    ItemDefinition{"elysium:item/brass_ingot","Brass Ingot","alloy",std::nullopt,false}, ItemDefinition{"elysium:item/invar_ingot","Invar Ingot","alloy",std::nullopt,false},
    ItemDefinition{"elysium:item/constantan_ingot","Constantan Ingot","alloy",std::nullopt,false}, ItemDefinition{"elysium:item/electrum_ingot","Electrum Ingot","alloy",std::nullopt,false},
};

constexpr std::array kMachines{
    MachineDefinition{"elysium:machine/burner_generator","Burner Generator",MD::Power,2,1,"fixed interval","fuel and venting",0},
    MachineDefinition{"elysium:machine/battery_bank","Battery Bank",MD::Power,2,1,"event/fixed","network connection",1},
    MachineDefinition{"elysium:machine/atmosphere_unit","Atmosphere Unit",MD::Habitat,2,1,"fixed interval","sealed bounded room",2},
    MachineDefinition{"elysium:machine/storage_crate","Storage Crate",MD::Logistics,1,3,"event-driven","accessible placement",3},
    MachineDefinition{"elysium:machine/airlock_controller","Airlock Controller",MD::Habitat,2,1,"event-driven","paired doors and chamber",4},
    MachineDefinition{"elysium:machine/sensor_mast","Sensor Mast",MD::Defense,2,2,"fixed interval","exposed mast",5},
    MachineDefinition{"elysium:machine/turret","Turret",MD::Defense,3,2,"tactical","ammo, power and line of sight",6},
    MachineDefinition{"elysium:machine/shield_pylon","Shield Pylon",MD::Defense,4,2,"tactical","power network",7},
    MachineDefinition{"elysium:machine/logic_controller","Logic Controller",MD::Command,3,2,"event-driven","connected automation network",8},
    MachineDefinition{"elysium:machine/furnace","Furnace",MD::Metallurgy,1,4,"fixed interval","fuel",9},
    MachineDefinition{"elysium:machine/alloy_crucible","Alloy Crucible",MD::Metallurgy,2,4,"fixed interval","power",10},
    MachineDefinition{"elysium:machine/refinery","Refinery",MD::Metallurgy,3,4,"fixed interval","power",11},
    MachineDefinition{"elysium:machine/network_storage","Network Storage",MD::Logistics,3,3,"event-driven","power and network",12},
    MachineDefinition{"elysium:machine/conveyor","Conveyor",MD::Logistics,3,3,"fixed interval","explicit links",13},
    MachineDefinition{"elysium:machine/sorter","Sorter",MD::Logistics,3,3,"fixed interval","filter inventory and links",14},
    MachineDefinition{"elysium:machine/cargo_loader","Cargo Loader",MD::Logistics,3,3,"fixed interval","linked inventories",15},
    MachineDefinition{"elysium:machine/crusher","Crusher",MD::Chemical,2,4,"fixed interval","power",16},
    MachineDefinition{"elysium:machine/chemical_vat","Chemical Vat",MD::Chemical,3,4,"fixed interval","power and compatible environment",17},
    MachineDefinition{"elysium:machine/fabricator","Fabricator",MD::Fabrication,4,4,"fixed interval","power",18},
    MachineDefinition{"elysium:machine/extractor","Extractor",MD::Extraction,3,4,"fixed interval","generated ore column and power",19},
    MachineDefinition{"elysium:machine/survival_workbench","Survival Workbench",MD::Survival,0,0,"event-driven","work surface",std::nullopt},
    MachineDefinition{"elysium:machine/kitchen","Kitchen",MD::Survival,1,5,"fixed interval","food-safe room",std::nullopt},
    MachineDefinition{"elysium:machine/brewer","Brewer / Fermenter",MD::Survival,1,5,"fixed interval","food-safe storage",std::nullopt},
    MachineDefinition{"elysium:machine/textile_bench","Textile Bench",MD::Survival,1,5,"event-driven","dry work area",std::nullopt},
    MachineDefinition{"elysium:machine/blast_furnace","Blast Furnace",MD::Metallurgy,3,4,"fixed interval","high heat and fuel/power",std::nullopt},
    MachineDefinition{"elysium:machine/foundry_caster","Foundry / Caster",MD::Metallurgy,3,4,"fixed interval","molten feed and molds",std::nullopt},
    MachineDefinition{"elysium:machine/machine_shop","Machine Shop",MD::Metallurgy,3,4,"fixed interval","power and precision tools",std::nullopt},
    MachineDefinition{"elysium:machine/arc_smelter","Arc Smelter",MD::Metallurgy,4,4,"fixed interval","high power and exotic shielding",std::nullopt},
    MachineDefinition{"elysium:machine/electronics_bench","Electronics Bench",MD::Fabrication,3,4,"event-driven","clean dry area",std::nullopt},
    MachineDefinition{"elysium:machine/composite_press","Composite Press",MD::Fabrication,3,4,"fixed interval","power",std::nullopt},
    MachineDefinition{"elysium:machine/optics_bench","Optics Bench",MD::Fabrication,3,4,"event-driven","clean precision workspace",std::nullopt},
    MachineDefinition{"elysium:machine/elysium_workstation","Elysium Workstation",MD::Fabrication,3,4,"event-driven","stable item-state operations",std::nullopt},
    MachineDefinition{"elysium:machine/separator_centrifuge","Separator / Centrifuge",MD::Chemical,3,4,"fixed interval","power",std::nullopt},
    MachineDefinition{"elysium:machine/medical_synthesizer","Medical Synthesizer",MD::Chemical,3,1,"fixed interval","clean medical supply",std::nullopt},
    MachineDefinition{"elysium:machine/nutrient_mixer","Nutrient Mixer",MD::Chemical,2,5,"fixed interval","water and organics",std::nullopt},
    MachineDefinition{"elysium:machine/solar_collector","Solar Collector",MD::Power,3,1,"fixed interval","illumination and exposure",std::nullopt},
    MachineDefinition{"elysium:machine/wind_storm_harvester","Wind / Storm Harvester",MD::Power,3,1,"fixed interval","exposed windy placement",std::nullopt},
    MachineDefinition{"elysium:machine/thermal_collector","Thermal Collector",MD::Power,3,1,"fixed interval","temperature gradient",std::nullopt},
    MachineDefinition{"elysium:machine/uranium_reactor","Uranium Reactor",MD::Power,4,1,"fixed interval","shielding and cooling",std::nullopt},
    MachineDefinition{"elysium:machine/aetheric_converter","Aetheric Converter",MD::Power,5,1,"fixed interval","exotic containment",std::nullopt},
    MachineDefinition{"elysium:machine/capacitor_array","Capacitor Array",MD::Power,4,2,"event/fixed","high-discharge network",std::nullopt},
    MachineDefinition{"elysium:machine/water_recycler","Water Recycler",MD::Habitat,2,1,"fixed interval","water/atmosphere service",std::nullopt},
    MachineDefinition{"elysium:machine/climate_unit","Climate Unit",MD::Habitat,3,1,"fixed interval","sealed room",std::nullopt},
    MachineDefinition{"elysium:machine/hydroponic_bed","Hydroponic Bed",MD::Habitat,2,5,"ecology interval","light, water, nutrients",std::nullopt},
    MachineDefinition{"elysium:machine/cryo_storage","Cryo Storage",MD::Habitat,3,1,"fixed interval","power",std::nullopt},
    MachineDefinition{"elysium:machine/med_bay_unit","Med Bay Unit",MD::Habitat,3,1,"event-driven","hospital room and supplies",std::nullopt},
    MachineDefinition{"elysium:machine/decon_unit","Decon Unit",MD::Habitat,3,1,"event-driven","water, filtration and drain",std::nullopt},
    MachineDefinition{"elysium:machine/pipe_pump","Pipe Pump",MD::Logistics,3,3,"fixed interval","bounded fluid route",std::nullopt},
    MachineDefinition{"elysium:machine/drone_port","Drone Port",MD::Logistics,4,3,"fixed interval","power and local navigation",std::nullopt},
    MachineDefinition{"elysium:machine/rail_loader","Rail Loader",MD::Logistics,3,3,"event-driven","rail stop and manifest",std::nullopt},
    MachineDefinition{"elysium:machine/rail_stop_brake","Rail Stop / Brake",MD::Logistics,3,3,"event-driven","rail network",std::nullopt},
    MachineDefinition{"elysium:machine/blast_door","Blast Door",MD::Defense,3,2,"event-driven","powered hardened opening",std::nullopt},
    MachineDefinition{"elysium:machine/decoy_beacon","Decoy Beacon",MD::Defense,4,2,"event-driven","power and sensor coverage",std::nullopt},
    MachineDefinition{"elysium:machine/interdictor","Interdictor",MD::Defense,5,2,"tactical","high power and exotic components",std::nullopt},
    MachineDefinition{"elysium:machine/security_scanner","Security Scanner",MD::Defense,3,2,"event-driven","access-control route",std::nullopt},
    MachineDefinition{"elysium:machine/research_console","Research Console",MD::Command,3,4,"event-driven","archive/data network",std::nullopt},
    MachineDefinition{"elysium:machine/map_room_console","Map Room Console",MD::Command,3,4,"event-driven","survey data",std::nullopt},
    MachineDefinition{"elysium:machine/archive_server","Archive Server",MD::Command,3,1,"event-driven","protected data storage",std::nullopt},
    MachineDefinition{"elysium:machine/justice_terminal","Justice Terminal",MD::Command,3,1,"event-driven","jurisdiction and case data",std::nullopt},
    MachineDefinition{"elysium:machine/vehicle_bay","Vehicle Bay",MD::Vehicles,3,3,"event-driven","hangar space",std::nullopt},
    MachineDefinition{"elysium:machine/drydock","Drydock",MD::Vehicles,4,3,"event-driven","orbital or hangar construction",std::nullopt},
    MachineDefinition{"elysium:machine/landing_pad_core","Landing Pad Core",MD::Vehicles,3,3,"event-driven","clear landing zone",std::nullopt},
    MachineDefinition{"elysium:machine/orbital_cargo_depot","Orbital Cargo Depot",MD::Vehicles,4,3,"strategic interval","orbital logistics node",std::nullopt},
    MachineDefinition{"elysium:machine/orbital_refinery","Orbital Refinery",MD::Vehicles,4,4,"strategic interval","orbital bulk feed",std::nullopt},
    MachineDefinition{"elysium:machine/gate_anchor","Gate Anchor",MD::Vehicles,5,2,"strategic interval","route infrastructure and defense",std::nullopt},
    MachineDefinition{"elysium:machine/registry_beacon_object","Registry Beacon Object",MD::Command,3,1,"event-driven","claimable planet",std::nullopt},
};

constexpr IngredientRef noneIngredient{"",0};
#define IN1(a,ca) std::array<IngredientRef,3>{{IngredientRef{a,ca},noneIngredient,noneIngredient}}
#define IN2(a,ca,b,cb) std::array<IngredientRef,3>{{IngredientRef{a,ca},IngredientRef{b,cb},noneIngredient}}
#define IN3(a,ca,b,cb,c,cc) std::array<IngredientRef,3>{{IngredientRef{a,ca},IngredientRef{b,cb},IngredientRef{c,cc}}}
constexpr std::array kRecipes{
    RecipeDefinition{"elysium:recipe/smelt_copper","Smelt Copper","elysium:machine/furnace",IN1("elysium:block/copper_ore",1),1,"elysium:item/copper_ingot",1,RS::Live,1},
    RecipeDefinition{"elysium:recipe/smelt_tin","Smelt Tin","elysium:machine/furnace",IN1("elysium:block/tin_ore",1),1,"elysium:item/tin_ingot",1,RS::Live,2},
    RecipeDefinition{"elysium:recipe/smelt_iron","Smelt Iron","elysium:machine/furnace",IN1("elysium:block/iron_ore",1),1,"elysium:item/iron_ingot",1,RS::Live,3},
    RecipeDefinition{"elysium:recipe/char_coal","Carbonize Coal","elysium:machine/furnace",IN1("elysium:block/coal_ore",1),1,"elysium:item/carbon",1,RS::Live,4},
    RecipeDefinition{"elysium:recipe/alloy_bronze","Alloy Bronze","elysium:machine/alloy_crucible",IN2("elysium:item/copper_ingot",3,"elysium:item/tin_ingot",1),2,"elysium:item/bronze_ingot",4,RS::Live,5},
    RecipeDefinition{"elysium:recipe/alloy_steel","Alloy Steel","elysium:machine/alloy_crucible",IN2("elysium:item/iron_ingot",1,"elysium:item/carbon",1),2,"elysium:item/steel_ingot",1,RS::Live,6},
    RecipeDefinition{"elysium:recipe/refine_copper","Refine Copper","elysium:machine/refinery",IN1("elysium:block/copper_ore",1),1,"elysium:item/copper_ingot",2,RS::Live,7},
    RecipeDefinition{"elysium:recipe/refine_tin","Refine Tin","elysium:machine/refinery",IN1("elysium:block/tin_ore",1),1,"elysium:item/tin_ingot",2,RS::Live,8},
    RecipeDefinition{"elysium:recipe/refine_iron","Refine Iron","elysium:machine/refinery",IN1("elysium:block/iron_ore",1),1,"elysium:item/iron_ingot",2,RS::Live,9},
    RecipeDefinition{"elysium:recipe/crush_stone","Crush Stone","elysium:machine/crusher",IN1("elysium:block/stone",1),1,"elysium:item/stone_aggregate",2,RS::Live,10},
    RecipeDefinition{"elysium:recipe/crush_copper","Crush Copper Ore","elysium:machine/crusher",IN1("elysium:block/copper_ore",1),1,"elysium:item/copper_concentrate",2,RS::Live,11},
    RecipeDefinition{"elysium:recipe/crush_tin","Crush Tin Ore","elysium:machine/crusher",IN1("elysium:block/tin_ore",1),1,"elysium:item/tin_concentrate",2,RS::Live,12},
    RecipeDefinition{"elysium:recipe/crush_iron","Crush Iron Ore","elysium:machine/crusher",IN1("elysium:block/iron_ore",1),1,"elysium:item/iron_concentrate",2,RS::Live,13},
    RecipeDefinition{"elysium:recipe/refine_copper_concentrate","Refine Copper Concentrate","elysium:machine/refinery",IN1("elysium:item/copper_concentrate",1),1,"elysium:item/copper_ingot",1,RS::Live,14},
    RecipeDefinition{"elysium:recipe/refine_tin_concentrate","Refine Tin Concentrate","elysium:machine/refinery",IN1("elysium:item/tin_concentrate",1),1,"elysium:item/tin_ingot",1,RS::Live,15},
    RecipeDefinition{"elysium:recipe/refine_iron_concentrate","Refine Iron Concentrate","elysium:machine/refinery",IN1("elysium:item/iron_concentrate",1),1,"elysium:item/iron_ingot",1,RS::Live,16},
    RecipeDefinition{"elysium:recipe/mix_sealant","Mix Sealant","elysium:machine/chemical_vat",IN2("elysium:item/stone_aggregate",2,"elysium:item/carbon",1),2,"elysium:item/sealant",1,RS::Live,17},
    RecipeDefinition{"elysium:recipe/fabricate_copper_wire","Draw Copper Wire","elysium:machine/fabricator",IN1("elysium:item/copper_ingot",1),1,"elysium:item/copper_wire",4,RS::Live,18},
    RecipeDefinition{"elysium:recipe/fabricate_steel_frame","Fabricate Steel Frame","elysium:machine/fabricator",IN1("elysium:item/steel_ingot",2),1,"elysium:item/steel_frame",1,RS::Live,19},
    RecipeDefinition{"elysium:recipe/fabricate_actuator","Fabricate Actuator","elysium:machine/fabricator",IN2("elysium:item/steel_ingot",1,"elysium:item/bronze_ingot",1),2,"elysium:item/actuator",1,RS::Live,20},
    RecipeDefinition{"elysium:recipe/fabricate_control_circuit","Fabricate Control Circuit","elysium:machine/fabricator",IN2("elysium:item/copper_wire",2,"elysium:item/carbon",1),2,"elysium:item/control_circuit",1,RS::Live,21},
    RecipeDefinition{"elysium:recipe/fabricate_sensor_package","Fabricate Sensor Package","elysium:machine/fabricator",IN3("elysium:item/control_circuit",1,"elysium:item/copper_wire",2,"elysium:item/steel_frame",1),3,"elysium:item/sensor_package",1,RS::Live,22},
    RecipeDefinition{"elysium:recipe/fabricate_turret_ammo","Fabricate Turret Ammunition","elysium:machine/fabricator",IN2("elysium:item/steel_ingot",1,"elysium:item/carbon",1),2,"elysium:item/turret_ammo",12,RS::Live,23},
    RecipeDefinition{"elysium:recipe/fabricate_repair_kit","Fabricate Repair Kit","elysium:machine/fabricator",IN2("elysium:item/steel_frame",1,"elysium:item/sealant",1),2,"elysium:item/repair_kit",1,RS::Live,24},
    RecipeDefinition{"elysium:recipe/fabricate_filter_cartridge","Fabricate Filter Cartridge","elysium:machine/fabricator",IN2("elysium:item/carbon",1,"elysium:block/planks",1),2,"elysium:item/filter_cartridge",2,RS::Live,25},
    RecipeDefinition{"elysium:recipe/fabricate_machine_casing","Fabricate Machine Casing","elysium:machine/fabricator",IN2("elysium:item/steel_frame",1,"elysium:item/copper_wire",2),2,"elysium:item/machine_casing",1,RS::Live,26},
    RecipeDefinition{"elysium:recipe/fabricate_composite_panel","Fabricate Composite Panel","elysium:machine/fabricator",IN3("elysium:block/planks",1,"elysium:item/sealant",1,"elysium:item/copper_wire",1),3,"elysium:item/composite_panel",2,RS::Live,27},
    RecipeDefinition{"elysium:recipe/alloy_brass","Alloy Brass","elysium:machine/alloy_crucible",IN2("elysium:item/copper_ingot",3,"elysium:item/zinc_ore",1),2,"elysium:item/brass_ingot",4,RS::Specification,std::nullopt},
    RecipeDefinition{"elysium:recipe/alloy_invar","Alloy Invar","elysium:machine/alloy_crucible",IN2("elysium:item/iron_ingot",2,"elysium:item/nickel_ore",1),2,"elysium:item/invar_ingot",3,RS::Specification,std::nullopt},
    RecipeDefinition{"elysium:recipe/alloy_constantan","Alloy Constantan","elysium:machine/alloy_crucible",IN2("elysium:item/copper_ingot",1,"elysium:item/nickel_ore",1),2,"elysium:item/constantan_ingot",2,RS::Specification,std::nullopt},
    RecipeDefinition{"elysium:recipe/alloy_electrum","Alloy Electrum","elysium:machine/alloy_crucible",IN2("elysium:item/gold_ore",1,"elysium:item/silver_ore",1),2,"elysium:item/electrum_ingot",2,RS::Specification,std::nullopt},
};
#undef IN1
#undef IN2
#undef IN3

constexpr std::array<int,12> gImperial{1,0,0,0,0,0,0,1,0,0,0,1};
constexpr std::array<int,12> gDruun{0,1,0,2,0,0,0,0,0,0,0,0};
constexpr std::array<int,12> gVeylari{0,0,0,0,0,1,0,0,2,0,0,0};
constexpr std::array<int,12> gKorrath{0,0,0,0,2,0,1,0,0,0,0,0};
constexpr std::array<int,12> gLumari{0,0,0,0,0,0,0,0,1,2,0,0};
constexpr std::array<int,12> gUnsworn{0,0,0,1,0,0,0,0,0,0,2,0};
constexpr std::array kRaces{
    RaceDefinition{"elysium:race/imperial","Imperial",{4,4,3,4,3,3,3,5,3,3,3,6},gImperial,"Sanctioned Answer: level-shaped reflection"},
    RaceDefinition{"elysium:race/druun","Druun",{6,8,5,8,2,3,2,2,1,3,2,2},gDruun,"Cold Blood: attack rises with missing health"},
    RaceDefinition{"elysium:race/veylari","Veylari",{3,2,2,3,6,8,5,1,7,2,3,2},gVeylari,"Lightfeather: deep fall-distance mitigation"},
    RaceDefinition{"elysium:race/korrath","Korrath",{5,3,3,4,8,4,7,2,2,1,4,1},gKorrath,"Molt: regeneration after being untouched"},
    RaceDefinition{"elysium:race/lumari","Lumari",{2,1,5,2,3,3,3,3,8,9,3,2},gLumari,"Photonic: doubled shield; fire resistance with general vulnerability"},
    RaceDefinition{"elysium:race/unsworn","Unsworn",{4,3,3,5,6,4,4,2,2,2,9,0},gUnsworn,"Uncounted: standing moves slower and Suspicion decays faster"},
};

constexpr std::array<int,12> cgMedicae{1,0,0,0,0,0,0,0,0,0,0,1};
constexpr std::array<int,12> cgFactor{0,0,0,0,0,0,0,0,0,0,1,1};
constexpr std::array<int,12> cgArtificer{0,0,0,0,0,0,0,0,1,0,0,1};
constexpr std::array<int,12> cgEnforcer{0,1,0,1,0,0,0,0,0,0,0,0};
constexpr std::array<int,12> cgPsion{0,0,0,0,0,0,0,0,1,1,0,0};
constexpr std::array<int,12> cgVoidrunner{0,0,0,0,1,0,1,0,0,0,0,0};
constexpr std::array<int,12> cgReclaimer{0,1,0,0,0,0,0,0,0,0,1,0};
constexpr std::array<int,12> cgWarden{0,0,1,0,0,0,0,1,0,0,0,0};
constexpr std::array<int,12> cgMarksman{0,0,0,0,1,1,0,0,0,0,0,0};
constexpr std::array kClasses{
    ClassDefinition{"elysium:class/medicae","Medicae",cgMedicae,"Triage Field: self and nearby ally regeneration"},
    ClassDefinition{"elysium:class/factor","Factor",cgFactor,"Profiteer: extra loot-roll chance"},
    ClassDefinition{"elysium:class/artificer","Artificer",cgArtificer,"Field Repair: durability save and improved reforge quality"},
    ClassDefinition{"elysium:class/enforcer","Enforcer",cgEnforcer,"Sanctioned Force: bonus versus Unsworn; reduced Suspicion gain"},
    ClassDefinition{"elysium:class/psion","Psion",cgPsion,"Resonance: amplified psionic scale"},
    ClassDefinition{"elysium:class/voidrunner","Voidrunner",cgVoidrunner,"Slipstream: half fall damage"},
    ClassDefinition{"elysium:class/reclaimer","Reclaimer",cgReclaimer,"Prospector: chance to double ore"},
    ClassDefinition{"elysium:class/warden","Warden",cgWarden,"Bulwark: doubled reflection below half health"},
    ClassDefinition{"elysium:class/marksman","Marksman",cgMarksman,"Called Shot: critical multiplier 2.25"},
};

constexpr std::array kRunes{
    RuneDefinition{"elysium:rune/voidward","Voidward",EA::Void,"armor toughness","resistance while below 40% health"},
    RuneDefinition{"elysium:rune/plasmaforge","Plasmaforge",EA::Plasma,"attack damage","Strength while above 70% health"},
    RuneDefinition{"elysium:rune/neuralspike","Neuralspike",EA::Neural,"attack speed","unconditional haste"},
    RuneDefinition{"elysium:rune/dimensionalshift","Dimensionalshift",EA::Dimensional,"movement speed","slow falling after a long fall"},
    RuneDefinition{"elysium:rune/kineticsurge","Kineticsurge",EA::Kinetic,"knockback resistance","unconditional jump boost"},
    RuneDefinition{"elysium:rune/stabilizer","Stabilizer",EA::None,"none","periodic healing while damaged"},
    RuneDefinition{"elysium:rune/reflex","Reflex",EA::None,"none","dodge chance"},
    RuneDefinition{"elysium:rune/barrier","Barrier",EA::None,"none","absorption cap and refill"},
    RuneDefinition{"elysium:rune/plasma_core","Plasma Core",EA::None,"none","fire and explosion reduction"},
};

constexpr std::array kGearMaterials{
    GearMaterialDefinition{"elysium:gear_material/voidglass","Voidglass",EA::Void,0,"diamond/titanium gate",false,"elysium:gear_material/voidglass"},
    GearMaterialDefinition{"elysium:gear_material/aetherium","Aetherium",EA::Dimensional,1,"diamond/titanium gate",false,"elysium:gear_material/aetherium"},
    GearMaterialDefinition{"elysium:gear_material/neutronium","Neutronium",EA::Kinetic,2,"neutronium gate",false,"elysium:gear_material/neutronium"},
    GearMaterialDefinition{"elysium:gear_material/copper","Copper",EA::Plasma,0,"iron-equivalent",false,"elysium:gear_material/copper"},
    GearMaterialDefinition{"elysium:gear_material/iron","Iron",EA::Kinetic,0,"iron-equivalent",false,"elysium:gear_material/iron"},
    GearMaterialDefinition{"elysium:gear_material/gold","Gold",EA::Neural,0,"iron-equivalent",false,"elysium:gear_material/gold"},
    GearMaterialDefinition{"elysium:gear_material/diamond","Diamond",EA::Dimensional,0,"diamond-equivalent",false,"elysium:gear_material/diamond"},
    GearMaterialDefinition{"elysium:gear_material/netherite_legacy","Netherite (legacy compatibility)",EA::Void,1,"legacy-only",true,"elysium:gear_material/neutronium"},
    GearMaterialDefinition{"elysium:gear_material/tin","Tin",EA::Neural,0,"iron-equivalent",false,"elysium:gear_material/tin"},
    GearMaterialDefinition{"elysium:gear_material/silver","Silver",EA::Neural,0,"iron-equivalent",false,"elysium:gear_material/silver"},
    GearMaterialDefinition{"elysium:gear_material/platinum","Platinum",EA::Neural,1,"diamond-equivalent",false,"elysium:gear_material/platinum"},
    GearMaterialDefinition{"elysium:gear_material/constantan","Constantan",EA::Neural,1,"diamond-equivalent",false,"elysium:gear_material/constantan"},
    GearMaterialDefinition{"elysium:gear_material/electrum","Electrum",EA::Neural,1,"diamond-equivalent",false,"elysium:gear_material/electrum"},
    GearMaterialDefinition{"elysium:gear_material/zinc","Zinc",EA::Plasma,0,"iron-equivalent",false,"elysium:gear_material/zinc"},
    GearMaterialDefinition{"elysium:gear_material/bronze","Bronze",EA::Plasma,0,"iron-equivalent",false,"elysium:gear_material/bronze"},
    GearMaterialDefinition{"elysium:gear_material/brass","Brass",EA::Plasma,0,"iron-equivalent",false,"elysium:gear_material/brass"},
    GearMaterialDefinition{"elysium:gear_material/lead","Lead",EA::Kinetic,0,"iron-equivalent",false,"elysium:gear_material/lead"},
    GearMaterialDefinition{"elysium:gear_material/nickel","Nickel",EA::Kinetic,0,"iron-equivalent",false,"elysium:gear_material/nickel"},
    GearMaterialDefinition{"elysium:gear_material/steel","Steel",EA::Kinetic,1,"diamond-equivalent",false,"elysium:gear_material/steel"},
    GearMaterialDefinition{"elysium:gear_material/invar","Invar",EA::Kinetic,1,"diamond-equivalent",false,"elysium:gear_material/invar"},
    GearMaterialDefinition{"elysium:gear_material/tungsten","Tungsten",EA::Kinetic,1,"diamond-equivalent",false,"elysium:gear_material/tungsten"},
    GearMaterialDefinition{"elysium:gear_material/aluminum","Aluminum",EA::Dimensional,0,"iron-equivalent",false,"elysium:gear_material/aluminum"},
    GearMaterialDefinition{"elysium:gear_material/titanium","Titanium",EA::Dimensional,1,"diamond-equivalent",false,"elysium:gear_material/titanium"},
    GearMaterialDefinition{"elysium:gear_material/cobalt","Cobalt",EA::Dimensional,1,"diamond-equivalent",false,"elysium:gear_material/cobalt"},
    GearMaterialDefinition{"elysium:gear_material/osmium","Osmium",EA::Void,1,"diamond-equivalent",false,"elysium:gear_material/osmium"},
    GearMaterialDefinition{"elysium:gear_material/uranium","Uranium",EA::Void,1,"diamond-equivalent",false,"elysium:gear_material/uranium"},
};

#define TR(slug,label,kind,slot,lvl,rule) TrinketDefinition{"elysium:trinket/" slug,label,kind,slot,lvl,rule}
constexpr std::array kTrinkets{
    TR("widows_thimble","Widow's Thimble",TK::Found,"ring",5,"first damaging blow at full health deals zero damage"),
    TR("cracked_reliquary","Cracked Reliquary",TK::Found,"necklace",5,"regeneration x3 below one-third health and x0.4 otherwise"),
    TR("nine_tenths_charm","Nine-Tenths Charm",TK::Found,"charm",15,"dodge chance scales with missing-health fraction"),
    TR("iron_discipline","Iron Discipline",TK::Found,"belt",10,"takes x0.55 damage while crouching"),
    TR("ashen_mantle","Ashen Mantle",TK::Found,"back",15,"reflects 30% while on fire"),
    TR("splintbone_fetish","Splintbone Fetish",TK::Found,"charm",20,"doubles reflection but contributes none itself"),
    TR("empty_reliquary","Empty Reliquary",TK::Found,"necklace",20,"shield x2 and regeneration x0.5"),
    TR("gravebound_coil","Gravebound Coil",TK::Found,"belt",10,"no fall damage; Suspicion gains x1.5"),
    TR("ratchet_gauntlet","Ratchet Gauntlet",TK::Found,"hands",15,"critical multiplier 2.6; attacks x0.85"),
    TR("duellists_cuff","Duellist's Cuff",TK::Found,"ring",10,"x1.35 damage against full-health targets"),
    TR("hollow_chime","Hollow Chime",TK::Found,"necklace",20,"20% lifesteal against targets below half health"),
    TR("carrion_signet","Carrion Signet",TK::Found,"ring",15,"on kill heal 10% victim max health capped at 6"),
    TR("pale_tourniquet","Pale Tourniquet",TK::Found,"hands",20,"a hit of 6 or more grants three seconds of Resistance"),
    TR("deadmans_ledger","Deadman's Ledger",TK::Found,"charm",25,"Favor gains x2 while Hunted"),
    TR("quiet_hours","Quiet Hours",TK::Found,"back",1,"both standing meters move at half rate"),
    TR("unsworn_bell","Unsworn Bell",TK::Found,"necklace",10,"both standing meters move x1.6"),
    TR("auditors_seal","Auditor's Seal",TK::Found,"charm",15,"standing decay rate x3"),
    TR("long_memory","Long Memory",TK::Found,"charm",25,"standing does not decay"),
    TR("tithe_bracelet","Tithe Bracelet",TK::Found,"ring",20,"+1 Favor on every kill"),
    TR("debtors_knot","Debtor's Knot",TK::Found,"charm",25,"+35% extra drop chance while Hunted"),
    TR("prospectors_lens","Prospector's Lens",TK::Found,"head",10,"doubles ore; experience x0.6"),
    TR("cartographers_nail","Cartographer's Nail",TK::Found,"hands",5,"spends no durability while crouching"),
    TR("longsight","Longsight",TK::Found,"head",15,"experience x1.6; attacks x0.85"),
    TR("reforgers_loupe","Reforger's Loupe",TK::Found,"head",20,"reforge quality x1.75; psionic scale x0.8"),
    TR("aetherium_band","Aetherium Band",TK::Crafted,"ring",10,"attack x1.15 at tier zero"),
    TR("executioners_grip","Executioner's Grip",TK::Crafted,"hands",20,"critical multiplier increases with ascension"),
    TR("bloodlet_ring","Bloodlet Ring",TK::Crafted,"ring",20,"lifesteal 6% at tier zero"),
    TR("voidglass_pendant","Voidglass Pendant",TK::Crafted,"necklace",15,"psionic x1.20 at tier zero"),
    TR("neutronium_band","Neutronium Band",TK::Crafted,"ring",10,"8% damage removed at tier zero"),
    TR("kinetic_spur","Kinetic Spur",TK::Crafted,"belt",15,"dodge 5% at tier zero"),
    TR("thornplate","Thornplate",TK::Crafted,"back",15,"reflect 10% at tier zero"),
    TR("wardens_gorget","Warden's Gorget",TK::Crafted,"necklace",20,"shield x1.25 at tier zero"),
    TR("dimensional_anchor","Dimensional Anchor",TK::Crafted,"belt",5,"ignore three blocks of fall distance, scaling with ascension"),
    TR("plasma_cord","Plasma Cord",TK::Crafted,"belt",10,"regeneration x1.30 at tier zero"),
    TR("neural_filament","Neural Filament",TK::Crafted,"head",10,"experience x1.20 at tier zero"),
    TR("favored_sigil","Favored Sigil",TK::Crafted,"charm",5,"Favor x1.25 at tier zero"),
    TR("shrouded_sigil","Shrouded Sigil",TK::Crafted,"charm",5,"15% of Suspicion gain removed at tier zero"),
    TR("prospect_charm","Prospect Charm",TK::Crafted,"charm",10,"extra drop chance 8% at tier zero"),
    TR("artificers_loupe","Artificer's Loupe",TK::Crafted,"head",15,"reforge x1.20 at tier zero"),
    TR("miners_rig","Miner's Rig",TK::Crafted,"hands",10,"20% durability save and independent 12% ore double"),
};
#undef TR

constexpr std::array kCrops{
    CropDefinition{"elysium:crop/grain","Grain","grain + seed","staple; highest calories per plot"},
    CropDefinition{"elysium:crop/root","Root","tuber","raw/roasted food and livestock feed"},
    CropDefinition{"elysium:crop/legume","Legume","pulse","best cooked; high saturation"},
    CropDefinition{"elysium:crop/gourd","Gourd","gourd","bulk food and container material"},
    CropDefinition{"elysium:crop/fibre","Fibre Crop","fibre","cloth, rope and suit liner; not food"},
    CropDefinition{"elysium:crop/reed","Reed","reed","sugar, paper and fermentation/fuel base"},
};

constexpr std::array kFoods{
    FoodDefinition{"elysium:food/bread","Bread","staple","grain-based high-calorie staple",EA::None},
    FoodDefinition{"elysium:food/roasted_tuber","Roasted Tuber","staple","simple cooked root food",EA::None},
    FoodDefinition{"elysium:food/pulse_stew","Pulse Stew","staple","high-saturation legume meal",EA::None},
    FoodDefinition{"elysium:food/gourd_roast","Gourd Roast","staple","bulk cooked food",EA::None},
    FoodDefinition{"elysium:food/protein_ration","Protein Ration","ration","preserved crop/livestock field food",EA::None},
    FoodDefinition{"elysium:food/fermented_reed","Fermented Reed","fermented","reed-derived drink/food culture base",EA::None},
    FoodDefinition{"elysium:food/thermal_stew","Thermal Stew","hazard meal","temporary Thermal resistance",EA::Plasma},
    FoodDefinition{"elysium:food/cryo_broth","Cryo Broth","hazard meal","temporary Cryogenic resistance",EA::Kinetic},
    FoodDefinition{"elysium:food/antitoxin_meal","Antitoxin Meal","hazard meal","temporary Corrosive resistance",EA::Neural},
    FoodDefinition{"elysium:food/radiological_field_meal","Radiological Field Meal","hazard meal","food-culture template for radiological expeditions; medical chelator remains separate",EA::Void},
    FoodDefinition{"elysium:food/pressure_field_meal","Pressure Field Meal","hazard meal","food-culture template for deep-pressure expeditions; medical tonic remains separate",EA::Dimensional},
};

constexpr std::array kLivestock{
    LivestockDefinition{"elysium:livestock/grazer","Grazer","milk analogue, meat, hide, manure","pasture pressure, escape and disease"},
    LivestockDefinition{"elysium:livestock/fowl","Fowl / Small Layer","eggs, meat, down, pest control","population explosion"},
    LivestockDefinition{"elysium:livestock/fiber_beast","Fiber Beast","fleece/fiber, meat and cold utility","shearing labor and feed"},
    LivestockDefinition{"elysium:livestock/swine_like","Swine-like Omnivore","high meat/fat and waste conversion","aggression and feed competition"},
    LivestockDefinition{"elysium:livestock/pack_beast","Pack Beast","hauling, riding and caravan support","training and path width"},
    LivestockDefinition{"elysium:livestock/pollinator","Pollinator","greenhouse/crop bonus and products","toxin sensitivity"},
    LivestockDefinition{"elysium:livestock/guard_animal","Guard Animal","security and tracking","training, access and friendly fire"},
    LivestockDefinition{"elysium:livestock/research_specimen","Research Specimen","xenobiology, medicine and breeding","containment, ethics and faction reactions"},
};

constexpr std::array kFaunaBodyPlans{
    FaunaBodyPlanDefinition{"elysium:fauna_body/quadruped","Quadruped",4,"grazers and predators",true},
    FaunaBodyPlanDefinition{"elysium:fauna_body/biped","Biped",2,"fast alert runners",true},
    FaunaBodyPlanDefinition{"elysium:fauna_body/hexapod","Hexapod",6,"low stable high-gravity form",true},
    FaunaBodyPlanDefinition{"elysium:fauna_body/serpentine","Serpentine",0,"burrowers and swimmers",true},
    FaunaBodyPlanDefinition{"elysium:fauna_body/flyer","Flyer",2,"airborne scan target",false},
    FaunaBodyPlanDefinition{"elysium:fauna_body/floater","Floater",0,"buoyant passive form",false},
};

constexpr std::array kFaunaTraits{
    FaunaTraitOptionDefinition{"elysium:fauna_trait/temperament/passive","temperament","Passive"},
    FaunaTraitOptionDefinition{"elysium:fauna_trait/temperament/skittish","temperament","Skittish"},
    FaunaTraitOptionDefinition{"elysium:fauna_trait/temperament/territorial","temperament","Territorial"},
    FaunaTraitOptionDefinition{"elysium:fauna_trait/temperament/aggressive","temperament","Aggressive"},
    FaunaTraitOptionDefinition{"elysium:fauna_trait/temperament/predatory","temperament","Predatory"},
    FaunaTraitOptionDefinition{"elysium:fauna_trait/diet/grazer","diet","Grazer"},
    FaunaTraitOptionDefinition{"elysium:fauna_trait/diet/scavenger","diet","Scavenger"},
    FaunaTraitOptionDefinition{"elysium:fauna_trait/diet/predator","diet","Predator"},
    FaunaTraitOptionDefinition{"elysium:fauna_trait/activity/diurnal","activity","Diurnal"},
    FaunaTraitOptionDefinition{"elysium:fauna_trait/activity/nocturnal","activity","Nocturnal"},
    FaunaTraitOptionDefinition{"elysium:fauna_trait/activity/always","activity","Always"},
};

constexpr std::array kFieldSupplies{
    FieldSupplyDefinition{"elysium:supply/torch","Torch","light; no suit energy"},
    FieldSupplyDefinition{"elysium:supply/oxygen_canister","Oxygen Canister","restore oxygen"},
    FieldSupplyDefinition{"elysium:supply/shield_cell","Shield Cell","restore hazard shielding"},
    FieldSupplyDefinition{"elysium:supply/fuel_cell","Fuel Cell","restore tool/suit energy"},
    FieldSupplyDefinition{"elysium:supply/bandage","Bandage","basic health recovery"},
    FieldSupplyDefinition{"elysium:supply/ration","Ration","hunger and saturation"},
    FieldSupplyDefinition{"elysium:supply/thermal_stew","Thermal Stew","temporary Thermal resistance"},
    FieldSupplyDefinition{"elysium:supply/cryo_broth","Cryo Broth","temporary Cryogenic resistance"},
    FieldSupplyDefinition{"elysium:supply/antitoxin_meal","Antitoxin Meal","temporary Corrosive resistance"},
    FieldSupplyDefinition{"elysium:supply/rad_chelator","Rad Chelator","temporary Radiological resistance"},
    FieldSupplyDefinition{"elysium:supply/pressure_tonic","Pressure Tonic","temporary Pressure resistance"},
    FieldSupplyDefinition{"elysium:supply/repair_kit","Repair Kit","field tool/ship repair"},
    FieldSupplyDefinition{"elysium:supply/seal_patch","Seal Patch","temporary atmosphere breach repair"},
    FieldSupplyDefinition{"elysium:supply/flare","Flare","visual marker and creature interaction"},
    FieldSupplyDefinition{"elysium:supply/scanner_beacon","Scanner Beacon","temporary local survey marker"},
};

constexpr std::array kShipModules{
    ShipModuleDefinition{"elysium:ship_module/warp_drive_i","Warp Drive I","propulsion","entry interstellar range"},
    ShipModuleDefinition{"elysium:ship_module/warp_drive_ii","Warp Drive II","propulsion","improved range/efficiency"},
    ShipModuleDefinition{"elysium:ship_module/warp_drive_iii","Warp Drive III","propulsion","advanced route access"},
    ShipModuleDefinition{"elysium:ship_module/warp_drive_iv","Warp Drive IV","propulsion","endgame route access"},
    ShipModuleDefinition{"elysium:ship_module/pulse_engine","Pulse Engine","propulsion","in-system transit"},
    ShipModuleDefinition{"elysium:ship_module/atmospheric_thrusters","Atmospheric Thrusters","propulsion","surface/orbit transition"},
    ShipModuleDefinition{"elysium:ship_module/vector_jets","Vector Jets","propulsion","maneuvering and landing control"},
    ShipModuleDefinition{"elysium:ship_module/fuel_tank","Fuel Tank","logistics","increased fuel capacity"},
    ShipModuleDefinition{"elysium:ship_module/cargo_rack","Cargo Rack","logistics","increased cargo capacity"},
    ShipModuleDefinition{"elysium:ship_module/survey_scanner","Survey Scanner","exploration","orbital survey persistence"},
    ShipModuleDefinition{"elysium:ship_module/deep_scanner","Deep Scanner","exploration","deep resource/anomaly interpretation"},
    ShipModuleDefinition{"elysium:ship_module/atmosphere_sampler","Atmosphere Sampler","exploration","hazard/atmosphere analysis"},
    ShipModuleDefinition{"elysium:ship_module/hull_plating","Hull Plating","defense","hull protection"},
    ShipModuleDefinition{"elysium:ship_module/thermal_shield","Thermal Shield","defense","thermal protection"},
    ShipModuleDefinition{"elysium:ship_module/radiation_baffle","Radiation Baffle","defense","radiological protection"},
    ShipModuleDefinition{"elysium:ship_module/autopilot_computer","Autopilot Computer","navigation","route execution"},
    ShipModuleDefinition{"elysium:ship_module/navigation_core","Navigation Core","navigation","route planning and range calculations"},
    ShipModuleDefinition{"elysium:ship_module/emergency_beacon","Emergency Beacon","safety","distress and recovery signaling"},
    ShipModuleDefinition{"elysium:ship_module/docking_collar","Docking Collar","logistics","station and depot interface"},
    ShipModuleDefinition{"elysium:ship_module/drone_bay","Drone Bay","utility","ship-supported drones"},
    ShipModuleDefinition{"elysium:ship_module/habitation_pod","Habitation Pod","habitat","crew life support"},
    ShipModuleDefinition{"elysium:ship_module/small_refinery","Small Refinery","industry","limited shipboard processing"},
    ShipModuleDefinition{"elysium:ship_module/smuggler_hold","Smuggler Hold","utility","concealed/unregistered cargo"},
};

constexpr std::array kVehicles{
    VehicleDefinition{"elysium:vehicle/scout_rover","Scout Rover","fast surface survey","low cargo and limited severe-terrain capability"},
    VehicleDefinition{"elysium:vehicle/cargo_crawler","Cargo Crawler","bulk surface logistics","slow, wide and terrain-sensitive"},
    VehicleDefinition{"elysium:vehicle/mining_rig","Mining Rig","mobile extraction support","industrial visibility and energy demand"},
    VehicleDefinition{"elysium:vehicle/amphibious_skiff","Amphibious Skiff","Oceanic/coast travel","weather and pressure limits"},
    VehicleDefinition{"elysium:vehicle/hover_sled","Hover Sled","late-game rough-terrain transport","energy intensive"},
    VehicleDefinition{"elysium:vehicle/siege_hauler","Siege Hauler","base defense and heavy logistics","large signature and high cost"},
};

constexpr std::array kFactions{
    FactionDefinition{"elysium:faction/neutral","Neutral"}, FactionDefinition{"elysium:faction/empire","Empire"}, FactionDefinition{"elysium:faction/unsworn","Unsworn"},
};

constexpr std::array kEnemyRoles{
    EnemyRoleDefinition{"elysium:enemy_role/scavenger","Scavenger","elysium:faction/unsworn","fast frail grunt; swarm/opportunism"},
    EnemyRoleDefinition{"elysium:enemy_role/reaver","Reaver","elysium:faction/unsworn","slow armored bruiser; corner pressure"},
    EnemyRoleDefinition{"elysium:enemy_role/whisper","Whisper","elysium:faction/unsworn","mobile high-damage ambusher"},
    EnemyRoleDefinition{"elysium:enemy_role/drone","Drone","elysium:faction/empire","hovering expendable enforcement"},
    EnemyRoleDefinition{"elysium:enemy_role/lictor","Lictor","elysium:faction/empire","armored elite; difficult to dislodge"},
    EnemyRoleDefinition{"elysium:enemy_role/adept","Adept","elysium:faction/empire","support; buffs, repairs and enables allies"},
    EnemyRoleDefinition{"elysium:enemy_role/praetor","Praetor","elysium:faction/empire","Register Action final-wave command threat"},
};

constexpr std::array kBosses{
    BossDefinition{"elysium:boss/choir_of_the_uncounted","Choir of the Uncounted","elysium:faction/unsworn",120,9,
        "three phases; summons on phase change; increasing self-speed; crowd trickle below phase cap; immovable and fire-immune"},
    BossDefinition{"elysium:boss/praetor_of_the_sanctioned_answer","Praetor of the Sanctioned Answer","elysium:faction/empire",150,12,
        "phase one regenerating shield worth 25% max HP; phase two shield removed, damage increased and 35% reflection; immovable and fire-immune"},
};

#define EV(fam,slug,label,fac,h,d,s,ability) EnemyVariantDefinition{"elysium:enemy_variant/" fam "/" slug,fam,label,"elysium:faction/" fac,h,d,s,ability}
constexpr std::array kEnemyVariants{
    EV("scavenger","ragpicker","Ragpicker","unsworn",1.00f,1.00f,1.00f,"baseline"),
    EV("scavenger","feral","Feral","unsworn",0.75f,1.45f,0.80f,"Cornered: damage rises sharply as health falls"),
    EV("scavenger","carrion","Carrion","unsworn",1.35f,0.85f,0.80f,"Knitting: regeneration while hurt"),
    EV("scavenger","scuttler","Scuttler","unsworn",0.70f,0.95f,1.35f,"Swift"),
    EV("scavenger","blightfed","Blightfed","unsworn",0.95f,1.00f,1.05f,"Venomous"),
    EV("reaver","chainbound","Chainbound","unsworn",1.00f,1.00f,1.00f,"Unshaken / immovable"),
    EV("reaver","slagfist","Slagfist","unsworn",1.30f,1.00f,0.70f,"Hardened: physical resistance and elemental vulnerability"),
    EV("reaver","hollowed","Hollowed","unsworn",0.85f,1.40f,0.75f,"Cornered"),
    EV("reaver","yokebreaker","Yokebreaker","unsworn",1.20f,0.95f,0.85f,"Deadfall explosion on death"),
    EV("reaver","grindmaw","Grindmaw","unsworn",0.95f,1.20f,0.85f,"Sundering: armor stripping"),
    EV("whisper","ashling","Ashling","unsworn",1.00f,1.00f,1.00f,"Swift"),
    EV("whisper","nightcut","Nightcut","unsworn",0.75f,1.45f,0.80f,"Cornered"),
    EV("whisper","veilwalk","Veilwalk","unsworn",0.85f,0.95f,1.20f,"stronger Swift"),
    EV("whisper","gutterghost","Gutterghost","unsworn",0.90f,1.05f,1.05f,"Blinding on killer"),
    EV("whisper","mourner","Mourner","unsworn",1.15f,0.95f,0.90f,"Echo: shares damage to nearby allies"),
    EV("drone","pattern_one","Pattern One","empire",1.00f,1.00f,1.00f,"baseline"),
    EV("drone","interdictor","Interdictor","empire",1.25f,0.90f,0.85f,"Bulwark: regenerating shield"),
    EV("drone","lancer","Lancer","empire",0.80f,1.40f,0.80f,"Sundering"),
    EV("drone","relay","Relay","empire",1.05f,0.85f,1.10f,"Standard: buffs nearby allies"),
    EV("drone","kill_switch","Kill Switch","empire",0.90f,1.05f,1.05f,"stronger Deadfall"),
    EV("lictor","sanctioned","Sanctioned","empire",1.00f,1.00f,1.00f,"Unshaken"),
    EV("lictor","aegis","Aegis","empire",1.30f,0.85f,0.85f,"stronger Bulwark"),
    EV("lictor","censor","Censor","empire",0.90f,1.35f,0.75f,"long Sundering"),
    EV("lictor","custodian","Custodian","empire",1.15f,0.95f,0.90f,"Echo 25%"),
    EV("lictor","inquisitor","Inquisitor","empire",0.95f,1.15f,0.90f,"Overcharged when hurt"),
    EV("adept","acolyte","Acolyte","empire",1.00f,1.00f,1.00f,"Standard support radius"),
    EV("adept","vivifier","Vivifier","empire",1.20f,0.85f,0.95f,"Knitting support"),
    EV("adept","resonant","Resonant","empire",0.85f,1.30f,0.85f,"Overcharged"),
    EV("adept","marshal","Marshal","empire",1.10f,0.90f,1.00f,"stronger Standard"),
    EV("adept","null_speaker","Null Speaker","empire",0.90f,1.05f,1.05f,"inverted Hardened: elemental resistance"),
};
#undef EV

#define TH(axis,slug,label) ThreatOptionDefinition{"elysium:rift_horror/" slug,TA::axis,label}
constexpr std::array kRiftHorrorOptions{
    TH(BodyPlan,"body/quadruped","Quadruped"), TH(BodyPlan,"body/serpentine","Serpentine"), TH(BodyPlan,"body/radial","Radial"),
    TH(BodyPlan,"body/flyer","Flyer"), TH(BodyPlan,"body/burrower","Burrower"), TH(BodyPlan,"body/many_limbed","Many-limbed"), TH(BodyPlan,"body/amorphous","Amorphous"),
    TH(Material,"material/flesh","Flesh"), TH(Material,"material/chitin","Chitin"), TH(Material,"material/crystal","Crystal"), TH(Material,"material/metal","Metal"),
    TH(Material,"material/glass","Glass"), TH(Material,"material/plasma_sheath","Plasma Sheath"), TH(Material,"material/fungal","Fungal"), TH(Material,"material/void_composite","Void Composite"),
    TH(Locomotion,"locomotion/walk","Walk"), TH(Locomotion,"locomotion/climb","Climb"), TH(Locomotion,"locomotion/burrow","Burrow"), TH(Locomotion,"locomotion/fly","Fly"),
    TH(Locomotion,"locomotion/phase_step","Phase-step"), TH(Locomotion,"locomotion/swim","Swim"), TH(Locomotion,"locomotion/wall_crawl","Wall-crawl"),
    TH(Defense,"defense/armor_plates","Armor Plates"), TH(Defense,"defense/regeneration","Regeneration"), TH(Defense,"defense/shield_organ","Shield Organ"),
    TH(Defense,"defense/dispersal","Dispersal"), TH(Defense,"defense/camouflage","Camouflage"),
    TH(Attack,"attack/breach","Breach"), TH(Attack,"attack/grab","Grab"), TH(Attack,"attack/spit","Spit"), TH(Attack,"attack/beam","Beam"), TH(Attack,"attack/cloud","Cloud"),
    TH(Attack,"attack/charge","Charge"), TH(Attack,"attack/summon_spawn","Summon Spawn"), TH(Attack,"attack/drain_power","Drain Power"), TH(Attack,"attack/corrupt_machines","Corrupt Machines"),
    TH(Emission,"emission/acid_mist","Acid Mist"), TH(Emission,"emission/spores","Spores"), TH(Emission,"emission/radiation","Radiation"), TH(Emission,"emission/neural_effect","Neural Effect"),
    TH(Emission,"emission/heat","Heat"), TH(Emission,"emission/cryogenic_wake","Cryogenic Wake"), TH(Emission,"emission/dimensional_distortion","Dimensional Distortion"),
    TH(Vulnerability,"vulnerability/material","Material"), TH(Vulnerability,"vulnerability/environment","Environment"), TH(Vulnerability,"vulnerability/body_region","Body Region"),
    TH(Vulnerability,"vulnerability/exposed_phase","Exposed Phase"), TH(Vulnerability,"vulnerability/sound_signal","Sound / Signal"), TH(Vulnerability,"vulnerability/power_state","Power State"),
};
#undef TH

#define POI(slug,label,loc,purpose) PoiDefinition{"elysium:poi/" slug,label,loc,purpose}
constexpr std::array kPois{
    POI("supply_cache","Supply Cache","surface","consumables, fuel and occasional rune reward"),
    POI("crashed_ship","Crashed Ship","surface","repairable hull, salvage and project hook"),
    POI("survey_outpost","Survey Outpost","surface","map reveal and ore clue"),
    POI("unsworn_camp","Unsworn Camp","surface","trade, directions and faction presence"),
    POI("mining_rig","Mining Rig","surface","rich vein access with Suspicion exposure"),
    POI("ruin","Ruin","surface","Aetherium fragments, history and traps"),
    POI("monolith","Monolith","surface","lore and once-per-planet discovery reward"),
    POI("rift_anomaly","Rift Anomaly","deep/special","dungeon or expedition entry"),
    POI("imperial_waystation","Imperial Waystation","surface","garrison and file infrastructure"),
    POI("abandoned_farm","Abandoned Farm","surface","seeds, food and domestic-fauna clues"),
    POI("weather_station","Weather Station","surface","forecast and hazard data"),
    POI("subsurface_vault","Subsurface Vault","deep","locked industrial cache"),
    POI("buried_reactor","Buried Reactor","deep","radiation, salvage and power-restoration route"),
    POI("research_habitat","Research Habitat","surface","biology logs and specimens"),
    POI("planetary_relay","Planetary Relay","surface","communications and map services"),
    POI("smuggler_den","Smuggler Den","surface","black-market services or ambush"),
    POI("ancient_observatory","Ancient Observatory","surface","charts and anomaly clues"),
    POI("geothermal_plant","Geothermal Plant","surface","power puzzle and thermal materials"),
    POI("flooded_complex","Flooded Complex","submerged","pressure traversal and salvage"),
    POI("orbital_debris_fall","Orbital Debris Fall","surface","ship salvage"),
    POI("trading_post","Trading Post","orbit","system-local commodity economy"),
    POI("imperial_station","Imperial Station","orbit","repair, file access and registered goods"),
    POI("unsworn_haven","Unsworn Haven","orbit/surface","charts and unregistered trade"),
    POI("derelict_freighter","Derelict Freighter","space","boarding encounter"),
    POI("data_buoy","Data Buoy","space","route, lore and scanner information"),
    POI("asteroid_refinery","Asteroid Refinery","space","bulk mining and industrial encounter"),
    POI("silent_colony","Silent Colony","surface","mystery settlement with persistent history"),
    POI("siege_ruin","Siege Ruin","surface","former fortress and defense salvage"),
    POI("anomaly_labyrinth","Anomaly Labyrinth","anomalous","set-piece exploration and rift science"),
    POI("court_anchorage","Court Anchorage","orbit","rare envoy staging and Imperial spectacle"),
};
#undef POI

constexpr std::array kDungeonRooms{
    DungeonRoomDefinition{"elysium:dungeon_room/entrance","Entrance",DR::Entrance,0,"return portal arch"},
    DungeonRoomDefinition{"elysium:dungeon_room/pillar_hall","Pillar Hall",DR::Filler,3,"columns every four blocks"},
    DungeonRoomDefinition{"elysium:dungeon_room/cistern","Cistern",DR::Filler,2,"inset water basin and rubble"},
    DungeonRoomDefinition{"elysium:dungeon_room/collapse","Collapse",DR::Filler,2,"8-16 debris piles"},
    DungeonRoomDefinition{"elysium:dungeon_room/crypt","Crypt",DR::Filler,2,"bone alcoves along both walls"},
    DungeonRoomDefinition{"elysium:dungeon_room/cold_forge","Cold Forge",DR::Filler,1,"magma centre"},
    DungeonRoomDefinition{"elysium:dungeon_room/bare","Bare",DR::Filler,1,"light only"},
    DungeonRoomDefinition{"elysium:dungeon_room/vault","Vault",DR::Loot,1,"raised plinth with 1-2 chests"},
    DungeonRoomDefinition{"elysium:dungeon_room/throne","Throne",DR::Boss,1,"raised dais; boss spawn"},
};

constexpr std::array kSettlementTemplates{
    SettlementTemplateDefinition{"elysium:settlement/camp","Camp","3-8 residents; trade, rumors, directions and simple contracts"},
    SettlementTemplateDefinition{"elysium:settlement/hamlet","Hamlet","8-25 residents; food, repair and specialist services"},
    SettlementTemplateDefinition{"elysium:settlement/frontier_town","Frontier Town","25-80 residents; market, ship pad, faction services and contracts"},
    SettlementTemplateDefinition{"elysium:settlement/imperial_enclave","Imperial Enclave","regulated settlement with Imperial offices and security"},
    SettlementTemplateDefinition{"elysium:settlement/unsworn_haven","Unsworn Haven","independent settlement with salvage, charts and unregistered trade"},
    SettlementTemplateDefinition{"elysium:settlement/player_outpost","Player Outpost","claim-linked dependent site using fortress systems"},
};

constexpr std::array kInstitutionTemplates{
    InstitutionTemplateDefinition{"elysium:institution/cantina","Cantina","food, drink, social activity and visitors"},
    InstitutionTemplateDefinition{"elysium:institution/shrine","Shrine","ritual, philosophy and faith needs"},
    InstitutionTemplateDefinition{"elysium:institution/guildhall","Guildhall","guild membership, teaching and petitions"},
    InstitutionTemplateDefinition{"elysium:institution/archive","Archive","knowledge storage, scholarship and historical records"},
    InstitutionTemplateDefinition{"elysium:institution/hospital","Hospital","diagnosis, treatment, quarantine and recovery"},
    InstitutionTemplateDefinition{"elysium:institution/memorial","Memorial","remembrance, grief processing and Chronicle history"},
    InstitutionTemplateDefinition{"elysium:institution/academy","Academy","teaching, research and skill development"},
    InstitutionTemplateDefinition{"elysium:institution/forum","Forum","public civic activity, administration and debate"},
};

#define CONTRACT(type,scope) ContractDefinition{"elysium:contract/" type "/" scope,type,scope}
constexpr std::array kContracts{
    CONTRACT("survey","local"), CONTRACT("survey","system"), CONTRACT("survey","regional"),
    CONTRACT("procurement","local"), CONTRACT("procurement","system"), CONTRACT("procurement","regional"),
    CONTRACT("construction","local"), CONTRACT("construction","system"), CONTRACT("construction","regional"),
    CONTRACT("recovery","local"), CONTRACT("recovery","system"), CONTRACT("recovery","regional"),
    CONTRACT("escort","local"), CONTRACT("escort","system"), CONTRACT("escort","regional"),
    CONTRACT("bounty","local"), CONTRACT("bounty","system"), CONTRACT("bounty","regional"),
    CONTRACT("infrastructure","local"), CONTRACT("infrastructure","system"), CONTRACT("infrastructure","regional"),
    CONTRACT("smuggling","local"), CONTRACT("smuggling","system"), CONTRACT("smuggling","regional"),
    CONTRACT("research","local"), CONTRACT("research","system"), CONTRACT("research","regional"),
    CONTRACT("defense","local"), CONTRACT("defense","system"), CONTRACT("defense","regional"),
};
#undef CONTRACT

constexpr std::array kCourtEnvoys{
    CourtEnvoyDefinition{"elysium:court/elysomnion","Elysomnion","Emperor of the Black and Emerald","Suspicion","Hunted"},
    CourtEnvoyDefinition{"elysium:court/sylphara_voss","Sylphara Voss","Chief Imperial Architect","Suspicion","Noted+"},
    CourtEnvoyDefinition{"elysium:court/sentinel","Sentinel","Stealth Envoy","Suspicion","always"},
    CourtEnvoyDefinition{"elysium:court/lillith","Lillith","Fleet Commander","Favor","Recognised+"},
    CourtEnvoyDefinition{"elysium:court/aurelia","Aurelia","Queen / former sentient star","Favor","Exalted"},
};

template<class T, std::size_t N>
constexpr std::span<const T> view(const std::array<T,N>& a) { return {a.data(),a.size()}; }

template<class T>
void appendIds(std::vector<std::string_view>& out, std::span<const T> values) {
    for(const auto& value : values) out.push_back(value.id);
}

bool hasCanonicalNamespace(std::string_view id) {
    return id.starts_with("elysium:") && id.find('/') != std::string_view::npos && id.back() != '/';
}

int sumStats(const std::array<int,12>& values) {
    int out=0;
    for(const int v:values) out+=v;
    return out;
}

std::string idError(std::string_view prefix, std::string_view id) {
    return std::string(prefix)+std::string(id);
}

} // namespace

std::span<const MaterialDefinition> materials() { return view(kMaterials); }
std::span<const BlockDefinition> blocks() { return view(kBlocks); }
std::span<const ConstructionPieceDefinition> constructionPieces() { return view(kConstructionPieces); }
std::span<const PlanetClassDefinition> planetClasses() { return view(kPlanetClasses); }
std::span<const BiomeDefinition> biomes() { return view(kBiomes); }
std::span<const WeatherDefinition> weather() { return view(kWeather); }
std::span<const OreDefinition> ores() { return view(kOres); }
std::span<const ItemDefinition> items() { return view(kItems); }
std::span<const MachineDefinition> machines() { return view(kMachines); }
std::span<const RecipeDefinition> recipes() { return view(kRecipes); }
std::span<const RaceDefinition> races() { return view(kRaces); }
std::span<const ClassDefinition> classes() { return view(kClasses); }
std::span<const RuneDefinition> runes() { return view(kRunes); }
std::span<const GearMaterialDefinition> gearMaterials() { return view(kGearMaterials); }
std::span<const TrinketDefinition> trinkets() { return view(kTrinkets); }
std::span<const CropDefinition> crops() { return view(kCrops); }
std::span<const FoodDefinition> foods() { return view(kFoods); }
std::span<const LivestockDefinition> livestock() { return view(kLivestock); }
std::span<const FaunaBodyPlanDefinition> faunaBodyPlans() { return view(kFaunaBodyPlans); }
std::span<const FaunaTraitOptionDefinition> faunaTraitOptions() { return view(kFaunaTraits); }
std::span<const FieldSupplyDefinition> fieldSupplies() { return view(kFieldSupplies); }
std::span<const ShipModuleDefinition> shipModules() { return view(kShipModules); }
std::span<const VehicleDefinition> vehicles() { return view(kVehicles); }
std::span<const FactionDefinition> factions() { return view(kFactions); }
std::span<const EnemyRoleDefinition> enemyRoles() { return view(kEnemyRoles); }
std::span<const BossDefinition> bosses() { return view(kBosses); }
std::span<const EnemyVariantDefinition> enemyVariants() { return view(kEnemyVariants); }
std::span<const ThreatOptionDefinition> riftHorrorOptions() { return view(kRiftHorrorOptions); }
std::span<const PoiDefinition> pois() { return view(kPois); }
std::span<const DungeonRoomDefinition> dungeonRooms() { return view(kDungeonRooms); }
std::span<const SettlementTemplateDefinition> settlementTemplates() { return view(kSettlementTemplates); }
std::span<const InstitutionTemplateDefinition> institutionTemplates() { return view(kInstitutionTemplates); }
std::span<const ContractDefinition> contracts() { return view(kContracts); }
std::span<const CourtEnvoyDefinition> courtEnvoys() { return view(kCourtEnvoys); }

std::vector<std::string_view> sortedContentIds() {
    std::vector<std::string_view> out;
    out.reserve(512);
    appendIds(out,materials()); appendIds(out,blocks()); appendIds(out,constructionPieces());
    appendIds(out,planetClasses()); appendIds(out,biomes()); appendIds(out,weather()); appendIds(out,ores());
    appendIds(out,items()); appendIds(out,machines()); appendIds(out,recipes()); appendIds(out,races());
    appendIds(out,classes()); appendIds(out,runes()); appendIds(out,gearMaterials()); appendIds(out,trinkets());
    appendIds(out,crops()); appendIds(out,foods()); appendIds(out,livestock()); appendIds(out,faunaBodyPlans()); appendIds(out,faunaTraitOptions());
    appendIds(out,fieldSupplies()); appendIds(out,shipModules()); appendIds(out,vehicles()); appendIds(out,factions());
    appendIds(out,enemyRoles()); appendIds(out,bosses()); appendIds(out,enemyVariants()); appendIds(out,riftHorrorOptions()); appendIds(out,pois());
    appendIds(out,dungeonRooms()); appendIds(out,settlementTemplates()); appendIds(out,institutionTemplates());
    appendIds(out,contracts()); appendIds(out,courtEnvoys());
    std::sort(out.begin(),out.end());
    return out;
}

std::uint64_t stableStringHash(std::string_view value) {
    std::uint64_t h=1469598103934665603ULL;
    for(const unsigned char c:value) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 1099511628211ULL;
    }
    return mix64(h ^ static_cast<std::uint64_t>(value.size()));
}

std::uint64_t stableContentSeed(std::uint64_t baseSeed, std::string_view contentId, std::string_view label) {
    const std::uint64_t idHash=stableStringHash(contentId);
    const std::uint64_t labelHash=stableStringHash(label);
    return mix64(baseSeed ^ mix64(idHash) ^ mix64(labelHash ^ 0x434F4E54454E5455ULL));
}

std::uint64_t catalogueFingerprint() {
    const auto ids=sortedContentIds();
    std::uint64_t h=mix64(0x454C595349554D41ULL ^ static_cast<std::uint64_t>(ids.size()));
    for(const auto id:ids) h=mix64(h ^ stableStringHash(id));
    return h;
}

std::vector<std::string> validateContentCatalogue() {
    std::vector<std::string> errors;
    const auto ids=sortedContentIds();
    for(const auto id:ids) {
        if(!hasCanonicalNamespace(id)) errors.push_back(idError("non-namespaced content ID: ",id));
    }
    for(std::size_t i=1;i<ids.size();++i) {
        if(ids[i]==ids[i-1]) errors.push_back(idError("duplicate content ID: ",ids[i]));
    }

    std::set<std::string_view> materialIds, planetIds, itemIds, machineIds, factionIds, gearIds;
    for(const auto& v:materials()) materialIds.insert(v.id);
    for(const auto& v:planetClasses()) planetIds.insert(v.id);
    for(const auto& v:blocks()) itemIds.insert(v.id);
    for(const auto& v:items()) itemIds.insert(v.id);
    for(const auto& v:machines()) machineIds.insert(v.id);
    for(const auto& v:factions()) factionIds.insert(v.id);
    for(const auto& v:gearMaterials()) gearIds.insert(v.id);

    for(const auto& b:blocks()) if(!materialIds.contains(b.materialId)) errors.push_back(idError("block references unknown material: ",b.id));
    for(const auto& b:biomes()) if(!planetIds.contains(b.planetClassId)) errors.push_back(idError("biome references unknown planet class: ",b.id));
    for(const auto& w:weather()) if(!planetIds.contains(w.planetClassId)) errors.push_back(idError("weather references unknown planet class: ",w.id));
    for(const auto& o:ores()) {
        if(!materialIds.contains(o.materialId)) errors.push_back(idError("ore references unknown material: ",o.id));
        if(!planetIds.contains(o.bestPlanetClassId)) errors.push_back(idError("ore references unknown best planet class: ",o.id));
        if(o.spreadMeters<=0 || o.toolTier<1 || o.toolTier>5) errors.push_back(idError("invalid ore spine parameters: ",o.id));
    }
    for(const auto& e:enemyRoles()) if(!factionIds.contains(e.factionId)) errors.push_back(idError("enemy role references unknown faction: ",e.id));
    for(const auto& b:bosses()) if(!factionIds.contains(b.factionId)) errors.push_back(idError("boss references unknown faction: ",b.id));
    for(const auto& e:enemyVariants()) if(!factionIds.contains(e.factionId)) errors.push_back(idError("enemy variant references unknown faction: ",e.id));

    std::set<int> blockNumeric, machineNumeric, itemNumeric, recipeNumeric;
    for(const auto& b:blocks()) if(b.legacyNumericId) {
        if(!blockNumeric.insert(*b.legacyNumericId).second) errors.push_back(idError("duplicate block numeric ID: ",b.id));
    }
    for(const auto& m:machines()) if(m.legacyNumericId) {
        if(!machineNumeric.insert(*m.legacyNumericId).second) errors.push_back(idError("duplicate machine numeric ID: ",m.id));
    }
    for(const auto& i:items()) if(i.legacyNumericId) {
        if(!itemNumeric.insert(*i.legacyNumericId).second) errors.push_back(idError("duplicate item numeric ID: ",i.id));
    }
    for(const auto& r:recipes()) {
        if(!machineIds.contains(r.machineId)) errors.push_back(idError("recipe references unknown machine: ",r.id));
        if(r.inputCount<1 || r.inputCount>3 || r.outputCount<=0) errors.push_back(idError("recipe has invalid arity/count: ",r.id));
        for(int n=0;n<r.inputCount && n<3;++n) {
            if(r.inputs[static_cast<std::size_t>(n)].count<=0 || !itemIds.contains(r.inputs[static_cast<std::size_t>(n)].itemId))
                errors.push_back(idError("recipe has unresolved/invalid input: ",r.id));
        }
        if(!itemIds.contains(r.outputItemId)) errors.push_back(idError("recipe has unresolved output: ",r.id));
        if(r.legacyNumericId && !recipeNumeric.insert(*r.legacyNumericId).second) errors.push_back(idError("duplicate recipe numeric ID: ",r.id));
    }

    if(blockNumeric.size()!=16 || *blockNumeric.begin()!=0 || *blockNumeric.rbegin()!=15) errors.push_back("legacy block IDs must remain exactly 0..15");
    if(machineNumeric.size()!=20 || *machineNumeric.begin()!=0 || *machineNumeric.rbegin()!=19) errors.push_back("legacy machine IDs must remain exactly 0..19");
    if(itemNumeric.size()!=21 || *itemNumeric.begin()!=3000 || *itemNumeric.rbegin()!=3020) errors.push_back("legacy industry item IDs must remain exactly 3000..3020");
    if(recipeNumeric.size()!=27 || *recipeNumeric.begin()!=1 || *recipeNumeric.rbegin()!=27) errors.push_back("legacy recipe IDs must remain exactly 1..27");

    int weight=0, unclaimable=0;
    for(const auto& p:planetClasses()) { weight+=p.generationWeightPercent; if(!p.claimable) ++unclaimable; }
    if(planetClasses().size()!=8 || weight!=100 || unclaimable!=1) errors.push_back("planet classes must be eight entries, sum to 100%, with exactly Anomalous unclaimable");
    const auto anomalous=std::find_if(planetClasses().begin(),planetClasses().end(),[](const auto& p){return p.id=="elysium:planet_class/anomalous";});
    if(anomalous==planetClasses().end() || anomalous->claimable) errors.push_back("Anomalous planet class must exist and remain unclaimable");
    if(ores().size()!=22) errors.push_back("ore spine must contain exactly 22 documented ores");

    if(races().size()!=6) errors.push_back("race catalogue must contain exactly six inherited races");
    for(const auto& r:races()) {
        if(sumStats(r.baseStats)!=44) errors.push_back(idError("race base-stat total is not 44: ",r.id));
        if(sumStats(r.growth)!=3) errors.push_back(idError("race growth total is not 3: ",r.id));
    }
    if(classes().size()!=9) errors.push_back("class catalogue must contain exactly nine inherited classes");
    for(const auto& c:classes()) if(sumStats(c.growth)!=2) errors.push_back(idError("class growth total is not 2: ",c.id));
    if(runes().size()!=9) errors.push_back("rune catalogue must contain exactly nine inherited runes");
    if(gearMaterials().size()!=26) errors.push_back("gear-material catalogue must preserve 26 inherited material identities");
    const auto legacyNetherite=std::find_if(gearMaterials().begin(),gearMaterials().end(),[](const auto& g){return g.id=="elysium:gear_material/netherite_legacy";});
    if(legacyNetherite==gearMaterials().end() || !legacyNetherite->legacyCompatibilityOnly || legacyNetherite->canonicalId!="elysium:gear_material/neutronium" || !gearIds.contains(legacyNetherite->canonicalId))
        errors.push_back("legacy Netherite compatibility identity must resolve explicitly to canonical Neutronium");
    if(trinkets().size()!=40) errors.push_back("trinket catalogue must contain 24 found + 16 crafted entries");
    int found=0, crafted=0; for(const auto& t:trinkets()) (t.kind==TK::Found?found:crafted)++;
    if(found!=24 || crafted!=16) errors.push_back("trinket split must remain 24 found / 16 crafted");

    if(crops().size()!=6) errors.push_back("crop catalogue must contain six inherited archetypes");
    if(foods().size()<10) errors.push_back("food catalogue must cover staples, fermentation and all hazard-meal roles");
    if(livestock().size()!=8) errors.push_back("livestock catalogue must contain eight fortress archetypes");
    if(faunaBodyPlans().size()!=6) errors.push_back("procedural fauna must retain six body plans");
    if(fieldSupplies().size()!=15) errors.push_back("field supply catalogue must contain fifteen documented supplies");
    if(shipModules().size()!=23) errors.push_back("ship-module catalogue must contain twenty-three documented module entries");
    if(vehicles().size()!=6) errors.push_back("vehicle catalogue must contain six documented archetypes");
    if(courtEnvoys().size()!=5) errors.push_back("Court catalogue must contain five authored envoys");
    if(pois().size()!=30) errors.push_back("POI catalogue must contain thirty authored site families");
    if(dungeonRooms().size()!=9) errors.push_back("dungeon-room catalogue must contain nine authored rooms");
    if(contracts().size()!=30) errors.push_back("contract catalogue must cover ten types across local/system/regional scope");
    if(constructionPieces().size()<60) errors.push_back("construction catalogue breadth fell below sixty authored variants");
    if(biomes().size()<70) errors.push_back("biome catalogue breadth fell below seventy definitions");
    if(machines().size()<60) errors.push_back("machine/workshop catalogue breadth fell below sixty definitions");

    if(bosses().size()!=2) errors.push_back("authored boss catalogue must contain exactly two inherited bosses");
    if(enemyVariants().size()!=30) errors.push_back("authored enemy variant catalogue must contain exactly thirty variants");
    std::map<std::string_view,int> baselines;
    for(const auto& v:enemyVariants()) {
        if(std::abs((v.healthMultiplier+v.damageMultiplier+v.speedMultiplier)-3.0f)>0.0001f) errors.push_back(idError("enemy variant budget is not exactly 3.00: ",v.id));
        if(std::abs(v.healthMultiplier-1.0f)<0.0001f && std::abs(v.damageMultiplier-1.0f)<0.0001f && std::abs(v.speedMultiplier-1.0f)<0.0001f) ++baselines[v.family];
    }
    for(const std::string_view family:{"scavenger","reaver","whisper","drone","lictor","adept"}) if(baselines[family]!=1)
        errors.push_back(std::string("enemy family must have exactly one 1/1/1 baseline: ")+std::string(family));

    std::array<int,7> threatAxisCounts{};
    for(const auto& option:riftHorrorOptions()) ++threatAxisCounts[static_cast<std::size_t>(option.axis)];
    for(std::size_t i=0;i<threatAxisCounts.size();++i) if(threatAxisCounts[i]==0) errors.push_back("Rift Horror grammar is missing one or more generation axes");

    const std::array<std::string_view,6> survivalSources{
        "elysium:block/stone","elysium:block/coal_ore","elysium:block/copper_ore","elysium:block/tin_ore","elysium:block/iron_ore","elysium:block/planks"
    };
    const auto closure=obtainabilityClosure(survivalSources);
    const std::set<std::string_view> reachable(closure.begin(),closure.end());
    for(const auto& i:items()) if(i.survivalCritical && !reachable.contains(i.id)) errors.push_back(idError("survival-critical item is unreachable from vertical-slice sources: ",i.id));

    return errors;
}

std::vector<std::string_view> obtainabilityClosure(std::span<const std::string_view> sourceItemIds) {
    std::set<std::string_view> reachable;
    for(const auto id:sourceItemIds) reachable.insert(id);
    bool changed=true;
    while(changed) {
        changed=false;
        for(const auto& recipe:recipes()) {
            bool ready=true;
            for(int n=0;n<recipe.inputCount;++n) if(!reachable.contains(recipe.inputs[static_cast<std::size_t>(n)].itemId)) { ready=false; break; }
            if(ready) changed = reachable.insert(recipe.outputItemId).second || changed;
        }
    }
    return {reachable.begin(),reachable.end()};
}

} // namespace elysium::content
