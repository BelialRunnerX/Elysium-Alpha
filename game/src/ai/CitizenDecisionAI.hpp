#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Select bounded citizen goals from needs, jobs, schedules, relationships, emergencies, and personal priorities.
struct CitizenDecisionAIOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CitizenDecisionAIData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CitizenDecisionAIStore { public: bool apply(const CitizenDecisionAIOp&); bool erase(std::uint64_t); const CitizenDecisionAIData* find(std::uint64_t) const; std::vector<CitizenDecisionAIData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CitizenDecisionAIData> data_; };
}
