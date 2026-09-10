#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Translate squad objectives into formation, movement, cover, targeting, breach, rescue, and retreat tactical intents.
struct SquadCommandAIOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SquadCommandAIData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SquadCommandAIStore { public: bool apply(const SquadCommandAIOp&); bool erase(std::uint64_t); const SquadCommandAIData* find(std::uint64_t) const; std::vector<SquadCommandAIData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SquadCommandAIData> data_; };
}
