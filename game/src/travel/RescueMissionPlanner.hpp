#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate rescue routes and asset assignments for stranded ships, injured explorers, lost squads, and disasters.
struct RescueMissionPlannerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RescueMissionPlannerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RescueMissionPlannerStore { public: bool apply(const RescueMissionPlannerOp&); bool erase(std::uint64_t); const RescueMissionPlannerData* find(std::uint64_t) const; std::vector<RescueMissionPlannerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RescueMissionPlannerData> data_; };
}
