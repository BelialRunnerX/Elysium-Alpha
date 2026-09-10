#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track long-term motives such as family, ambition, duty, curiosity, wealth, revenge, faith, security, and belonging.
struct MotivationSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct MotivationSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class MotivationSystemStore { public: bool apply(const MotivationSystemOp&); bool erase(std::uint64_t); const MotivationSystemData* find(std::uint64_t) const; std::vector<MotivationSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MotivationSystemData> data_; };
}
