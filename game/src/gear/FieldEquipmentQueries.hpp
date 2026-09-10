#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::gear {
enum class FieldEffect : std::uint8_t {
    ScanRange, AnalysisDetail, RepairRate, HeavyAmmo, TurretUse,
    GroundMobility, EnergyEfficiency, HazardTraversal, Count
};
enum class FieldQueryReason { Ready, InvalidInput, UnknownEffect, EnergyUnavailable, PermissionDenied };
struct FieldEffectDefinition {
    std::uint64_t id{}, passiveId{};
    FieldEffect kind{};
    double magnitude{}, energyPerSecond{}, signature{};
};
struct ActiveFieldEffect { std::uint64_t item{}, effect{}; bool enabled{true}; };
struct FieldQueryInput {
    double seconds{}, energy{}, baseScanRange{}, baseRepairRate{};
    bool authorizedTurret{}, authorizedHeavyAmmo{};
};
struct FieldQueryOutput {
    FieldQueryReason reason{FieldQueryReason::Ready};
    double energyConsumed{}, scanRange{}, repairRate{}, mobilityScale{1},
           movementEnergyScale{1}, traversalBonus{}, signature{};
    unsigned analysisDetail{};
    bool canUseTurret{}, canHandleHeavyAmmo{};
    std::vector<std::uint64_t> contributingPassives, unpoweredItems;
};
// Produces requests/modifiers only. Scanner discovers, construction consumes
// repair materials, and defense authorizes turret commands in their own owners.
class FieldEquipmentQueries {
public:
    bool publish(FieldEffectDefinition);
    bool restore(std::vector<ActiveFieldEffect>);
    std::vector<ActiveFieldEffect> snapshot() const { return equipped_; }
    FieldQueryOutput evaluate(const FieldQueryInput&) const;
private:
    std::map<std::uint64_t,FieldEffectDefinition> definitions_;
    std::vector<ActiveFieldEffect> equipped_;
};
}
