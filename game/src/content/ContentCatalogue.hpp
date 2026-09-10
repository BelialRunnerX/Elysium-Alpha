// Intended function: imported content implementation for ContentCatalogue; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace elysium::content {

enum class ElementAffinity : std::uint8_t {
    None,
    Void,
    Plasma,
    Neural,
    Dimensional,
    Kinetic,
    Mixed
};

enum class DensityBand : std::uint8_t { VeryLight, Light, Medium, Heavy, Extreme };
enum class StrengthBand : std::uint8_t { Fragile, Soft, Medium, Hard, Extreme };
enum class ThermalBehavior : std::uint8_t { Ordinary, Insulating, Conductive, HeatResistant, Cryogenic, Refractory };
enum class CorrosionBehavior : std::uint8_t { Vulnerable, Ordinary, Resistant, Inert };
enum class ShieldingBand : std::uint8_t { None, Low, Medium, High, Extreme };
enum class ConductivityBand : std::uint8_t { Insulator, Low, Medium, High, Resonant };

enum class MachineDomain : std::uint8_t {
    Survival,
    Metallurgy,
    Fabrication,
    Chemical,
    Power,
    Habitat,
    Logistics,
    Defense,
    Command,
    Vehicles,
    Extraction
};

enum class RecipeStatus : std::uint8_t { Live, Specification, Provisional };
enum class TrinketKind : std::uint8_t { Found, Crafted };
enum class DungeonRoomKind : std::uint8_t { Entrance, Filler, Loot, Boss };
enum class ThreatAxis : std::uint8_t { BodyPlan, Material, Locomotion, Defense, Attack, Emission, Vulnerability };

struct MaterialDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view family;
    DensityBand density;
    StrengthBand strength;
    ThermalBehavior thermal;
    CorrosionBehavior corrosion;
    ShieldingBand radiationShielding;
    ConductivityBand conductivity;
    ElementAffinity element;
    bool sealable;
    bool permeable;
    bool biologicallyCompatible;
    bool microRefinable;
    std::string_view renderProfile;
};

struct BlockDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view family;
    std::string_view materialId;
    std::string_view supportClass;
    bool seal;
    bool microRefinable;
    bool natural;
    std::optional<int> legacyNumericId;
};

struct ConstructionPieceDefinition {
    std::string_view id;
    std::string_view family;
    std::string_view variant;
    bool structural;
    bool sealCapable;
    bool utilityCarrier;
    bool interactiveObject;
};

struct PlanetClassDefinition {
    std::string_view id;
    std::string_view name;
    ElementAffinity element;
    std::string_view primaryHazard;
    int generationWeightPercent;
    bool claimable;
    std::string_view coreIdentity;
};

struct BiomeDefinition {
    std::string_view id;
    std::string_view planetClassId;
    std::string_view name;
};

struct WeatherDefinition {
    std::string_view id;
    std::string_view planetClassId;
    std::string_view name;
    std::string_view operationalEffect;
};

struct OreDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view materialId;
    int peakMeters;
    int spreadMeters;
    int toolTier;
    ElementAffinity element;
    std::string_view bestPlanetClassId;
    std::string_view role;
};

struct ItemDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view family;
    std::optional<int> legacyNumericId;
    bool survivalCritical;
};

struct MachineDefinition {
    std::string_view id;
    std::string_view name;
    MachineDomain domain;
    int tier;
    int powerPriority;
    std::string_view tickPolicy;
    std::string_view environment;
    std::optional<int> legacyNumericId;
};

struct IngredientRef {
    std::string_view itemId;
    int count;
};

struct RecipeDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view machineId;
    std::array<IngredientRef, 3> inputs;
    int inputCount;
    std::string_view outputItemId;
    int outputCount;
    RecipeStatus status;
    std::optional<int> legacyNumericId;
};

struct RaceDefinition {
    std::string_view id;
    std::string_view name;
    std::array<int, 12> baseStats;
    std::array<int, 12> growth;
    std::string_view passiveHook;
};

struct ClassDefinition {
    std::string_view id;
    std::string_view name;
    std::array<int, 12> growth;
    std::string_view passiveHook;
};

struct RuneDefinition {
    std::string_view id;
    std::string_view name;
    ElementAffinity element;
    std::string_view affix;
    std::string_view effect;
};

struct GearMaterialDefinition {
    std::string_view id;
    std::string_view name;
    ElementAffinity element;
    int progressionTier;
    std::string_view miningTier;
    bool legacyCompatibilityOnly;
    std::string_view canonicalId;
};

struct TrinketDefinition {
    std::string_view id;
    std::string_view name;
    TrinketKind kind;
    std::string_view slot;
    int requiredLevel;
    std::string_view rule;
};

struct CropDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view primaryYield;
    std::string_view role;
};

struct FoodDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view category;
    std::string_view primaryRole;
    ElementAffinity hazardAffinity;
};

struct LivestockDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view productsOrServices;
    std::string_view risks;
};

struct FaunaBodyPlanDefinition {
    std::string_view id;
    std::string_view name;
    int limbCount;
    std::string_view role;
    bool domesticable;
};

struct FaunaTraitOptionDefinition {
    std::string_view id;
    std::string_view axis;
    std::string_view name;
};

struct FieldSupplyDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view role;
};

struct ShipModuleDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view domain;
    std::string_view function;
};

struct VehicleDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view role;
    std::string_view constraint;
};

struct FactionDefinition {
    std::string_view id;
    std::string_view name;
};

struct EnemyVariantDefinition {
    std::string_view id;
    std::string_view family;
    std::string_view variant;
    std::string_view factionId;
    float healthMultiplier;
    float damageMultiplier;
    float speedMultiplier;
    std::string_view ability;
};

struct EnemyRoleDefinition {
    std::string_view id;
    std::string_view family;
    std::string_view factionId;
    std::string_view role;
};

struct BossDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view factionId;
    int baseHealth;
    int baseDamage;
    std::string_view phaseContract;
};

struct ThreatOptionDefinition {
    std::string_view id;
    ThreatAxis axis;
    std::string_view name;
};

struct PoiDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view location;
    std::string_view purpose;
};

struct DungeonRoomDefinition {
    std::string_view id;
    std::string_view name;
    DungeonRoomKind kind;
    int weight;
    std::string_view contents;
};

struct SettlementTemplateDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view function;
};

struct InstitutionTemplateDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view primaryService;
};

struct ContractDefinition {
    std::string_view id;
    std::string_view type;
    std::string_view scope;
};

struct CourtEnvoyDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view office;
    std::string_view meter;
    std::string_view gate;
};

std::span<const MaterialDefinition> materials();
std::span<const BlockDefinition> blocks();
std::span<const ConstructionPieceDefinition> constructionPieces();
std::span<const PlanetClassDefinition> planetClasses();
std::span<const BiomeDefinition> biomes();
std::span<const WeatherDefinition> weather();
std::span<const OreDefinition> ores();
std::span<const ItemDefinition> items();
std::span<const MachineDefinition> machines();
std::span<const RecipeDefinition> recipes();
std::span<const RaceDefinition> races();
std::span<const ClassDefinition> classes();
std::span<const RuneDefinition> runes();
std::span<const GearMaterialDefinition> gearMaterials();
std::span<const TrinketDefinition> trinkets();
std::span<const CropDefinition> crops();
std::span<const FoodDefinition> foods();
std::span<const LivestockDefinition> livestock();
std::span<const FaunaBodyPlanDefinition> faunaBodyPlans();
std::span<const FaunaTraitOptionDefinition> faunaTraitOptions();
std::span<const FieldSupplyDefinition> fieldSupplies();
std::span<const ShipModuleDefinition> shipModules();
std::span<const VehicleDefinition> vehicles();
std::span<const FactionDefinition> factions();
std::span<const EnemyRoleDefinition> enemyRoles();
std::span<const BossDefinition> bosses();
std::span<const EnemyVariantDefinition> enemyVariants();
std::span<const ThreatOptionDefinition> riftHorrorOptions();
std::span<const PoiDefinition> pois();
std::span<const DungeonRoomDefinition> dungeonRooms();
std::span<const SettlementTemplateDefinition> settlementTemplates();
std::span<const InstitutionTemplateDefinition> institutionTemplates();
std::span<const ContractDefinition> contracts();
std::span<const CourtEnvoyDefinition> courtEnvoys();

// Canonical deterministic order for generator-facing content. Source/registration
// order is never a generation contract.
std::vector<std::string_view> sortedContentIds();
std::uint64_t stableStringHash(std::string_view value);
std::uint64_t stableContentSeed(std::uint64_t baseSeed, std::string_view contentId, std::string_view label);
std::uint64_t catalogueFingerprint();

// Returns every located validation error. Empty means the immutable catalogue is
// internally coherent. Validation includes namespacing, duplicate IDs,
// references, inherited balance invariants, live numeric compatibility and
// broad content coverage gates.
std::vector<std::string> validateContentCatalogue();

// Fixed-point closure over catalogue recipes. The caller supplies stable item IDs
// available from world generation, starter inventory, drops or explicit grants.
// Quantity is intentionally ignored: this validates reachability, not balance.
std::vector<std::string_view> obtainabilityClosure(std::span<const std::string_view> sourceItemIds);

} // namespace elysium::content
