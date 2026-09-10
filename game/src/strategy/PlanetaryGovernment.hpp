#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate multiple settlements, stations, claims, orbital assets, laws, and strategic priorities under planetary authority.
struct PlanetaryGovernmentOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct PlanetaryGovernmentData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class PlanetaryGovernmentStore { public: bool apply(const PlanetaryGovernmentOp&); bool erase(std::uint64_t); const PlanetaryGovernmentData* find(std::uint64_t) const; std::vector<PlanetaryGovernmentData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PlanetaryGovernmentData> data_; };
}
