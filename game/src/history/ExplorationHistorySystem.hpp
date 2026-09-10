#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track expeditions, mapped regions, first contacts, discoveries, lost crews, artifacts, and frontier expansion.
struct ExplorationHistorySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ExplorationHistorySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ExplorationHistorySystemStore { public: bool apply(const ExplorationHistorySystemOp&); bool erase(std::uint64_t); const ExplorationHistorySystemData* find(std::uint64_t) const; std::vector<ExplorationHistorySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ExplorationHistorySystemData> data_; };
}
