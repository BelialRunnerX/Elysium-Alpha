#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track local machinery, combat, transit, crowd, and construction noise for citizen and wildlife simulation.
struct NoiseSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct NoiseSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class NoiseSystemStore { public: bool apply(const NoiseSystemOp&); bool erase(std::uint64_t); const NoiseSystemData* find(std::uint64_t) const; std::vector<NoiseSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,NoiseSystemData> data_; };
}
