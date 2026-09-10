#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate severe cold/snow events affecting travel, power demand, exposure, visibility, structures, and logistics.
struct BlizzardSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct BlizzardSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class BlizzardSystemStore { public: bool apply(const BlizzardSystemOp&); bool erase(std::uint64_t); const BlizzardSystemData* find(std::uint64_t) const; std::vector<BlizzardSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BlizzardSystemData> data_; };
}
