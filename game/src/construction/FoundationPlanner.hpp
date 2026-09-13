#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Plan stable-address foundations, grading, supports, and staged construction prerequisites for large structures.
struct FoundationPlannerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FoundationPlannerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FoundationPlannerStore { public: bool apply(const FoundationPlannerOp&); bool erase(std::uint64_t); const FoundationPlannerData* find(std::uint64_t) const; std::vector<FoundationPlannerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FoundationPlannerData> data_; };
}
