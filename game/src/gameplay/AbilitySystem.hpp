// Intended function: data-driven ability cooldown/charge/resource gating that emits activation facts instead of directly applying effects.
#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
namespace elysium{
struct AbilityDefinition{std::uint64_t abilityId{};float energyCost{},cooldownSeconds{};std::uint8_t maxCharges{1};std::uint32_t tagMask{};};
struct AbilityRuntime{std::uint64_t actorId{},abilityId{};float cooldownRemaining{};std::uint8_t charges{1};};
struct AbilityActivation{std::uint64_t actorId{},abilityId{},targetId{},targetAddress{};float energySpent{};};
class AbilitySystem{public:void registerDefinition(AbilityDefinition d);void grant(std::uint64_t actor,std::uint64_t ability);void tick(float dt);std::optional<AbilityActivation>tryActivate(std::uint64_t actor,std::uint64_t ability,std::uint64_t target,std::uint64_t address,float availableEnergy);private:std::unordered_map<std::uint64_t,AbilityDefinition>defs_;std::vector<AbilityRuntime>runtime_;};
}
