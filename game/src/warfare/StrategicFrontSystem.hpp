#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track sparse strategic fronts, control, supply, fortification, pressure, objectives, and theater-level state.
struct StrategicFrontSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct StrategicFrontSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class StrategicFrontSystemStore { public: bool apply(const StrategicFrontSystemOp&); bool erase(std::uint64_t); const StrategicFrontSystemData* find(std::uint64_t) const; std::vector<StrategicFrontSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StrategicFrontSystemData> data_; };
}
