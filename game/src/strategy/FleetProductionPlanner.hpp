#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Allocate strategic shipbuilding capacity across hull classes, repairs, refits, escorts, logistics, and reserve goals.
struct FleetProductionPlannerCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct FleetProductionPlannerState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class FleetProductionPlannerSystem { public: bool submit(const FleetProductionPlannerCommand&); const FleetProductionPlannerState* find(std::uint64_t) const; std::vector<FleetProductionPlannerState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FleetProductionPlannerState> map_; };
}
