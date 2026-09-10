#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Promote selected historical events/people/artifacts into mutable legends with retellings, distortion, and cultural reach.
struct LegendSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct LegendSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class LegendSystemStore { public: bool apply(const LegendSystemOp&); bool erase(std::uint64_t); const LegendSystemData* find(std::uint64_t) const; std::vector<LegendSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,LegendSystemData> data_; };
}
