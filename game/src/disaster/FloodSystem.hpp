#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track flooding from storms, rivers, reservoirs, failures, terrain flow, contamination, damage, and evacuation.
struct FloodSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FloodSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FloodSystemStore { public: bool apply(const FloodSystemOp&); bool erase(std::uint64_t); const FloodSystemData* find(std::uint64_t) const; std::vector<FloodSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FloodSystemData> data_; };
}
