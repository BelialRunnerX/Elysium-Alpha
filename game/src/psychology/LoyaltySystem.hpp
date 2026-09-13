#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track loyalty to people, settlements, factions, institutions, squads, causes, and competing obligations.
struct LoyaltySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct LoyaltySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class LoyaltySystemStore { public: bool apply(const LoyaltySystemOp&); bool erase(std::uint64_t); const LoyaltySystemData* find(std::uint64_t) const; std::vector<LoyaltySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,LoyaltySystemData> data_; };
}
