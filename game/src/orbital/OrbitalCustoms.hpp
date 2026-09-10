#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent inspections, manifests, tariffs, contraband risk, quarantine, and faction jurisdiction at ports.
struct OrbitalCustomsOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct OrbitalCustomsData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class OrbitalCustomsStore { public: bool apply(const OrbitalCustomsOp&); bool erase(std::uint64_t); const OrbitalCustomsData* find(std::uint64_t) const; std::vector<OrbitalCustomsData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,OrbitalCustomsData> data_; };
}
