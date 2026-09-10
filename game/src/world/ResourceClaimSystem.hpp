#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track mining, salvage, agricultural, archaeological, and energy claims with owners, boundaries, rights, and disputes.
struct ResourceClaimSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ResourceClaimSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ResourceClaimSystemStore { public: bool apply(const ResourceClaimSystemOp&); bool erase(std::uint64_t); const ResourceClaimSystemData* find(std::uint64_t) const; std::vector<ResourceClaimSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ResourceClaimSystemData> data_; };
}
