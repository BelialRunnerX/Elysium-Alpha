#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Store faction ground doctrine for squads, armor, drones, fortification, artillery, medevac, and objectives.
struct GroundDoctrineSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct GroundDoctrineSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class GroundDoctrineSystemStore { public: bool apply(const GroundDoctrineSystemOp&); bool erase(std::uint64_t); const GroundDoctrineSystemData* find(std::uint64_t) const; std::vector<GroundDoctrineSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,GroundDoctrineSystemData> data_; };
}
