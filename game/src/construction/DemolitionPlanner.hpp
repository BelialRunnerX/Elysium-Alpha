#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate safe demolition, salvage, evacuation, support removal, hazard isolation, and debris-handling intents.
struct DemolitionPlannerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct DemolitionPlannerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class DemolitionPlannerStore { public: bool apply(const DemolitionPlannerOp&); bool erase(std::uint64_t); const DemolitionPlannerData* find(std::uint64_t) const; std::vector<DemolitionPlannerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DemolitionPlannerData> data_; };
}
