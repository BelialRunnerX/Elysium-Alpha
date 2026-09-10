#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate developer-only setup intents for actors, items, hazards, structures, settlements, fleets, and strategic events.
struct ScenarioSpawnerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ScenarioSpawnerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ScenarioSpawnerStore { public: bool apply(const ScenarioSpawnerOp&); bool erase(std::uint64_t); const ScenarioSpawnerData* find(std::uint64_t) const; std::vector<ScenarioSpawnerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ScenarioSpawnerData> data_; };
}
