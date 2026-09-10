#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate systems into regional administration with budgets, fleets, trade policy, infrastructure, and crisis response.
struct RegionalGovernmentOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RegionalGovernmentData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RegionalGovernmentStore { public: bool apply(const RegionalGovernmentOp&); bool erase(std::uint64_t); const RegionalGovernmentData* find(std::uint64_t) const; std::vector<RegionalGovernmentData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RegionalGovernmentData> data_; };
}
