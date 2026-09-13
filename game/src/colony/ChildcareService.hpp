#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track dependents, caregivers, facilities, schedules, safety, family workload relief, and emergency coverage.
struct ChildcareServiceOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ChildcareServiceData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ChildcareServiceStore { public: bool apply(const ChildcareServiceOp&); bool erase(std::uint64_t); const ChildcareServiceData* find(std::uint64_t) const; std::vector<ChildcareServiceData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ChildcareServiceData> data_; };
}
