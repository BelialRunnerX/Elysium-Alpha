#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate casualties, duration, economic cost, shortages, propaganda, victories, and citizen/faction willingness to continue war.
struct WarWearinessOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct WarWearinessData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class WarWearinessStore { public: bool apply(const WarWearinessOp&); bool erase(std::uint64_t); const WarWearinessData* find(std::uint64_t) const; std::vector<WarWearinessData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,WarWearinessData> data_; };
}
