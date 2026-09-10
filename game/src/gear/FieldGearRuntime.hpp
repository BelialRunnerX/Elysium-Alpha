#pragma once
#include <cstdint>
namespace elysium::gear {
struct FieldGearInput{double availableEnergy{},dt{},carriedMass{},fallSpeed{},scanWork{},repairWork{};bool industrialExo{},surveyKit{},siegeKit{},explorerKit{};};
struct FieldGearOutput{double energyConsumed{},carryCapacityScale{1},miningRateScale{1},fallDamageScale{1},scanRateScale{1},repairRateScale{1},movementEfficiencyScale{1};bool toolPowerLost{};};
class FieldGearRuntime{public:[[nodiscard]]FieldGearOutput evaluate(const FieldGearInput&i)const;};
} // namespace elysium::gear
