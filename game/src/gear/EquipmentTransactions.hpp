#pragma once
#include "gear/GearFamilyRuntime.hpp"
#include "survival/Survival.hpp"
#include <array>
#include <map>
#include <string>

namespace elysium::gear {
// Content identifiers, never family names, control compatibility and behavior.
enum class Obtainability : std::uint8_t { Available, Proposed, MissingRecipe };
enum class EquipmentError : std::uint8_t {
    None, InvalidDefinition, UnknownModule, InvalidItem, DuplicateItem,
    IncompatibleFamily, SlotConflict, Unavailable, StaleRevision, InvalidSnapshot
};
enum class HookKind : std::uint8_t { Activate, Deactivate };
struct FamilyModule {
    ContentId id{}, family{}, recipe{};
    std::vector<EquipmentChannel> slots;
    std::vector<ContentId> compatibleHostFamilies;
    std::vector<ContentId> passiveIds;
    std::array<float,kSurvivalHazardCount> hazardShares{};
    float energyTax{}, oxygenTax{}, signatureDelta{}, carryDelta{};
    Obtainability obtainability{Obtainability::MissingRecipe};
};
struct EquipmentItem { StableId id{}; ContentId module{}; bool enabled{true}; };
struct EquipmentSnapshot {
    std::uint32_t schema{1};
    StableId owner{};
    ContentId hostFamily{};
    std::uint64_t revision{};
    std::vector<EquipmentItem> items;
};
struct EquipmentHook {
    HookKind kind{}; StableId owner{}, item{}; ContentId module{};
    std::uint64_t revision{};
};
struct ModuleContributions {
    struct Passive { StableId item{}; ContentId passive{}; };
    std::array<float,kSurvivalHazardCount> hazardShares{};
    float energyTax{}, oxygenTax{}, signatureDelta{}, carryDelta{};
    std::vector<Passive> passives;
};
// Owner-thread service. Mutation is compare-and-swap against a local revision.
// Hooks are committed in the same operation; consumers apply them once using
// (owner, revision, item, kind). Persistence contains no transient ECS handles.
class EquipmentTransactions {
public:
    EquipmentTransactions(StableId owner, ContentId hostFamily);
    EquipmentError publish(FamilyModule definition);
    EquipmentError equip(EquipmentItem item, std::uint64_t expectedRevision);
    EquipmentError remove(StableId item, std::uint64_t expectedRevision);
    EquipmentError setEnabled(StableId item, bool enabled, std::uint64_t expectedRevision);
    EquipmentError restore(const EquipmentSnapshot&, std::uint64_t expectedRevision);
    EquipmentSnapshot snapshot() const;
    ModuleContributions contributions() const;
    std::vector<EquipmentHook> drainHooks();
    std::uint64_t revision() const { return revision_; }
    static void applySurvival(const ModuleContributions&, SurvivalTickInput&);
private:
    EquipmentError validate(const std::vector<EquipmentItem>&) const;
    EquipmentError commit(std::vector<EquipmentItem>, std::uint64_t expectedRevision);
    StableId owner_{}; ContentId host_{}; std::uint64_t revision_{};
    std::map<ContentId, FamilyModule> definitions_;
    std::vector<EquipmentItem> items_;
    std::vector<EquipmentHook> hooks_;
};
}
