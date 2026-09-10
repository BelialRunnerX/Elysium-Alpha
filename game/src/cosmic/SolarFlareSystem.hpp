#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate deterministic flare events with severity, warning, radiation, comms, grid, and orbital effects.
struct SolarFlareSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SolarFlareSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SolarFlareSystemStore { public: bool apply(const SolarFlareSystemOp&); bool erase(std::uint64_t); const SolarFlareSystemData* find(std::uint64_t) const; std::vector<SolarFlareSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SolarFlareSystemData> data_; };
}
