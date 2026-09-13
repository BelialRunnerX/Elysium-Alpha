#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track discoveries, inventors, institutions, patents/ownership hooks, diffusion, accidents, and strategic impact.
struct TechnologyHistorySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct TechnologyHistorySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class TechnologyHistorySystemStore { public: bool apply(const TechnologyHistorySystemOp&); bool erase(std::uint64_t); const TechnologyHistorySystemData* find(std::uint64_t) const; std::vector<TechnologyHistorySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TechnologyHistorySystemData> data_; };
}
