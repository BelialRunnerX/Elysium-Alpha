#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate linked multi-stage quests with alternative branches, faction choices, world-state gates, and Chronicle significance.
struct QuestChainGeneratorOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct QuestChainGeneratorData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class QuestChainGeneratorStore { public: bool apply(const QuestChainGeneratorOp&); bool erase(std::uint64_t); const QuestChainGeneratorData* find(std::uint64_t) const; std::vector<QuestChainGeneratorData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,QuestChainGeneratorData> data_; };
}
