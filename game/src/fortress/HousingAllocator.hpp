#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Assign available rooms/beds to citizens and households using privacy, quality, location, role, and policy.
struct HousingAllocatorOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct HousingAllocatorData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class HousingAllocatorStore { public: bool apply(const HousingAllocatorOp&); bool erase(std::uint64_t); const HousingAllocatorData* find(std::uint64_t) const; std::vector<HousingAllocatorData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,HousingAllocatorData> data_; };
}
