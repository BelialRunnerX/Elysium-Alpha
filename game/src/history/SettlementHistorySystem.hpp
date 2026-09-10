#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate founding, growth, disasters, governments, migrations, industries, wars, discoveries, and landmark changes.
struct SettlementHistorySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SettlementHistorySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SettlementHistorySystemStore { public: bool apply(const SettlementHistorySystemOp&); bool erase(std::uint64_t); const SettlementHistorySystemData* find(std::uint64_t) const; std::vector<SettlementHistorySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SettlementHistorySystemData> data_; };
}
