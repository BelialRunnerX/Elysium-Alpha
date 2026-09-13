#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track wreck fields, salvage claims, hazards, tug operations, recovery manifests, and ownership transfer.
struct OrbitalSalvageOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct OrbitalSalvageData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class OrbitalSalvageStore { public: bool apply(const OrbitalSalvageOp&); bool erase(std::uint64_t); const OrbitalSalvageData* find(std::uint64_t) const; std::vector<OrbitalSalvageData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,OrbitalSalvageData> data_; };
}
