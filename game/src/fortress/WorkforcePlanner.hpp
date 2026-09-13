#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Allocate labor pools across production, construction, logistics, defense, research, agriculture, and services.
struct WorkforcePlannerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct WorkforcePlannerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class WorkforcePlannerStore { public: bool apply(const WorkforcePlannerOp&); bool erase(std::uint64_t); const WorkforcePlannerData* find(std::uint64_t) const; std::vector<WorkforcePlannerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,WorkforcePlannerData> data_; };
}
