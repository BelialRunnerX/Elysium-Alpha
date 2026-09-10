#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Evaluate machine placement, logistics distance, utilities, hazards, maintenance access, throughput, and expansion space.
struct FactoryLayoutPlannerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FactoryLayoutPlannerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FactoryLayoutPlannerStore { public: bool apply(const FactoryLayoutPlannerOp&); bool erase(std::uint64_t); const FactoryLayoutPlannerData* find(std::uint64_t) const; std::vector<FactoryLayoutPlannerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FactoryLayoutPlannerData> data_; };
}
