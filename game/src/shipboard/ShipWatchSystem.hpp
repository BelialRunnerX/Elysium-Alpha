#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate deterministic bridge/engineering/security/medical watch rotations from crew qualifications and fatigue.
struct ShipWatchSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ShipWatchSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ShipWatchSystemStore { public: bool apply(const ShipWatchSystemOp&); bool erase(std::uint64_t); const ShipWatchSystemData* find(std::uint64_t) const; std::vector<ShipWatchSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipWatchSystemData> data_; };
}
