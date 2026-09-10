#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Plan strategic fuel, ammunition, spare parts, food, medical, transport, depots, and convoy support for campaigns.
struct MilitaryLogisticsPlannerCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct MilitaryLogisticsPlannerState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class MilitaryLogisticsPlannerSystem { public: bool submit(const MilitaryLogisticsPlannerCommand&); const MilitaryLogisticsPlannerState* find(std::uint64_t) const; std::vector<MilitaryLogisticsPlannerState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MilitaryLogisticsPlannerState> map_; };
}
