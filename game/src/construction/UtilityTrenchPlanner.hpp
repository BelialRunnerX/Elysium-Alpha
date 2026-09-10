#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Plan bundled power, fluid, data, waste, heat, and atmosphere routes through sparse settlement space.
struct UtilityTrenchPlannerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct UtilityTrenchPlannerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class UtilityTrenchPlannerStore { public: bool apply(const UtilityTrenchPlannerOp&); bool erase(std::uint64_t); const UtilityTrenchPlannerData* find(std::uint64_t) const; std::vector<UtilityTrenchPlannerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,UtilityTrenchPlannerData> data_; };
}
