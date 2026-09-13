// Intended function: imported rules implementation for CharacterRpg; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace elysium::rpg {

// Persistent/content-facing identifiers in this module are namespaced strings.
// They are deliberately not entt::entity values or registry iteration indices.
using ContentId = std::string;

enum class Stat : std::uint8_t {
    Vitality = 0,
    Fortitude,
    Resilience,
    Strength,
    Agility,
    Accuracy,
    Reflexes,
    Retribution,
    Intellect,
    Willpower,
    Luck,
    Presence,
    Count
};

constexpr std::size_t kStatCount = static_cast<std::size_t>(Stat::Count);

struct StatBlock {
    std::array<double, kStatCount> values{};

    double get(Stat stat) const;
    double& get(Stat stat);
    double sum() const;
    void add(const StatBlock& other, double scale = 1.0);
};

struct ProgressionState {
    std::uint64_t level{1};
    std::uint64_t experience{}; // XP bank toward the next level; may retain excess after the per-award safety guard.
    std::uint64_t spendableStatPoints{};
    StatBlock spentStats{};
};

struct ExperienceAwardResult {
    std::uint32_t levelsGained{};
    std::uint64_t statPointsGained{};
    bool safetyGuardHit{};
};

std::uint64_t experienceToNextLevel(std::uint64_t level);
ExperienceAwardResult awardExperience(ProgressionState& state, std::uint64_t amount);
bool spendStatPoints(ProgressionState& state, Stat stat, std::uint64_t points = 1);

double boundedProportion(double value, double halfway, double ceiling);
double combineIndependentShares(double a, double b);

struct DerivedStats {
    double vitalityHealthBonus{};
    double regenerationPerTick{};
    double armour{};
    double meleeDamageBonus{};
    double resilienceReduction{};
    double movementSpeedShare{};
    double criticalChance{};
    double reflexDodgeChance{};
    double retributionShare{};
    double psionicScale{1.0};
    double shieldCapacity{};
    double luckExtraDropChance{};
    double presenceScale{1.0};
};

DerivedStats deriveStats(const StatBlock& stats);

enum class HazardAffinity : std::uint8_t {
    None = 0,
    Thermal,
    Cryogenic,
    Corrosive,
    Radiological,
    Pressure
};

enum class GearSlot : std::uint8_t {
    Weapon = 0,
    Head,
    Chest,
    Legs,
    Feet,
    Ring,
    Back,
    Belt,
    Charm,
    Hands,
    Necklace,
    Undersuit,
    Pack,
    Frame
};

enum class PassiveEvent : std::uint8_t {
    Query = 0,
    ServerTick,
    Kill,
    Damaged
};

struct PassiveContext {
    PassiveEvent event{PassiveEvent::Query};
    std::uint64_t level{1};
    double healthFraction{1.0};
    double targetHealthFraction{1.0};
    double secondsSinceLastHit{1.0e9};
    double fallingDistance{};
    double incomingDamage{};
    double victimMaxHealth{};
    bool hurtRecovery{};
    bool crouching{};
    bool onFire{};
    bool targetIsUnsworn{};
    bool hunted{};
    bool incomingFireOrExplosion{};
};

// One source's answer to the fixed passive vocabulary. Sources return neutral
// values for questions they do not answer. Combining policy is centralized in
// mergePassiveContribution(), so content never needs a race/class/trinket name switch.
struct PassiveContribution {
    double attackScale{1.0};
    double critMultiplier{1.5};
    double defenceScale{1.0};
    double reflectShare{};
    double reflectMultiplier{1.0};
    double dodgeChance{};
    double lifestealShare{};
    double fallDamageScale{1.0};
    double fallDistanceIgnored{};
    double regenScale{1.0};
    double shieldScale{1.0};
    double psionicScale{1.0};
    double xpScale{1.0};
    double favorScale{1.0};
    double suspicionScale{1.0};
    double reforgeScale{1.0};
    double decayRateContribution{};
    double extraDropChance{};

    // The inherited hook names are boolean, while several canonical sources
    // actually specify probabilities. Keeping both forms preserves exact data
    // without forcing mining/durability callers into a central content switch.
    bool savesDurabilityAlways{};
    bool doublesOreAlways{};
    double savesDurabilityChance{};
    double doublesOreChance{};

    // Gear/rune adapter outputs that sit outside the core 21 hook questions.
    double heatReductionAdd{};            // summed then capped by combat adapter at 0.60
    double knockbackResistanceAdd{};
    double attackSpeedScale{1.0};
    double movementScale{1.0};
    double armourToughnessAdd{};
    double attackDamageAdd{};
    double barrierCapacityAdd{};
    double barrierRefreshAdd{};
    double periodicHealAdd{};
    bool resistanceEffect{};
    bool strengthEffect{};
    bool hasteEffect{};
    bool slowFallingEffect{};
    bool jumpBoostEffect{};

    // Event outputs. The owning combat/standing systems decide when/how to
    // commit these effects; this module does not sequence combat.
    bool negateOpeningDamageAtFullHealth{};
    bool blockStandingDecay{};
    double bonusFavorOnKill{};
    double healOnKill{};
    double resistanceSecondsOnDamaged{};
    double nearbyTriageHealScale{};

    // Unsworn's inherited decay exception is Suspicion-specific, unlike the
    // general decayRate hook shared by accessory content.
    double suspicionDecayScale{1.0};
};

struct PassiveSummary : PassiveContribution {
    double cappedHeatReduction() const;
};

using PassiveQuery = std::function<PassiveContribution(const PassiveContext&, int progressionTier, double amplifier)>;

struct ElementDefinition {
    ContentId id;
    std::string displayName;
    std::array<ContentId, 2> beats;
    std::array<Stat, 2> grantedStats;
    HazardAffinity environmentalAffinity{HazardAffinity::None};
};

struct RaceDefinition {
    ContentId id;
    std::string displayName;
    StatBlock baseStats;
    StatBlock growthPerLevel;
    PassiveQuery passive;
};

struct ClassDefinition {
    ContentId id;
    std::string displayName;
    StatBlock growthPerLevel;
    PassiveQuery passive;
};

struct RuneDefinition {
    ContentId id;
    std::string displayName;
    std::optional<ContentId> elementId;
    PassiveQuery passive;
};

struct TrinketDefinition {
    ContentId id;
    std::string displayName;
    GearSlot slot{GearSlot::Charm};
    int requiredLevel{};
    int baseTier{};
    bool found{};
    bool canAscend{};
    PassiveQuery passive;
};

struct GearFamilyDefinition {
    ContentId id;
    std::string displayName;
    std::vector<GearSlot> slots;
    std::vector<std::string> purposes;
};

class RpgCatalog {
public:
    void registerElement(ElementDefinition definition);
    void registerRace(RaceDefinition definition);
    void registerClass(ClassDefinition definition);
    void registerRune(RuneDefinition definition);
    void registerTrinket(TrinketDefinition definition);
    void registerGearFamily(GearFamilyDefinition definition);

    const ElementDefinition& element(std::string_view id) const;
    const RaceDefinition& race(std::string_view id) const;
    const ClassDefinition& characterClass(std::string_view id) const;
    const RuneDefinition& rune(std::string_view id) const;
    const TrinketDefinition& trinket(std::string_view id) const;
    const GearFamilyDefinition& gearFamily(std::string_view id) const;

    const std::vector<ElementDefinition>& elements() const;
    const std::vector<RaceDefinition>& races() const;
    const std::vector<ClassDefinition>& classes() const;
    const std::vector<RuneDefinition>& runes() const;
    const std::vector<TrinketDefinition>& trinkets() const;
    const std::vector<GearFamilyDefinition>& gearFamilies() const;

    bool elementBeats(std::string_view attacker, std::string_view defender) const;
    bool validateClosedElementGraph() const;
    bool validateCanonicalBalance() const;
    void freeze() const;
    bool frozen() const { return frozen_; }

private:
    template <class T>
    static const T& lookup(const std::vector<T>& values,
                           const std::unordered_map<ContentId, std::size_t>& index,
                           std::string_view id,
                           const char* kind);

    template <class T>
    static void insert(std::vector<T>& values,
                       std::unordered_map<ContentId, std::size_t>& index,
                       T definition,
                       const char* kind,
                       bool frozen);

    mutable bool frozen_{};
    std::vector<ElementDefinition> elements_;
    std::vector<RaceDefinition> races_;
    std::vector<ClassDefinition> classes_;
    std::vector<RuneDefinition> runes_;
    std::vector<TrinketDefinition> trinkets_;
    std::vector<GearFamilyDefinition> gearFamilies_;
    std::unordered_map<ContentId, std::size_t> elementIndex_;
    std::unordered_map<ContentId, std::size_t> raceIndex_;
    std::unordered_map<ContentId, std::size_t> classIndex_;
    std::unordered_map<ContentId, std::size_t> runeIndex_;
    std::unordered_map<ContentId, std::size_t> trinketIndex_;
    std::unordered_map<ContentId, std::size_t> gearFamilyIndex_;
};

void registerCanonicalRpgContent(RpgCatalog& catalog);
RpgCatalog makeCanonicalRpgCatalog();

struct ReforgeRolls {
    int armour{};
    int health{};
    int speed{};
    friend bool operator==(const ReforgeRolls&, const ReforgeRolls&) = default;
};

struct GearItemDefinition {
    ContentId id;
    std::string displayName;
    std::optional<ContentId> elementId;
    GearSlot slot{GearSlot::Weapon};
    int baseTier{};
    bool armour{};
    bool canAscend{true};
    double gradeMultiplier{1.0};
    double maximumCondition{100.0};
    std::optional<ContentId> familyId;
};

struct GearItemState {
    ContentId itemId;
    int tier{};
    double condition{100.0};
    double maximumCondition{100.0};
    std::vector<ContentId> socketedRunes;
    ReforgeRolls reforgeRolls{};
    int reforgeCharges{3};
};

struct TrinketInstance {
    ContentId id;
    int ascensions{};
};

struct CharacterBuild {
    ProgressionState progression;
    ContentId raceId;
    ContentId classId;
    std::vector<GearItemState> gear;
    std::vector<GearItemDefinition> gearDefinitions;
    std::vector<TrinketInstance> trinkets;
};

// Canonical accessory loadout: ring x2, and one each of back, belt, charm,
// hands, head and necklace, for eight worn trinkets total. Other gear slots
// are not accessory slots and therefore have zero trinket capacity.
int trinketSlotCapacity(GearSlot slot);

struct LoadoutValidationIssue {
    ContentId contentId;
    std::string reason;
};

struct LoadoutValidationResult {
    bool valid{true};
    std::vector<LoadoutValidationIssue> issues;
};

LoadoutValidationResult validateTrinketLoadout(const RpgCatalog& catalog,
                                               const CharacterBuild& build);

enum class PassiveSourceKind : std::uint8_t {
    Race = 0,
    Class,
    Trinket,
    Rune
};

struct PassiveSourceRecord {
    PassiveSourceKind kind{PassiveSourceKind::Race};
    ContentId sourceId;
    int progressionTier{};
    double amplifier{1.0};
    PassiveContribution contribution;
};

struct PassiveBreakdown {
    PassiveSummary summary;
    std::vector<PassiveSourceRecord> sources;
};

int requiredLevelForTier(int tier);
double tierScale(int tier);
double tierAdded(double base, int tier);
int statWeight(int tier);
double elementalAdvantageForTier(int tier);
int socketCapacityForTier(int tier);
double craftedMultiplier(double base, int ascensions);
double craftedShare(double base, int ascensions);
int reforgeBasePoints(int tier);
int reforgeFinalPoints(int tier, double gradeMultiplier, double reforgeScale);

GearItemState makeGearItem(const GearItemDefinition& definition);
bool socketRune(GearItemState& item, const GearItemDefinition& definition,
                const RpgCatalog& catalog, std::string_view runeId,
                std::string* failureReason = nullptr);
double runeAlignmentMultiplier(const GearItemDefinition& item,
                               const RuneDefinition& rune);

struct ReforgeResult {
    bool success{};
    ReforgeRolls rolls{};
    std::string failureReason;
};

ReforgeResult reforgeGear(GearItemState& item, const GearItemDefinition& definition,
                          double reforgeScale, std::uint64_t deterministicEntropy);

struct AscensionResult {
    bool success{};
    std::string failureReason;
};

AscensionResult ascendGear(GearItemState& target,
                           std::optional<GearItemState>& counterpart,
                           const GearItemDefinition& definition);

const GearItemDefinition& findGearDefinition(const CharacterBuild& build, std::string_view itemId);
StatBlock evaluateCharacterStats(const RpgCatalog& catalog, const CharacterBuild& build);
PassiveSummary evaluatePassives(const RpgCatalog& catalog, const CharacterBuild& build,
                                const PassiveContext& context,
                                const StatBlock* precomputedStats = nullptr);
PassiveBreakdown evaluatePassiveBreakdown(const RpgCatalog& catalog, const CharacterBuild& build,
                                          const PassiveContext& context,
                                          const StatBlock* precomputedStats = nullptr);

bool deterministicChance(std::uint64_t entropy, double probability);

// Stable canonical IDs. Keeping these as data-facing strings lets save/content
// layers serialize IDs without ever serializing runtime entity handles.
namespace ids {
inline constexpr std::string_view elementVoid = "elysium:element/void";
inline constexpr std::string_view elementPlasma = "elysium:element/plasma";
inline constexpr std::string_view elementNeural = "elysium:element/neural";
inline constexpr std::string_view elementDimensional = "elysium:element/dimensional";
inline constexpr std::string_view elementKinetic = "elysium:element/kinetic";

inline constexpr std::string_view raceImperial = "elysium:race/imperial";
inline constexpr std::string_view raceDruun = "elysium:race/druun";
inline constexpr std::string_view raceVeylari = "elysium:race/veylari";
inline constexpr std::string_view raceKorrath = "elysium:race/korrath";
inline constexpr std::string_view raceLumari = "elysium:race/lumari";
inline constexpr std::string_view raceUnsworn = "elysium:race/unsworn";

inline constexpr std::string_view classMedicae = "elysium:class/medicae";
inline constexpr std::string_view classFactor = "elysium:class/factor";
inline constexpr std::string_view classArtificer = "elysium:class/artificer";
inline constexpr std::string_view classEnforcer = "elysium:class/enforcer";
inline constexpr std::string_view classPsion = "elysium:class/psion";
inline constexpr std::string_view classVoidrunner = "elysium:class/voidrunner";
inline constexpr std::string_view classReclaimer = "elysium:class/reclaimer";
inline constexpr std::string_view classWarden = "elysium:class/warden";
inline constexpr std::string_view classMarksman = "elysium:class/marksman";

inline constexpr std::string_view runeVoidward = "elysium:rune/voidward";
inline constexpr std::string_view runePlasmaforge = "elysium:rune/plasmaforge";
inline constexpr std::string_view runeNeuralspike = "elysium:rune/neuralspike";
inline constexpr std::string_view runeDimensionalshift = "elysium:rune/dimensionalshift";
inline constexpr std::string_view runeKineticsurge = "elysium:rune/kineticsurge";
inline constexpr std::string_view runeStabilizer = "elysium:rune/stabilizer";
inline constexpr std::string_view runeReflex = "elysium:rune/reflex";
inline constexpr std::string_view runeBarrier = "elysium:rune/barrier";
inline constexpr std::string_view runePlasmaCore = "elysium:rune/plasma_core";
} // namespace ids

} // namespace elysium::rpg
