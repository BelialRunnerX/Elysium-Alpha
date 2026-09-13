#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track plots, alliances, blackmail, leaks, sabotage, promotions, scandals, and investigation hooks.
struct CourtIntrigueSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CourtIntrigueSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CourtIntrigueSystemStore { public: bool apply(const CourtIntrigueSystemOp&); bool erase(std::uint64_t); const CourtIntrigueSystemData* find(std::uint64_t) const; std::vector<CourtIntrigueSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CourtIntrigueSystemData> data_; };
}
