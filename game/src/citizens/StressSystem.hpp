#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track acute and chronic stress from danger, overwork, trauma, conflict, isolation, illness, and uncertainty.
struct StressSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct StressSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class StressSystemStore { public: bool apply(const StressSystemOp&); bool erase(std::uint64_t); const StressSystemData* find(std::uint64_t) const; std::vector<StressSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StressSystemData> data_; };
}
