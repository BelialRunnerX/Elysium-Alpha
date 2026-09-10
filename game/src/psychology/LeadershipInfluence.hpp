#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track leader charisma, competence, legitimacy, fear, trust, communication, and local morale/behavior effects.
struct LeadershipInfluenceOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct LeadershipInfluenceData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class LeadershipInfluenceStore { public: bool apply(const LeadershipInfluenceOp&); bool erase(std::uint64_t); const LeadershipInfluenceData* find(std::uint64_t) const; std::vector<LeadershipInfluenceData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,LeadershipInfluenceData> data_; };
}
