#pragma once
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <span>
#include <vector>
namespace elysium::base {
using StableId=std::uint64_t;
struct PowerNodeSnapshot {
    StableId nodeId{};std::uint64_t baseId{};std::uint64_t networkId{};
    double generation{};double demand{};double storedEnergy{};double storageCapacity{};
    double maxChargeRate{};double maxDischargeRate{};std::uint16_t priority{100};bool isolated{};bool enabled{true};
};
struct PowerNodeAllocation { StableId nodeId{};double served{};double storageDelta{};bool powered{};bool shed{}; };
struct PowerAllocationPlan { double generation{},demand{},served{},unserved{},storageDelta{};std::vector<PowerNodeAllocation> nodes; };
class ScalarPowerNetwork {
public:[[nodiscard]] PowerAllocationPlan allocate(std::span<const PowerNodeSnapshot> nodes,double dt)const;
};
} // namespace elysium::base
