#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track occupied settlements, resistance, collaboration, garrisons, law, logistics, legitimacy, and liberation risk.
struct OccupationSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct OccupationSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class OccupationSystemStore { public: bool apply(const OccupationSystemOp&); bool erase(std::uint64_t); const OccupationSystemData* find(std::uint64_t) const; std::vector<OccupationSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,OccupationSystemData> data_; };
}
