#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate purchase, production, salvage, and trade requests from shortages, projects, reserves, and strategic policy.
struct ProcurementPlannerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ProcurementPlannerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ProcurementPlannerStore { public: bool apply(const ProcurementPlannerOp&); bool erase(std::uint64_t); const ProcurementPlannerData* find(std::uint64_t) const; std::vector<ProcurementPlannerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ProcurementPlannerData> data_; };
}
