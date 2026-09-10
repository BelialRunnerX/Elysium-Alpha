#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent persistent trauma triggers, coping, treatment, relapse, and behavior modifiers from severe events.
struct TraumaSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct TraumaSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class TraumaSystemStore { public: bool apply(const TraumaSystemOp&); bool erase(std::uint64_t); const TraumaSystemData* find(std::uint64_t) const; std::vector<TraumaSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TraumaSystemData> data_; };
}
