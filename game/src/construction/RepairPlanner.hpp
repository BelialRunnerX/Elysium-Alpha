#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate repair work from damage, structural risk, system criticality, available materials, access, and workforce.
struct RepairPlannerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RepairPlannerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RepairPlannerStore { public: bool apply(const RepairPlannerOp&); bool erase(std::uint64_t); const RepairPlannerData* find(std::uint64_t) const; std::vector<RepairPlannerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RepairPlannerData> data_; };
}
