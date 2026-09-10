#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Translate recent needs, memories, environment, social interactions, health, and events into bounded mood state.
struct MoodSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct MoodSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class MoodSystemStore { public: bool apply(const MoodSystemOp&); bool erase(std::uint64_t); const MoodSystemData* find(std::uint64_t) const; std::vector<MoodSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MoodSystemData> data_; };
}
