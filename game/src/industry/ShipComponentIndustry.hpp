#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent drives, reactors, radiators, armor, hull sections, avionics, weapons, life support, and ship-module manufacturing.
struct ShipComponentIndustryOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ShipComponentIndustryData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ShipComponentIndustryStore { public: bool apply(const ShipComponentIndustryOp&); bool erase(std::uint64_t); const ShipComponentIndustryData* find(std::uint64_t) const; std::vector<ShipComponentIndustryData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipComponentIndustryData> data_; };
}
