#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Allocate food, water, medicine, fuel, ammunition, and emergency reserves by configurable settlement priorities.
struct RationPolicyOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RationPolicyData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RationPolicyStore { public: bool apply(const RationPolicyOp&); bool erase(std::uint64_t); const RationPolicyData* find(std::uint64_t) const; std::vector<RationPolicyData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RationPolicyData> data_; };
}
