#pragma once
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>
namespace elysium::gear {
using StableId=std::uint64_t;using ContentId=std::uint64_t;
enum class EquipmentChannel:std::uint8_t{Head,Undersuit,Back,Belt,Hands,Feet,Frame,Pack};
struct GearModuleDefinition{ContentId moduleId{},familyId{};std::vector<EquipmentChannel>occupies;double powerDraw{},oxygenModifier{},hazardShieldModifier{},carryModifier{},toolEnergyModifier{};};
struct EquippedModule{StableId itemId{};ContentId moduleId{};bool enabled{true};};
struct GearLoadoutEvaluation{bool valid{};reason::ReasonStack reasons;double powerDraw{},oxygenModifier{},hazardShieldModifier{},carryModifier{},toolEnergyModifier{};};
class GearFamilyRuntime{public:bool publish(GearModuleDefinition d,reason::ReasonStack*r=nullptr);[[nodiscard]]GearLoadoutEvaluation evaluate(const std::vector<EquippedModule>&loadout)const;private:std::unordered_map<ContentId,GearModuleDefinition>defs_;};
} // namespace elysium::gear
