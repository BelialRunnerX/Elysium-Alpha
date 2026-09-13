#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track important routes, commodities, merchant houses, booms, shortages, embargoes, smuggling, and market crises.
struct TradeHistorySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct TradeHistorySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class TradeHistorySystemStore { public: bool apply(const TradeHistorySystemOp&); bool erase(std::uint64_t); const TradeHistorySystemData* find(std::uint64_t) const; std::vector<TradeHistorySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TradeHistorySystemData> data_; };
}
