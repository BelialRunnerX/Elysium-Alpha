#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate civilian, military, trade, and industrial traffic around worlds and stations for strategic simulation.
struct SpaceTrafficSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SpaceTrafficSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SpaceTrafficSystemStore { public: bool apply(const SpaceTrafficSystemOp&); bool erase(std::uint64_t); const SpaceTrafficSystemData* find(std::uint64_t) const; std::vector<SpaceTrafficSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SpaceTrafficSystemData> data_; };
}
