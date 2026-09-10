#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate invasion staging, objectives, fleets, landing sites, supply routes, timing, reserves, and retreat conditions.
struct InvasionPlannerCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct InvasionPlannerState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class InvasionPlannerSystem { public: bool submit(const InvasionPlannerCommand&); const InvasionPlannerState* find(std::uint64_t) const; std::vector<InvasionPlannerState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,InvasionPlannerState> map_; };
}
