#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track post-battle wreck claims, recovery teams, hazards, parts, survivors, intelligence, and legal ownership.
struct FleetSalvageSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FleetSalvageSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FleetSalvageSystemStore { public: bool apply(const FleetSalvageSystemOp&); bool erase(std::uint64_t); const FleetSalvageSystemData* find(std::uint64_t) const; std::vector<FleetSalvageSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FleetSalvageSystemData> data_; };
}
