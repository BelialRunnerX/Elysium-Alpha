// Intended function: imported rules implementation for CharacterRpg; preserves the agent-authored subsystem contract for later integration/debugging.
#include "rules/CharacterRpg.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace elysium::rpg {
namespace {

constexpr double kShareInputCeiling = 0.999;
constexpr int kExperienceAwardLevelGuard = 1000;

std::size_t statIndex(Stat stat) {
    const auto index = static_cast<std::size_t>(stat);
    if (index >= kStatCount) throw std::out_of_range("invalid RPG stat");
    return index;
}

double finiteNonNegative(double value, double fallback = 0.0) {
    if (!std::isfinite(value)) return fallback;
    return std::max(0.0, value);
}

double clamp01(double value) {
    if (!std::isfinite(value)) return 0.0;
    return std::clamp(value, 0.0, 1.0);
}

bool namespacedId(std::string_view id) {
    return !id.empty() && id.find(':') != std::string_view::npos && id.find('/') != std::string_view::npos;
}

StatBlock stats(std::initializer_list<std::pair<Stat, double>> entries) {
    StatBlock block;
    for (const auto& [stat, value] : entries) block.get(stat) = value;
    return block;
}

std::uint64_t stableTextHash(std::string_view text) {
    std::uint64_t h = 1469598103934665603ULL;
    for (const unsigned char ch : text) {
        h ^= static_cast<std::uint64_t>(ch);
        h *= 1099511628211ULL;
    }
    return elysium::mix64(h);
}

std::uint64_t nextRandom(std::uint64_t& state) {
    state = elysium::mix64(state);
    return state;
}

int rollOneTo(std::uint64_t& state, int upperInclusive) {
    upperInclusive = std::max(1, upperInclusive);
    return 1 + static_cast<int>(nextRandom(state) % static_cast<std::uint64_t>(upperInclusive));
}

void requireValidId(std::string_view id, const char* kind) {
    if (!namespacedId(id)) throw std::invalid_argument(std::string(kind) + " requires a namespaced stable id");
}

void mergePassiveContribution(PassiveSummary& out, const PassiveContribution& in) {
    out.attackScale *= finiteNonNegative(in.attackScale, 1.0);
    out.critMultiplier = std::max(out.critMultiplier, finiteNonNegative(in.critMultiplier, 1.5));
    out.defenceScale *= finiteNonNegative(in.defenceScale, 1.0);
    out.reflectShare = combineIndependentShares(out.reflectShare, in.reflectShare);
    out.reflectMultiplier *= finiteNonNegative(in.reflectMultiplier, 1.0);
    out.dodgeChance = combineIndependentShares(out.dodgeChance, in.dodgeChance);
    out.lifestealShare = combineIndependentShares(out.lifestealShare, in.lifestealShare);
    out.fallDamageScale *= finiteNonNegative(in.fallDamageScale, 1.0);
    out.fallDistanceIgnored = std::max(out.fallDistanceIgnored, finiteNonNegative(in.fallDistanceIgnored));
    out.regenScale *= finiteNonNegative(in.regenScale, 1.0);
    out.shieldScale *= finiteNonNegative(in.shieldScale, 1.0);
    out.psionicScale *= finiteNonNegative(in.psionicScale, 1.0);
    out.xpScale *= finiteNonNegative(in.xpScale, 1.0);
    out.favorScale *= finiteNonNegative(in.favorScale, 1.0);
    out.suspicionScale *= finiteNonNegative(in.suspicionScale, 1.0);
    out.reforgeScale *= finiteNonNegative(in.reforgeScale, 1.0);
    out.decayRateContribution += finiteNonNegative(in.decayRateContribution);
    out.extraDropChance = combineIndependentShares(out.extraDropChance, in.extraDropChance);

    out.savesDurabilityAlways = out.savesDurabilityAlways || in.savesDurabilityAlways;
    out.doublesOreAlways = out.doublesOreAlways || in.doublesOreAlways;
    out.savesDurabilityChance = combineIndependentShares(out.savesDurabilityChance, in.savesDurabilityChance);
    out.doublesOreChance = combineIndependentShares(out.doublesOreChance, in.doublesOreChance);

    out.heatReductionAdd += finiteNonNegative(in.heatReductionAdd);
    out.knockbackResistanceAdd += finiteNonNegative(in.knockbackResistanceAdd);
    out.attackSpeedScale *= finiteNonNegative(in.attackSpeedScale, 1.0);
    out.movementScale *= finiteNonNegative(in.movementScale, 1.0);
    out.armourToughnessAdd += finiteNonNegative(in.armourToughnessAdd);
    out.attackDamageAdd += finiteNonNegative(in.attackDamageAdd);
    out.barrierCapacityAdd += finiteNonNegative(in.barrierCapacityAdd);
    out.barrierRefreshAdd += finiteNonNegative(in.barrierRefreshAdd);
    out.periodicHealAdd += finiteNonNegative(in.periodicHealAdd);
    out.resistanceEffect = out.resistanceEffect || in.resistanceEffect;
    out.strengthEffect = out.strengthEffect || in.strengthEffect;
    out.hasteEffect = out.hasteEffect || in.hasteEffect;
    out.slowFallingEffect = out.slowFallingEffect || in.slowFallingEffect;
    out.jumpBoostEffect = out.jumpBoostEffect || in.jumpBoostEffect;

    out.negateOpeningDamageAtFullHealth = out.negateOpeningDamageAtFullHealth || in.negateOpeningDamageAtFullHealth;
    out.blockStandingDecay = out.blockStandingDecay || in.blockStandingDecay;
    out.bonusFavorOnKill += in.bonusFavorOnKill;
    out.healOnKill += in.healOnKill;
    out.resistanceSecondsOnDamaged = std::max(out.resistanceSecondsOnDamaged, finiteNonNegative(in.resistanceSecondsOnDamaged));
    out.nearbyTriageHealScale += finiteNonNegative(in.nearbyTriageHealScale);
    out.suspicionDecayScale *= finiteNonNegative(in.suspicionDecayScale, 1.0);
}

void addRace(RpgCatalog& catalog, ContentId id, std::string displayName,
             const StatBlock& base, const StatBlock& growth, PassiveQuery passive) {
    catalog.registerRace({std::move(id), std::move(displayName), base, growth, std::move(passive)});
}

void addClass(RpgCatalog& catalog, ContentId id, std::string displayName,
              const StatBlock& growth, PassiveQuery passive) {
    catalog.registerClass({std::move(id), std::move(displayName), growth, std::move(passive)});
}

void addRune(RpgCatalog& catalog, ContentId id, std::string displayName,
             std::optional<ContentId> element, PassiveQuery passive) {
    catalog.registerRune({std::move(id), std::move(displayName), std::move(element), std::move(passive)});
}

void addTrinket(RpgCatalog& catalog, ContentId id, std::string displayName,
                GearSlot slot, int level, bool found, PassiveQuery passive) {
    catalog.registerTrinket({std::move(id), std::move(displayName), slot, level,
                             found ? 0 : 2, found, !found, std::move(passive)});
}

ContentId tid(std::string_view slug) {
    return "elysium:trinket/" + std::string(slug);
}

ContentId gid(std::string_view slug) {
    return "elysium:gear_family/" + std::string(slug);
}

} // namespace

double StatBlock::get(Stat stat) const { return values[statIndex(stat)]; }
double& StatBlock::get(Stat stat) { return values[statIndex(stat)]; }

double StatBlock::sum() const {
    return std::accumulate(values.begin(), values.end(), 0.0);
}

void StatBlock::add(const StatBlock& other, double scale) {
    if (!std::isfinite(scale)) throw std::invalid_argument("stat scale must be finite");
    for (std::size_t i = 0; i < values.size(); ++i) values[i] += other.values[i] * scale;
}

std::uint64_t experienceToNextLevel(std::uint64_t level) {
    if (level < 1) level = 1;
    constexpr auto max = std::numeric_limits<std::uint64_t>::max();
    if (level > (max - 60ULL) / 40ULL) return max;
    return 60ULL + 40ULL * level;
}

ExperienceAwardResult awardExperience(ProgressionState& state, std::uint64_t amount) {
    if (state.level < 1) state.level = 1;
    const auto max = std::numeric_limits<std::uint64_t>::max();
    state.experience = (amount > max - state.experience) ? max : state.experience + amount;

    ExperienceAwardResult result;
    while (result.levelsGained < kExperienceAwardLevelGuard) {
        const auto needed = experienceToNextLevel(state.level);
        if (state.experience < needed) break;
        state.experience -= needed;
        if (state.level < max) ++state.level;
        state.spendableStatPoints = state.spendableStatPoints > max - 2ULL ? max : state.spendableStatPoints + 2ULL;
        result.statPointsGained = result.statPointsGained > max - 2ULL ? max : result.statPointsGained + 2ULL;
        ++result.levelsGained;
        if (state.level == max) break;
    }
    result.safetyGuardHit = result.levelsGained == kExperienceAwardLevelGuard &&
                            state.experience >= experienceToNextLevel(state.level);
    return result;
}

bool spendStatPoints(ProgressionState& state, Stat stat, std::uint64_t points) {
    if (points == 0) return true;
    if (state.spendableStatPoints < points) return false;
    state.spendableStatPoints -= points;
    state.spentStats.get(stat) += static_cast<double>(points);
    return true;
}

double boundedProportion(double value, double halfway, double ceiling) {
    value = finiteNonNegative(value);
    halfway = std::max(1.0e-12, finiteNonNegative(halfway, 1.0));
    ceiling = finiteNonNegative(ceiling);
    if (value == 0.0 || ceiling == 0.0) return 0.0;
    const long double v = value;
    const long double k = halfway;
    const long double c = ceiling;
    const long double result = (v / (v + k)) * c;
    if (!std::isfinite(static_cast<double>(result))) return ceiling;
    return std::min(ceiling, static_cast<double>(result));
}

double combineIndependentShares(double a, double b) {
    a = std::clamp(finiteNonNegative(a), 0.0, kShareInputCeiling);
    b = std::clamp(finiteNonNegative(b), 0.0, kShareInputCeiling);
    return 1.0 - (1.0 - a) * (1.0 - b);
}

DerivedStats deriveStats(const StatBlock& s) {
    DerivedStats d;
    d.vitalityHealthBonus = 0.2 * finiteNonNegative(s.get(Stat::Vitality));
    d.regenerationPerTick = 0.25 + 0.05 * finiteNonNegative(s.get(Stat::Vitality));
    d.armour = 0.5 * finiteNonNegative(s.get(Stat::Fortitude));
    d.meleeDamageBonus = 0.25 * finiteNonNegative(s.get(Stat::Strength));
    d.resilienceReduction = boundedProportion(s.get(Stat::Resilience), 120.0, 1.0);
    d.movementSpeedShare = boundedProportion(s.get(Stat::Agility), 200.0, 0.60);
    d.criticalChance = boundedProportion(s.get(Stat::Accuracy), 220.0, 0.75);
    d.reflexDodgeChance = boundedProportion(s.get(Stat::Reflexes), 380.0, 0.50);
    d.retributionShare = boundedProportion(s.get(Stat::Retribution), 260.0, 0.80);
    d.psionicScale = 1.0 + 0.02 * finiteNonNegative(s.get(Stat::Intellect));
    d.shieldCapacity = 0.4 * finiteNonNegative(s.get(Stat::Willpower));
    d.luckExtraDropChance = boundedProportion(s.get(Stat::Luck), 160.0, 0.90);
    d.presenceScale = 1.0 + 0.02 * finiteNonNegative(s.get(Stat::Presence));
    return d;
}

double PassiveSummary::cappedHeatReduction() const {
    return std::min(0.60, finiteNonNegative(heatReductionAdd));
}

template <class T>
const T& RpgCatalog::lookup(const std::vector<T>& values,
                            const std::unordered_map<ContentId, std::size_t>& index,
                            std::string_view id, const char* kind) {
    const auto it = index.find(std::string(id));
    if (it == index.end() || it->second >= values.size()) throw std::out_of_range(std::string("unknown ") + kind + ": " + std::string(id));
    return values[it->second];
}

template <class T>
void RpgCatalog::insert(std::vector<T>& values,
                        std::unordered_map<ContentId, std::size_t>& index,
                        T definition, const char* kind, bool frozen) {
    if (frozen) throw std::logic_error(std::string("late ") + kind + " registration after RPG catalog freeze");
    requireValidId(definition.id, kind);
    if (index.contains(definition.id)) throw std::invalid_argument(std::string("duplicate ") + kind + " id: " + definition.id);
    const auto position = values.size();
    index.emplace(definition.id, position);
    values.push_back(std::move(definition));
}

void RpgCatalog::registerElement(ElementDefinition d) { insert(elements_, elementIndex_, std::move(d), "element", frozen_); }
void RpgCatalog::registerRace(RaceDefinition d) { insert(races_, raceIndex_, std::move(d), "race", frozen_); }
void RpgCatalog::registerClass(ClassDefinition d) { insert(classes_, classIndex_, std::move(d), "class", frozen_); }
void RpgCatalog::registerRune(RuneDefinition d) { insert(runes_, runeIndex_, std::move(d), "rune", frozen_); }
void RpgCatalog::registerTrinket(TrinketDefinition d) { insert(trinkets_, trinketIndex_, std::move(d), "trinket", frozen_); }
void RpgCatalog::registerGearFamily(GearFamilyDefinition d) { insert(gearFamilies_, gearFamilyIndex_, std::move(d), "gear family", frozen_); }

const ElementDefinition& RpgCatalog::element(std::string_view id) const { freeze(); return lookup(elements_, elementIndex_, id, "element"); }
const RaceDefinition& RpgCatalog::race(std::string_view id) const { freeze(); return lookup(races_, raceIndex_, id, "race"); }
const ClassDefinition& RpgCatalog::characterClass(std::string_view id) const { freeze(); return lookup(classes_, classIndex_, id, "class"); }
const RuneDefinition& RpgCatalog::rune(std::string_view id) const { freeze(); return lookup(runes_, runeIndex_, id, "rune"); }
const TrinketDefinition& RpgCatalog::trinket(std::string_view id) const { freeze(); return lookup(trinkets_, trinketIndex_, id, "trinket"); }
const GearFamilyDefinition& RpgCatalog::gearFamily(std::string_view id) const { freeze(); return lookup(gearFamilies_, gearFamilyIndex_, id, "gear family"); }

const std::vector<ElementDefinition>& RpgCatalog::elements() const { freeze(); return elements_; }
const std::vector<RaceDefinition>& RpgCatalog::races() const { freeze(); return races_; }
const std::vector<ClassDefinition>& RpgCatalog::classes() const { freeze(); return classes_; }
const std::vector<RuneDefinition>& RpgCatalog::runes() const { freeze(); return runes_; }
const std::vector<TrinketDefinition>& RpgCatalog::trinkets() const { freeze(); return trinkets_; }
const std::vector<GearFamilyDefinition>& RpgCatalog::gearFamilies() const { freeze(); return gearFamilies_; }

void RpgCatalog::freeze() const { frozen_ = true; }

bool RpgCatalog::elementBeats(std::string_view attacker, std::string_view defender) const {
    const auto& e = element(attacker);
    return e.beats[0] == defender || e.beats[1] == defender;
}

bool RpgCatalog::validateClosedElementGraph() const {
    freeze();
    if (elements_.empty()) return false;
    for (const auto& elementDef : elements_) {
        if (elementDef.beats[0] == elementDef.id || elementDef.beats[1] == elementDef.id || elementDef.beats[0] == elementDef.beats[1]) return false;
        if (!elementIndex_.contains(elementDef.beats[0]) || !elementIndex_.contains(elementDef.beats[1])) return false;
    }
    for (const auto& candidate : elements_) {
        int incoming = 0;
        for (const auto& source : elements_) {
            if (source.beats[0] == candidate.id || source.beats[1] == candidate.id) ++incoming;
        }
        if (incoming != 2) return false;
    }
    return true;
}

bool RpgCatalog::validateCanonicalBalance() const {
    freeze();
    if (races_.size() != 6 || classes_.size() != 9) return false;
    for (const auto& raceDef : races_) {
        if (std::abs(raceDef.baseStats.sum() - 44.0) > 1.0e-9) return false;
        if (std::abs(raceDef.growthPerLevel.sum() - 3.0) > 1.0e-9) return false;
    }
    for (const auto& classDef : classes_) {
        if (std::abs(classDef.growthPerLevel.sum() - 2.0) > 1.0e-9) return false;
    }
    return true;
}

int requiredLevelForTier(int tier) {
    if (tier <= 0) return 0;
    if (tier > std::numeric_limits<int>::max() / 5) return std::numeric_limits<int>::max();
    return tier * 5;
}

double tierScale(int tier) {
    if (tier <= 0) return 1.0;
    constexpr double cap = 1.0e6;
    const double exponent = static_cast<double>(tier) * std::log(1.25);
    if (exponent >= std::log(cap)) return cap;
    const double value = std::exp(exponent);
    return std::min(cap, std::isfinite(value) ? value : cap);
}

double tierAdded(double base, int tier) {
    if (!std::isfinite(base)) throw std::invalid_argument("tierAdded base must be finite");
    return base * (tierScale(tier) - 1.0);
}

int statWeight(int tier) {
    const double raw = std::round(4.0 * tierScale(tier) - 3.0);
    if (raw >= static_cast<double>(std::numeric_limits<int>::max())) return std::numeric_limits<int>::max();
    return std::max(1, static_cast<int>(raw));
}

double elementalAdvantageForTier(int tier) {
    if (tier <= 0) return 0.05;
    constexpr std::array<double, 6> values{0.05, 0.10, 0.15, 0.20, 0.30, 0.40};
    if (tier <= 5) return values[static_cast<std::size_t>(tier)];
    const long double b = static_cast<long double>(tier - 5);
    const long double result = 0.40L + 0.60L * b / (b + 12.0L);
    return std::min(0.999999999, static_cast<double>(result));
}

int socketCapacityForTier(int tier) {
    if (tier < 0) tier = 0;
    if (tier > (std::numeric_limits<int>::max() - 1) / 2) return std::numeric_limits<int>::max();
    return 1 + tier / 2;
}

double craftedMultiplier(double base, int ascensions) {
    if (!std::isfinite(base)) throw std::invalid_argument("crafted multiplier base must be finite");
    return 1.0 + (base - 1.0) * tierScale(std::max(0, ascensions));
}

double craftedShare(double base, int ascensions) {
    base = std::clamp(finiteNonNegative(base), 0.0, 0.95);
    const double exponent = tierScale(std::max(0, ascensions));
    const double value = 1.0 - std::pow(1.0 - base, exponent);
    return std::min(0.95, std::max(0.0, value));
}

int reforgeBasePoints(int tier) {
    constexpr std::array<int, 6> base{3, 5, 8, 12, 18, 25};
    if (tier <= 0) return base[0];
    if (tier <= 5) return base[static_cast<std::size_t>(tier)];
    const double exponent = static_cast<double>(tier - 5) * std::log(1.4);
    if (exponent >= std::log(4000.0)) return 100000;
    return std::min(100000, static_cast<int>(std::round(25.0 * std::exp(exponent))));
}

int reforgeFinalPoints(int tier, double gradeMultiplier, double reforgeScaleValue) {
    gradeMultiplier = finiteNonNegative(gradeMultiplier, 1.0);
    reforgeScaleValue = finiteNonNegative(reforgeScaleValue, 1.0);
    const long double tierMultiplier = std::max(1, tier + 1);
    const long double value = static_cast<long double>(reforgeBasePoints(tier)) * tierMultiplier * gradeMultiplier * reforgeScaleValue;
    if (!std::isfinite(static_cast<double>(value)) || value >= static_cast<long double>(std::numeric_limits<int>::max())) return std::numeric_limits<int>::max();
    return std::max(2, static_cast<int>(std::llround(value)));
}

GearItemState makeGearItem(const GearItemDefinition& definition) {
    requireValidId(definition.id, "gear item");
    GearItemState state;
    state.itemId = definition.id;
    state.tier = std::max(0, definition.baseTier);
    state.maximumCondition = std::max(0.0, definition.maximumCondition);
    state.condition = state.maximumCondition;
    state.reforgeCharges = 3;
    return state;
}

double runeAlignmentMultiplier(const GearItemDefinition& item, const RuneDefinition& rune) {
    if (!rune.elementId || !item.elementId) return 1.0;
    return *rune.elementId == *item.elementId ? 1.75 : 1.0;
}

bool socketRune(GearItemState& item, const GearItemDefinition& definition,
                const RpgCatalog& catalog, std::string_view runeId, std::string* failureReason) {
    if (item.itemId != definition.id) {
        if (failureReason) *failureReason = "gear state/definition id mismatch";
        return false;
    }
    try { (void)catalog.rune(runeId); }
    catch (const std::exception&) {
        if (failureReason) *failureReason = "unknown rune";
        return false;
    }
    if (std::find(item.socketedRunes.begin(), item.socketedRunes.end(), runeId) != item.socketedRunes.end()) {
        if (failureReason) *failureReason = "duplicate rune on one item";
        return false;
    }
    if (static_cast<int>(item.socketedRunes.size()) >= socketCapacityForTier(item.tier)) {
        if (failureReason) *failureReason = "no open rune socket";
        return false;
    }
    item.socketedRunes.emplace_back(runeId);
    if (failureReason) failureReason->clear();
    return true;
}

ReforgeResult reforgeGear(GearItemState& item, const GearItemDefinition& definition,
                          double reforgeScaleValue, std::uint64_t deterministicEntropy) {
    ReforgeResult result;
    if (item.itemId != definition.id) {
        result.failureReason = "gear state/definition id mismatch";
        return result;
    }
    if (item.reforgeCharges <= 0) {
        result.failureReason = "no reforge charges remaining";
        return result;
    }
    const int points = reforgeFinalPoints(item.tier, definition.gradeMultiplier, reforgeScaleValue);
    std::uint64_t state = elysium::mix64(deterministicEntropy ^ stableTextHash(item.itemId) ^
                                         (static_cast<std::uint64_t>(static_cast<std::uint32_t>(item.tier)) << 32U) ^
                                         static_cast<std::uint64_t>(item.reforgeCharges));
    const int half = std::max(1, points / 2);
    const int third = std::max(1, points / 3);
    result.rolls.armour = rollOneTo(state, half);
    result.rolls.health = rollOneTo(state, half);
    result.rolls.speed = rollOneTo(state, third);
    item.reforgeRolls = result.rolls;
    --item.reforgeCharges;
    result.success = true;
    return result;
}

AscensionResult ascendGear(GearItemState& target,
                           std::optional<GearItemState>& counterpart,
                           const GearItemDefinition& definition) {
    AscensionResult result;
    if (!definition.canAscend) {
        result.failureReason = "item definition forbids ascension";
        return result;
    }
    if (!counterpart) {
        result.failureReason = "missing counterpart";
        return result;
    }
    if (target.itemId != definition.id || counterpart->itemId != definition.id || target.itemId != counterpart->itemId) {
        result.failureReason = "ascension requires the same item";
        return result;
    }
    if (target.tier != counterpart->tier) {
        result.failureReason = "ascension requires equal effective tiers";
        return result;
    }
    if (target.tier == std::numeric_limits<int>::max()) {
        result.failureReason = "tier representation exhausted";
        return result;
    }
    ++target.tier;
    target.maximumCondition = std::max(target.maximumCondition, definition.maximumCondition);
    target.condition = target.maximumCondition;
    target.reforgeCharges = 3;
    counterpart.reset(); // Consumption is explicit and atomic at the transaction boundary.
    result.success = true;
    return result;
}

const GearItemDefinition& findGearDefinition(const CharacterBuild& build, std::string_view itemId) {
    const auto it = std::find_if(build.gearDefinitions.begin(), build.gearDefinitions.end(),
                                 [&](const GearItemDefinition& d) { return d.id == itemId; });
    if (it == build.gearDefinitions.end()) throw std::out_of_range("missing gear definition for state: " + std::string(itemId));
    return *it;
}

int trinketSlotCapacity(GearSlot slot) {
    switch (slot) {
    case GearSlot::Ring: return 2;
    case GearSlot::Back:
    case GearSlot::Belt:
    case GearSlot::Charm:
    case GearSlot::Hands:
    case GearSlot::Head:
    case GearSlot::Necklace:
        return 1;
    default:
        return 0;
    }
}

LoadoutValidationResult validateTrinketLoadout(const RpgCatalog& catalog,
                                               const CharacterBuild& build) {
    LoadoutValidationResult result;
    std::array<int, static_cast<std::size_t>(GearSlot::Frame) + 1> used{};
    const auto level = std::max<std::uint64_t>(1, build.progression.level);

    auto issue = [&](ContentId id, std::string reason) {
        result.valid = false;
        result.issues.push_back({std::move(id), std::move(reason)});
    };

    for (const auto& instance : build.trinkets) {
        const TrinketDefinition* definition = nullptr;
        try { definition = &catalog.trinket(instance.id); }
        catch (const std::exception&) {
            issue(instance.id, "unknown trinket id");
            continue;
        }

        if (definition->requiredLevel > 0 &&
            level < static_cast<std::uint64_t>(definition->requiredLevel)) {
            issue(instance.id, "character level is below trinket requirement");
        }
        if (!definition->canAscend && instance.ascensions != 0) {
            issue(instance.id, "found trinket cannot ascend");
        }
        if (instance.ascensions < 0) {
            issue(instance.id, "trinket ascension count cannot be negative");
        }

        const auto slotIndex = static_cast<std::size_t>(definition->slot);
        const int capacity = trinketSlotCapacity(definition->slot);
        if (capacity <= 0) {
            issue(instance.id, "trinket uses a non-accessory gear slot");
            continue;
        }
        if (++used[slotIndex] > capacity) {
            issue(instance.id, "trinket slot capacity exceeded");
        }
    }
    return result;
}

StatBlock evaluateCharacterStats(const RpgCatalog& catalog, const CharacterBuild& build) {
    const auto& raceDef = catalog.race(build.raceId);
    const auto& classDef = catalog.characterClass(build.classId);
    const auto level = std::max<std::uint64_t>(1, build.progression.level);
    const double levelSteps = static_cast<double>(level - 1);
    StatBlock total = raceDef.baseStats;
    total.add(raceDef.growthPerLevel, levelSteps);
    total.add(classDef.growthPerLevel, levelSteps);
    total.add(build.progression.spentStats);

    for (const auto& item : build.gear) {
        const auto& def = findGearDefinition(build, item.itemId);
        const int weight = statWeight(item.tier);
        if (def.elementId) {
            const auto& elementDef = catalog.element(*def.elementId);
            total.get(elementDef.grantedStats[0]) += weight;
            total.get(elementDef.grantedStats[1]) += weight;
        } else {
            total.get(Stat::Fortitude) += weight;
            total.get(Stat::Vitality) += weight;
        }
        if (def.armour) total.get(Stat::Fortitude) += std::max(1, weight / 2);
        total.get(Stat::Fortitude) += item.reforgeRolls.armour;
        total.get(Stat::Vitality) += item.reforgeRolls.health;
        total.get(Stat::Agility) += item.reforgeRolls.speed;
    }
    return total;
}

PassiveBreakdown evaluatePassiveBreakdown(const RpgCatalog& catalog, const CharacterBuild& build,
                                          const PassiveContext& context, const StatBlock* precomputedStats) {
    PassiveBreakdown breakdown;
    auto apply = [&](PassiveSourceKind kind, const ContentId& sourceId, const PassiveQuery& query,
                     int tier, double amplifier) {
        if (!query) return;
        auto contribution = query(context, tier, amplifier);
        mergePassiveContribution(breakdown.summary, contribution);
        breakdown.sources.push_back({kind, sourceId, tier, amplifier, std::move(contribution)});
    };

    const auto& race = catalog.race(build.raceId);
    const auto& characterClass = catalog.characterClass(build.classId);
    apply(PassiveSourceKind::Race, race.id, race.passive, 0, 1.0);
    apply(PassiveSourceKind::Class, characterClass.id, characterClass.passive, 0, 1.0);

    for (const auto& instance : build.trinkets) {
        const auto& definition = catalog.trinket(instance.id);
        const int ascensions = definition.canAscend ? std::max(0, instance.ascensions) : 0;
        apply(PassiveSourceKind::Trinket, definition.id, definition.passive, ascensions, 1.0);
    }

    for (const auto& item : build.gear) {
        const auto& itemDef = findGearDefinition(build, item.itemId);
        for (const auto& runeId : item.socketedRunes) {
            const auto& rune = catalog.rune(runeId);
            const double amplifier = runeAlignmentMultiplier(itemDef, rune);
            apply(PassiveSourceKind::Rune, rune.id, rune.passive, item.tier, amplifier);
        }
    }

    const StatBlock statsBlock = precomputedStats ? *precomputedStats : evaluateCharacterStats(catalog, build);
    const auto derived = deriveStats(statsBlock);
    breakdown.summary.favorScale *= derived.presenceScale;
    breakdown.summary.suspicionScale *= derived.presenceScale;
    breakdown.summary.reforgeScale *= derived.presenceScale;
    breakdown.summary.extraDropChance = combineIndependentShares(breakdown.summary.extraDropChance, derived.luckExtraDropChance);
    breakdown.summary.decayRateContribution = std::max(1.0, breakdown.summary.decayRateContribution);
    return breakdown;
}

PassiveSummary evaluatePassives(const RpgCatalog& catalog, const CharacterBuild& build,
                                const PassiveContext& context, const StatBlock* precomputedStats) {
    return evaluatePassiveBreakdown(catalog, build, context, precomputedStats).summary;
}

bool deterministicChance(std::uint64_t entropy, double probability) {
    probability = clamp01(probability);
    if (probability <= 0.0) return false;
    if (probability >= 1.0) return true;
    const std::uint64_t h = elysium::mix64(entropy);
    constexpr long double denom = static_cast<long double>(std::numeric_limits<std::uint64_t>::max());
    const long double sample = static_cast<long double>(h) / denom;
    return sample < probability;
}

void registerCanonicalRpgContent(RpgCatalog& catalog) {
    catalog.registerElement({std::string(ids::elementVoid), "Void",
        {std::string(ids::elementKinetic), std::string(ids::elementDimensional)},
        {Stat::Resilience, Stat::Willpower}, HazardAffinity::Radiological});
    catalog.registerElement({std::string(ids::elementPlasma), "Plasma",
        {std::string(ids::elementVoid), std::string(ids::elementKinetic)},
        {Stat::Strength, Stat::Accuracy}, HazardAffinity::Thermal});
    catalog.registerElement({std::string(ids::elementNeural), "Neural",
        {std::string(ids::elementPlasma), std::string(ids::elementVoid)},
        {Stat::Intellect, Stat::Agility}, HazardAffinity::Corrosive});
    catalog.registerElement({std::string(ids::elementDimensional), "Dimensional",
        {std::string(ids::elementNeural), std::string(ids::elementPlasma)},
        {Stat::Agility, Stat::Reflexes}, HazardAffinity::Pressure});
    catalog.registerElement({std::string(ids::elementKinetic), "Kinetic",
        {std::string(ids::elementDimensional), std::string(ids::elementNeural)},
        {Stat::Strength, Stat::Retribution}, HazardAffinity::Cryogenic});

    addRace(catalog, std::string(ids::raceImperial), "Imperial",
        stats({{Stat::Vitality,4},{Stat::Fortitude,4},{Stat::Resilience,3},{Stat::Strength,4},{Stat::Agility,3},{Stat::Accuracy,3},
               {Stat::Reflexes,3},{Stat::Retribution,5},{Stat::Intellect,3},{Stat::Willpower,3},{Stat::Luck,3},{Stat::Presence,6}}),
        stats({{Stat::Retribution,1},{Stat::Presence,1},{Stat::Vitality,1}}),
        [](const PassiveContext& c, int, double){ PassiveContribution p; const long double l=std::max<std::uint64_t>(1,c.level); p.reflectShare=static_cast<double>(l/(l+100.0L)); return p; });
    addRace(catalog, std::string(ids::raceDruun), "Druun",
        stats({{Stat::Vitality,6},{Stat::Fortitude,8},{Stat::Resilience,5},{Stat::Strength,8},{Stat::Agility,2},{Stat::Accuracy,3},
               {Stat::Reflexes,2},{Stat::Retribution,2},{Stat::Intellect,1},{Stat::Willpower,3},{Stat::Luck,2},{Stat::Presence,2}}),
        stats({{Stat::Strength,2},{Stat::Fortitude,1}}),
        [](const PassiveContext& c, int, double){ PassiveContribution p; p.attackScale=1.0+0.60*(1.0-clamp01(c.healthFraction)); return p; });
    addRace(catalog, std::string(ids::raceVeylari), "Veylari",
        stats({{Stat::Vitality,3},{Stat::Fortitude,2},{Stat::Resilience,2},{Stat::Strength,3},{Stat::Agility,6},{Stat::Accuracy,8},
               {Stat::Reflexes,5},{Stat::Retribution,1},{Stat::Intellect,7},{Stat::Willpower,2},{Stat::Luck,3},{Stat::Presence,2}}),
        stats({{Stat::Intellect,2},{Stat::Accuracy,1}}),
        [](const PassiveContext&, int, double){ PassiveContribution p; p.fallDistanceIgnored=10.0; p.fallDamageScale=1.0/3.0; return p; });
    addRace(catalog, std::string(ids::raceKorrath), "Korrath",
        stats({{Stat::Vitality,5},{Stat::Fortitude,3},{Stat::Resilience,3},{Stat::Strength,4},{Stat::Agility,8},{Stat::Accuracy,4},
               {Stat::Reflexes,7},{Stat::Retribution,2},{Stat::Intellect,2},{Stat::Willpower,1},{Stat::Luck,4},{Stat::Presence,1}}),
        stats({{Stat::Agility,2},{Stat::Reflexes,1}}),
        [](const PassiveContext& c, int, double){ PassiveContribution p; if(!c.hurtRecovery && c.secondsSinceLastHit>=5.0) p.regenScale=3.0; return p; });
    addRace(catalog, std::string(ids::raceLumari), "Lumari",
        stats({{Stat::Vitality,2},{Stat::Fortitude,1},{Stat::Resilience,5},{Stat::Strength,2},{Stat::Agility,3},{Stat::Accuracy,3},
               {Stat::Reflexes,3},{Stat::Retribution,3},{Stat::Intellect,8},{Stat::Willpower,9},{Stat::Luck,3},{Stat::Presence,2}}),
        stats({{Stat::Willpower,2},{Stat::Intellect,1}}),
        [](const PassiveContext& c, int, double){ PassiveContribution p; p.shieldScale=2.0; p.defenceScale=c.incomingFireOrExplosion?0.66:1.15; return p; });
    addRace(catalog, std::string(ids::raceUnsworn), "Unsworn",
        stats({{Stat::Vitality,4},{Stat::Fortitude,3},{Stat::Resilience,3},{Stat::Strength,5},{Stat::Agility,6},{Stat::Accuracy,4},
               {Stat::Reflexes,4},{Stat::Retribution,2},{Stat::Intellect,2},{Stat::Willpower,2},{Stat::Luck,9},{Stat::Presence,0}}),
        stats({{Stat::Luck,2},{Stat::Strength,1}}),
        [](const PassiveContext&, int, double){ PassiveContribution p; p.favorScale=.5; p.suspicionScale=.5; p.suspicionDecayScale=2.0; return p; });

    addClass(catalog, std::string(ids::classMedicae), "Medicae", stats({{Stat::Vitality,1},{Stat::Presence,1}}),
        [](const PassiveContext&,int,double){ PassiveContribution p; p.regenScale=1.5; p.nearbyTriageHealScale=.75; return p; });
    addClass(catalog, std::string(ids::classFactor), "Factor", stats({{Stat::Luck,1},{Stat::Presence,1}}),
        [](const PassiveContext&,int,double){ PassiveContribution p; p.extraDropChance=.25; return p; });
    addClass(catalog, std::string(ids::classArtificer), "Artificer", stats({{Stat::Intellect,1},{Stat::Presence,1}}),
        [](const PassiveContext&,int,double){ PassiveContribution p; p.savesDurabilityChance=.34; p.reforgeScale=1.5; return p; });
    addClass(catalog, std::string(ids::classEnforcer), "Enforcer", stats({{Stat::Strength,1},{Stat::Fortitude,1}}),
        [](const PassiveContext& c,int,double){ PassiveContribution p; if(c.targetIsUnsworn) p.attackScale=1.25; p.suspicionScale=.6; return p; });
    addClass(catalog, std::string(ids::classPsion), "Psion", stats({{Stat::Intellect,1},{Stat::Willpower,1}}),
        [](const PassiveContext&,int,double){ PassiveContribution p; p.psionicScale=1.5; return p; });
    addClass(catalog, std::string(ids::classVoidrunner), "Voidrunner", stats({{Stat::Agility,1},{Stat::Reflexes,1}}),
        [](const PassiveContext&,int,double){ PassiveContribution p; p.fallDamageScale=.5; return p; });
    addClass(catalog, std::string(ids::classReclaimer), "Reclaimer", stats({{Stat::Fortitude,1},{Stat::Luck,1}}),
        [](const PassiveContext&,int,double){ PassiveContribution p; p.doublesOreChance=.25; return p; });
    addClass(catalog, std::string(ids::classWarden), "Warden", stats({{Stat::Resilience,1},{Stat::Retribution,1}}),
        [](const PassiveContext& c,int,double){ PassiveContribution p; if(c.healthFraction<.5) p.reflectMultiplier=2.0; return p; });
    addClass(catalog, std::string(ids::classMarksman), "Marksman", stats({{Stat::Accuracy,1},{Stat::Agility,1}}),
        [](const PassiveContext&,int,double){ PassiveContribution p; p.critMultiplier=2.25; return p; });

    addRune(catalog, std::string(ids::runeVoidward), "Voidward", ContentId(ids::elementVoid),
        [](const PassiveContext& c,int,double a){ PassiveContribution p; p.armourToughnessAdd=2.0*a; p.resistanceEffect=c.healthFraction<.40; return p; });
    addRune(catalog, std::string(ids::runePlasmaforge), "Plasmaforge", ContentId(ids::elementPlasma),
        [](const PassiveContext& c,int,double a){ PassiveContribution p; p.attackDamageAdd=1.5*a; p.strengthEffect=c.healthFraction>.70; return p; });
    addRune(catalog, std::string(ids::runeNeuralspike), "Neuralspike", ContentId(ids::elementNeural),
        [](const PassiveContext&,int,double a){ PassiveContribution p; p.attackSpeedScale=1.0+.15*a; p.hasteEffect=true; return p; });
    addRune(catalog, std::string(ids::runeDimensionalshift), "Dimensionalshift", ContentId(ids::elementDimensional),
        [](const PassiveContext& c,int,double a){ PassiveContribution p; p.movementScale=1.0+.08*a; p.slowFallingEffect=c.fallingDistance>2.5; return p; });
    addRune(catalog, std::string(ids::runeKineticsurge), "Kineticsurge", ContentId(ids::elementKinetic),
        [](const PassiveContext&,int,double a){ PassiveContribution p; p.knockbackResistanceAdd=.10*a; p.jumpBoostEffect=true; return p; });
    addRune(catalog, std::string(ids::runeStabilizer), "Stabilizer", std::nullopt,
        [](const PassiveContext& c,int,double){ PassiveContribution p; if(c.healthFraction<1.0) p.periodicHealAdd=.5; return p; });
    addRune(catalog, std::string(ids::runeReflex), "Reflex", std::nullopt,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.dodgeChance=.05; return p; });
    addRune(catalog, std::string(ids::runeBarrier), "Barrier", std::nullopt,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.barrierCapacityAdd=4.0; p.barrierRefreshAdd=1.0; return p; });
    addRune(catalog, std::string(ids::runePlasmaCore), "Plasma Core", std::nullopt,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.heatReductionAdd=.12; return p; });

    addTrinket(catalog, tid("widows_thimble"), "Widow's Thimble", GearSlot::Ring,5,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; p.negateOpeningDamageAtFullHealth=(c.event==PassiveEvent::Damaged && c.healthFraction>=1.0 && c.incomingDamage>0.0); return p; });
    addTrinket(catalog, tid("cracked_reliquary"), "Cracked Reliquary", GearSlot::Necklace,5,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; p.regenScale=c.healthFraction<(1.0/3.0)?3.0:.4; return p; });
    addTrinket(catalog, tid("nine_tenths_charm"), "Nine-Tenths Charm", GearSlot::Charm,15,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; p.dodgeChance=(1.0-clamp01(c.healthFraction))*.35; return p; });
    addTrinket(catalog, tid("iron_discipline"), "Iron Discipline", GearSlot::Belt,10,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; if(c.crouching) p.defenceScale=.55; return p; });
    addTrinket(catalog, tid("ashen_mantle"), "Ashen Mantle", GearSlot::Back,15,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; if(c.onFire) p.reflectShare=.30; return p; });
    addTrinket(catalog, tid("splintbone_fetish"), "Splintbone Fetish", GearSlot::Charm,20,true,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.reflectMultiplier=2.0; return p; });
    addTrinket(catalog, tid("empty_reliquary"), "Empty Reliquary", GearSlot::Necklace,20,true,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.shieldScale=2.0; p.regenScale=.5; return p; });
    addTrinket(catalog, tid("gravebound_coil"), "Gravebound Coil", GearSlot::Belt,10,true,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.fallDamageScale=0.0; p.suspicionScale=1.5; return p; });
    addTrinket(catalog, tid("ratchet_gauntlet"), "Ratchet Gauntlet", GearSlot::Hands,15,true,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.critMultiplier=2.6; p.attackScale=.85; return p; });
    addTrinket(catalog, tid("duellists_cuff"), "Duellist's Cuff", GearSlot::Ring,10,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; if(c.targetHealthFraction>=1.0) p.attackScale=1.35; return p; });
    addTrinket(catalog, tid("hollow_chime"), "Hollow Chime", GearSlot::Necklace,20,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; if(c.targetHealthFraction<.5) p.lifestealShare=.20; return p; });
    addTrinket(catalog, tid("carrion_signet"), "Carrion Signet", GearSlot::Ring,15,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; if(c.event==PassiveEvent::Kill) p.healOnKill=std::min(6.0,.10*finiteNonNegative(c.victimMaxHealth)); return p; });
    addTrinket(catalog, tid("pale_tourniquet"), "Pale Tourniquet", GearSlot::Hands,20,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; if(c.event==PassiveEvent::Damaged && c.incomingDamage>=6.0) p.resistanceSecondsOnDamaged=3.0; return p; });
    addTrinket(catalog, tid("deadmans_ledger"), "Deadman's Ledger", GearSlot::Charm,25,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; if(c.hunted) p.favorScale=2.0; return p; });
    addTrinket(catalog, tid("quiet_hours"), "Quiet Hours", GearSlot::Back,1,true,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.favorScale=.5; p.suspicionScale=.5; return p; });
    addTrinket(catalog, tid("unsworn_bell"), "Unsworn Bell", GearSlot::Necklace,10,true,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.favorScale=1.6; p.suspicionScale=1.6; return p; });
    addTrinket(catalog, tid("auditors_seal"), "Auditor's Seal", GearSlot::Charm,15,true,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.decayRateContribution=3.0; return p; });
    addTrinket(catalog, tid("long_memory"), "Long Memory", GearSlot::Charm,25,true,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.blockStandingDecay=true; return p; });
    addTrinket(catalog, tid("tithe_bracelet"), "Tithe Bracelet", GearSlot::Ring,20,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; if(c.event==PassiveEvent::Kill) p.bonusFavorOnKill=1.0; return p; });
    addTrinket(catalog, tid("debtors_knot"), "Debtor's Knot", GearSlot::Charm,25,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; if(c.hunted) p.extraDropChance=.35; return p; });
    addTrinket(catalog, tid("prospectors_lens"), "Prospector's Lens", GearSlot::Head,10,true,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.doublesOreAlways=true; p.xpScale=.6; return p; });
    addTrinket(catalog, tid("cartographers_nail"), "Cartographer's Nail", GearSlot::Hands,5,true,
        [](const PassiveContext& c,int,double){ PassiveContribution p; p.savesDurabilityAlways=c.crouching; return p; });
    addTrinket(catalog, tid("longsight"), "Longsight", GearSlot::Head,15,true,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.xpScale=1.6; p.attackScale=.85; return p; });
    addTrinket(catalog, tid("reforgers_loupe"), "Reforger's Loupe", GearSlot::Head,20,true,
        [](const PassiveContext&,int,double){ PassiveContribution p; p.reforgeScale=1.75; p.psionicScale=.8; return p; });

    addTrinket(catalog, tid("aetherium_band"), "Aetherium Band", GearSlot::Ring,10,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.attackScale=craftedMultiplier(1.15,a); return p; });
    addTrinket(catalog, tid("executioners_grip"), "Executioner's Grip", GearSlot::Hands,20,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.critMultiplier=1.5+.35*tierScale(a); return p; });
    addTrinket(catalog, tid("bloodlet_ring"), "Bloodlet Ring", GearSlot::Ring,20,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.lifestealShare=craftedShare(.06,a); return p; });
    addTrinket(catalog, tid("voidglass_pendant"), "Voidglass Pendant", GearSlot::Necklace,15,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.psionicScale=craftedMultiplier(1.20,a); return p; });
    addTrinket(catalog, tid("neutronium_band"), "Neutronium Band", GearSlot::Ring,10,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.defenceScale=1.0-craftedShare(.08,a); return p; });
    addTrinket(catalog, tid("kinetic_spur"), "Kinetic Spur", GearSlot::Belt,15,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.dodgeChance=craftedShare(.05,a); return p; });
    addTrinket(catalog, tid("thornplate"), "Thornplate", GearSlot::Back,15,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.reflectShare=craftedShare(.10,a); return p; });
    addTrinket(catalog, tid("wardens_gorget"), "Warden's Gorget", GearSlot::Necklace,20,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.shieldScale=craftedMultiplier(1.25,a); return p; });
    addTrinket(catalog, tid("dimensional_anchor"), "Dimensional Anchor", GearSlot::Belt,5,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.fallDistanceIgnored=3.0*tierScale(a); return p; });
    addTrinket(catalog, tid("plasma_cord"), "Plasma Cord", GearSlot::Belt,10,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.regenScale=craftedMultiplier(1.30,a); return p; });
    addTrinket(catalog, tid("neural_filament"), "Neural Filament", GearSlot::Head,10,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.xpScale=craftedMultiplier(1.20,a); return p; });
    addTrinket(catalog, tid("favored_sigil"), "Favored Sigil", GearSlot::Charm,5,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.favorScale=craftedMultiplier(1.25,a); return p; });
    addTrinket(catalog, tid("shrouded_sigil"), "Shrouded Sigil", GearSlot::Charm,5,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.suspicionScale=1.0-craftedShare(.15,a); return p; });
    addTrinket(catalog, tid("prospect_charm"), "Prospect Charm", GearSlot::Charm,10,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.extraDropChance=craftedShare(.08,a); return p; });
    addTrinket(catalog, tid("artificers_loupe"), "Artificer's Loupe", GearSlot::Head,15,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.reforgeScale=craftedMultiplier(1.20,a); return p; });
    addTrinket(catalog, tid("miners_rig"), "Miner's Rig", GearSlot::Hands,10,false,
        [](const PassiveContext&,int a,double){ PassiveContribution p; p.savesDurabilityChance=craftedShare(.20,a); p.doublesOreChance=craftedShare(.12,a); return p; });

    catalog.registerGearFamily({gid("hazard_suit_modules"), "Hazard suit modules", {GearSlot::Undersuit,GearSlot::Pack},
        {"oxygen","shielding","filtration","pressure","thermal management"}});
    catalog.registerGearFamily({gid("utility_harness"), "Utility harness", {GearSlot::Belt,GearSlot::Back},
        {"cargo","scanner charge","tool energy"}});
    catalog.registerGearFamily({gid("industrial_exoskeleton"), "Industrial exoskeleton", {GearSlot::Frame},
        {"carry capacity","mining speed","fall control at energy cost"}});
    catalog.registerGearFamily({gid("survey_kit"), "Survey kit", {GearSlot::Head,GearSlot::Back},
        {"scan range","flora analysis","fauna analysis","mineral analysis"}});
    catalog.registerGearFamily({gid("siege_kit"), "Siege kit", {GearSlot::Back,GearSlot::Hands},
        {"base repair","heavy ammunition","turret interaction"}});
    catalog.registerGearFamily({gid("explorer_kit"), "Explorer kit", {GearSlot::Feet,GearSlot::Back},
        {"mobility","stamina/energy efficiency","hazard traversal"}});
}

RpgCatalog makeCanonicalRpgCatalog() {
    RpgCatalog catalog;
    registerCanonicalRpgContent(catalog);
    catalog.freeze();
    return catalog;
}

} // namespace elysium::rpg
