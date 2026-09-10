#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track chronic overwork, low control, poor recovery, trauma, social support, role mismatch, and burnout recovery.
struct BurnoutSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct BurnoutSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class BurnoutSystemStore { public: bool apply(const BurnoutSystemOp&); bool erase(std::uint64_t); const BurnoutSystemData* find(std::uint64_t) const; std::vector<BurnoutSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BurnoutSystemData> data_; };
}
