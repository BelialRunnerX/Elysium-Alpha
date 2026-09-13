#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate eruptions, lava, ash, gases, heat, terrain change, resources, evacuation, and climate impacts.
struct VolcanicSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct VolcanicSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class VolcanicSystemStore { public: bool apply(const VolcanicSystemOp&); bool erase(std::uint64_t); const VolcanicSystemData* find(std::uint64_t) const; std::vector<VolcanicSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,VolcanicSystemData> data_; };
}
