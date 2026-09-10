#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track fire stations, crews, apparatus, coverage, dispatch, water/foam supply, rescue, and recovery.
struct FireDepartmentOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FireDepartmentData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FireDepartmentStore { public: bool apply(const FireDepartmentOp&); bool erase(std::uint64_t); const FireDepartmentData* find(std::uint64_t) const; std::vector<FireDepartmentData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FireDepartmentData> data_; };
}
