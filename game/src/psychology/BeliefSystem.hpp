#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track citizen beliefs, values, ideologies, taboos, rituals, identity, conviction, and change from experience/social influence.
struct BeliefSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct BeliefSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class BeliefSystemStore { public: bool apply(const BeliefSystemOp&); bool erase(std::uint64_t); const BeliefSystemData* find(std::uint64_t) const; std::vector<BeliefSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BeliefSystemData> data_; };
}
