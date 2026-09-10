#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track interpersonal disputes, escalation, mediation, violence risk, grudges, and justice hooks.
struct ConflictSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ConflictSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ConflictSystemStore { public: bool apply(const ConflictSystemOp&); bool erase(std::uint64_t); const ConflictSystemData* find(std::uint64_t) const; std::vector<ConflictSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ConflictSystemData> data_; };
}
