#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track bounded air/water/soil pollution sources, spread summaries, cleanup, health effects, and faction/civic consequences.
struct PollutionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct PollutionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class PollutionSystemStore { public: bool apply(const PollutionSystemOp&); bool erase(std::uint64_t); const PollutionSystemData* find(std::uint64_t) const; std::vector<PollutionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PollutionSystemData> data_; };
}
