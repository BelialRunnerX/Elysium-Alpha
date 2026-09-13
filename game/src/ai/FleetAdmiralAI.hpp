#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate fleet formations, patrols, escort, blockade, retreat, resupply, and engagement decisions.
struct FleetAdmiralAIOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FleetAdmiralAIData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FleetAdmiralAIStore { public: bool apply(const FleetAdmiralAIOp&); bool erase(std::uint64_t); const FleetAdmiralAIData* find(std::uint64_t) const; std::vector<FleetAdmiralAIData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FleetAdmiralAIData> data_; };
}
