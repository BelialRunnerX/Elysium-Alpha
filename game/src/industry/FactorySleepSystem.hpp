#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate idle/offscreen production lines into deterministic coarse updates while preserving queues, inventories, and failures.
struct FactorySleepSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FactorySleepSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FactorySleepSystemStore { public: bool apply(const FactorySleepSystemOp&); bool erase(std::uint64_t); const FactorySleepSystemData* find(std::uint64_t) const; std::vector<FactorySleepSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FactorySleepSystemData> data_; };
}
