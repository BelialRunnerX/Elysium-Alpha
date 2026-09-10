#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent client prediction intents and authoritative reconciliation metadata without owning gameplay truth.
struct CommandPredictionModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CommandPredictionModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CommandPredictionModelStore { public: bool apply(const CommandPredictionModelOp&); bool erase(std::uint64_t); const CommandPredictionModelData* find(std::uint64_t) const; std::vector<CommandPredictionModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CommandPredictionModelData> data_; };
}
