#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Score stay, migrate, flee, return, or retire decisions from safety, opportunity, relationships, ideology, and services.
struct MigrationDecisionOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct MigrationDecisionData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class MigrationDecisionStore { public: bool apply(const MigrationDecisionOp&); bool erase(std::uint64_t); const MigrationDecisionData* find(std::uint64_t) const; std::vector<MigrationDecisionData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MigrationDecisionData> data_; };
}
