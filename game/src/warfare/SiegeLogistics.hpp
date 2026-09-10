#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track ammunition, food, fuel, medicine, repair, morale, breaches, blockade, and relief capacity during sieges.
struct SiegeLogisticsOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SiegeLogisticsData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SiegeLogisticsStore { public: bool apply(const SiegeLogisticsOp&); bool erase(std::uint64_t); const SiegeLogisticsData* find(std::uint64_t) const; std::vector<SiegeLogisticsData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SiegeLogisticsData> data_; };
}
