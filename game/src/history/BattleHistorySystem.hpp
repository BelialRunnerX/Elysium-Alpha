#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Record battles with participants, commanders, forces, objectives, casualties, terrain, turning points, and consequences.
struct BattleHistorySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct BattleHistorySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class BattleHistorySystemStore { public: bool apply(const BattleHistorySystemOp&); bool erase(std::uint64_t); const BattleHistorySystemData* find(std::uint64_t) const; std::vector<BattleHistorySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BattleHistorySystemData> data_; };
}
