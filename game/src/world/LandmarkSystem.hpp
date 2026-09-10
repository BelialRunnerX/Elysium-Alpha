#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track stable natural/artificial landmarks used by navigation, exploration, quests, naming, claims, and Chronicle.
struct LandmarkSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct LandmarkSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class LandmarkSystemStore { public: bool apply(const LandmarkSystemOp&); bool erase(std::uint64_t); const LandmarkSystemData* find(std::uint64_t) const; std::vector<LandmarkSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,LandmarkSystemData> data_; };
}
