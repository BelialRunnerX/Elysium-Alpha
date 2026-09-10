#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Compute deterministic surface-to-orbit and interbody launch windows from simplified orbital summaries.
struct LaunchWindowPlannerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct LaunchWindowPlannerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class LaunchWindowPlannerStore { public: bool apply(const LaunchWindowPlannerOp&); bool erase(std::uint64_t); const LaunchWindowPlannerData* find(std::uint64_t) const; std::vector<LaunchWindowPlannerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,LaunchWindowPlannerData> data_; };
}
