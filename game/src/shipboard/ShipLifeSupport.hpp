#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track oxygen, water, food, waste, heat, scrubbers, reserves, recycler performance, and emergency consumption.
struct ShipLifeSupportOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ShipLifeSupportData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ShipLifeSupportStore { public: bool apply(const ShipLifeSupportOp&); bool erase(std::uint64_t); const ShipLifeSupportData* find(std::uint64_t) const; std::vector<ShipLifeSupportData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipLifeSupportData> data_; };
}
