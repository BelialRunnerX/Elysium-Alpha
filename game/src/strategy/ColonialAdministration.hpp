#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track colony charters, autonomy, taxation, defense obligations, governors, migration, and parent-faction relations.
struct ColonialAdministrationOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ColonialAdministrationData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ColonialAdministrationStore { public: bool apply(const ColonialAdministrationOp&); bool erase(std::uint64_t); const ColonialAdministrationData* find(std::uint64_t) const; std::vector<ColonialAdministrationData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ColonialAdministrationData> data_; };
}
