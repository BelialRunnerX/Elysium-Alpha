#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Group citizens into households with shared rooms, dependents, resources, schedules, and migration decisions.
struct HouseholdSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct HouseholdSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class HouseholdSystemStore { public: bool apply(const HouseholdSystemOp&); bool erase(std::uint64_t); const HouseholdSystemData* find(std::uint64_t) const; std::vector<HouseholdSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,HouseholdSystemData> data_; };
}
