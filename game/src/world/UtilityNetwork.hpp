// Intended function: bounded local cable/pipe/duct network solver for power, liquids, gases, heat and data utilities.
#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>
namespace elysium{
enum class UtilityKind:std::uint8_t{Power,Water,Oxygen,Hydrogen,Coolant,Sewage,Heat,Data};
struct UtilityPort{std::uint64_t portId{},ownerId{},address{};UtilityKind kind{};float supply{},demand{},capacity{};bool enabled{true};};
struct UtilityLink{std::uint64_t linkId{},a{},b{};UtilityKind kind{};float capacity{},lossPerUnit{};bool enabled{true};};
struct UtilityAllocation{std::uint64_t sourcePort{},sinkPort{};UtilityKind kind{};float delivered{},lost{};};
class UtilityNetwork{public:void upsertPort(UtilityPort p);void upsertLink(UtilityLink l);std::vector<UtilityAllocation>solve(UtilityKind kind,std::size_t maxAllocations=4096)const;private:std::unordered_map<std::uint64_t,UtilityPort>ports_;std::unordered_map<std::uint64_t,UtilityLink>links_;};
}
