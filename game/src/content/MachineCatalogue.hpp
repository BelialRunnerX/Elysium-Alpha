// Intended function: immutable machine definitions/capabilities separate from live runtime state.
#pragma once
#include <cstdint>
#include <span>
#include <vector>
namespace elysium::content{
struct MachineCapability{std::uint64_t capabilityId{};double rate{};std::uint32_t flags{};};
struct MachineDefinition{std::uint64_t machineId{};std::uint64_t constructionRecipeId{};double idlePower{};double activePower{};std::uint64_t inputCapacity{};std::uint64_t outputCapacity{};std::vector<MachineCapability>capabilities;std::uint32_t flags{};};
class MachineCatalogue final{public:bool publish(MachineDefinition d);const MachineDefinition*find(std::uint64_t id)const;bool supports(std::uint64_t machineId,std::uint64_t capabilityId)const;double capabilityRate(std::uint64_t machineId,std::uint64_t capabilityId)const;const std::vector<MachineDefinition>&all()const noexcept{return defs_;}private:static MachineDefinition sanitize(MachineDefinition d);std::vector<MachineDefinition>defs_;};
}
