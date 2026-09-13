#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track damage-control teams, patches, firefighting, isolation, power rerouting, flooding/gas, and repair priorities.
struct ShipDamageControlOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ShipDamageControlData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ShipDamageControlStore { public: bool apply(const ShipDamageControlOp&); bool erase(std::uint64_t); const ShipDamageControlData* find(std::uint64_t) const; std::vector<ShipDamageControlData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipDamageControlData> data_; };
}
