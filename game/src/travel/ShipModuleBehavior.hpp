#pragma once
#include "travel/ShipFrameRuntime.hpp"
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::travel {

enum class ModuleAvailability : std::uint8_t { Available, ProposedUnavailable };
enum class ModuleEffect : std::uint32_t {
 FuelCapacity=1u<<0, CargoCapacity=1u<<1, SurveyScan=1u<<2, DeepScan=1u<<3,
 AtmosphereSampling=1u<<4, HullProtection=1u<<5, ThermalMitigation=1u<<6,
 RadiationMitigation=1u<<7, Autopilot=1u<<8, Navigation=1u<<9,
 EmergencyRecovery=1u<<10, Docking=1u<<11, DroneSupport=1u<<12,
 Habitation=1u<<13, FieldRefinery=1u<<14, SmugglerExposure=1u<<15
};
constexpr std::uint32_t moduleEffect(ModuleEffect e) noexcept { return static_cast<std::uint32_t>(e); }

struct ModuleBehaviorDefinition {
 std::uint64_t contentId{};
 ShipSlotDomain domain{};
 ModuleAvailability availability{ModuleAvailability::Available};
 std::uint32_t effects{};
 std::uint64_t fuelCapacityBonus{};
 std::uint64_t cargoCapacityBonus{};
 double hullBonus{};
 double thermalMitigation{};
 double radiationMitigation{};
 std::uint32_t scannerGradeBonus{};
 double customsExposureMultiplier{1.0};
 std::uint64_t requiredClearanceId{};
 std::uint64_t requiredDependencyId{};
 bool requiresPower{};
 bool requiresFuel{};
};

struct InstalledModuleBehaviorState {
 std::uint64_t stableItemId{};
 std::uint64_t moduleContentId{};
 std::uint64_t revision{1};
 double durability{1.0};
 bool enabled{true};
 bool powered{true};
 bool fueled{true};
 std::uint64_t clearanceId{};
 bool dependencyAvailable{true};
};

enum class ModuleBlockReason : std::uint8_t {
 None, MissingDefinition, ProposedUnavailable, InvalidDomain, Disabled, Broken,
 Unpowered, Unfueled, MissingClearance, MissingDependency, DuplicateStableItem,
 InvalidValue
};
struct ModuleBlocker { std::uint64_t stableItemId{}; ModuleBlockReason reason{ModuleBlockReason::None}; };

struct ShipModuleProjection {
 bool valid{true};
 std::uint64_t fuelCapacityBonus{};
 std::uint64_t cargoCapacityBonus{};
 double hullBonus{};
 double thermalMitigation{};
 double radiationMitigation{};
 std::uint32_t scannerGradeBonus{};
 double customsExposureMultiplier{1.0};
 std::uint32_t activeEffects{};
 std::vector<ModuleBlocker> blockers;
 [[nodiscard]] bool has(ModuleEffect e) const noexcept { return (activeEffects & moduleEffect(e)) != 0; }
};

struct ShipModuleBehaviorSnapshot {
 std::vector<ModuleBehaviorDefinition> definitions;
 std::vector<InstalledModuleBehaviorState> installed;
};

class ShipModuleBehaviorRuntime {
public:
 bool publish(ModuleBehaviorDefinition);
 bool install(InstalledModuleBehaviorState);
 bool update(const InstalledModuleBehaviorState& expectedCurrent, InstalledModuleBehaviorState replacement);
 bool remove(std::uint64_t stableItemId, std::uint64_t expectedRevision);
 [[nodiscard]] const InstalledModuleBehaviorState* find(std::uint64_t stableItemId) const;
 [[nodiscard]] ShipModuleProjection project(const std::vector<std::uint64_t>& stableItemIds) const;
 [[nodiscard]] ShipModuleBehaviorSnapshot snapshot() const;
 bool restore(const ShipModuleBehaviorSnapshot&);
private:
 std::map<std::uint64_t,ModuleBehaviorDefinition> definitions_;
 std::map<std::uint64_t,InstalledModuleBehaviorState> installed_;
};

} // namespace elysium::travel
