#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track favor/debt/prestige across Imperial offices and personalities without replacing broader diplomacy standing.
struct CourtFavorSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CourtFavorSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CourtFavorSystemStore { public: bool apply(const CourtFavorSystemOp&); bool erase(std::uint64_t); const CourtFavorSystemData* find(std::uint64_t) const; std::vector<CourtFavorSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CourtFavorSystemData> data_; };
}
