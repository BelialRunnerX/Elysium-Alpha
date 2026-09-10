#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build production chains, bottlenecks, machine utilization, power, logistics, maintenance, quality, and expansion projections.
struct IndustryPlannerScreenModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct IndustryPlannerScreenModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class IndustryPlannerScreenModelStore { public: bool apply(const IndustryPlannerScreenModelOp&); bool erase(std::uint64_t); const IndustryPlannerScreenModelData* find(std::uint64_t) const; std::vector<IndustryPlannerScreenModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,IndustryPlannerScreenModelData> data_; };
}
