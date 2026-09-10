#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build deterministic daily schedules from shifts, sleep, meals, treatment, recreation, duties, and emergencies.
struct ScheduleSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ScheduleSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ScheduleSystemStore { public: bool apply(const ScheduleSystemOp&); bool erase(std::uint64_t); const ScheduleSystemData* find(std::uint64_t) const; std::vector<ScheduleSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ScheduleSystemData> data_; };
}
