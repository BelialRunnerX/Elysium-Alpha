#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track player-authored terrain grading and leveling requests as touched-chunk deltas without global terrain rewrites.
struct SurfaceGradingSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SurfaceGradingSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SurfaceGradingSystemStore { public: bool apply(const SurfaceGradingSystemOp&); bool erase(std::uint64_t); const SurfaceGradingSystemData* find(std::uint64_t) const; std::vector<SurfaceGradingSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SurfaceGradingSystemData> data_; };
}
