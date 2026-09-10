#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track search-and-rescue teams, readiness, vehicles, medical handoff, hazards, and mission prioritization.
struct RescueServiceOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RescueServiceData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RescueServiceStore { public: bool apply(const RescueServiceOp&); bool erase(std::uint64_t); const RescueServiceData* find(std::uint64_t) const; std::vector<RescueServiceData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RescueServiceData> data_; };
}
