#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate and track lost-colony investigations with records, descendants, ruins, hazards, factions, and possible outcomes.
struct LostColonySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct LostColonySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class LostColonySystemStore { public: bool apply(const LostColonySystemOp&); bool erase(std::uint64_t); const LostColonySystemData* find(std::uint64_t) const; std::vector<LostColonySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,LostColonySystemData> data_; };
}
