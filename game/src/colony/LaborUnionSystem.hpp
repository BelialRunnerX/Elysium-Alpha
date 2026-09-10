#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track worker organizations, membership, contracts, grievances, strikes, safety demands, and negotiation state.
struct LaborUnionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct LaborUnionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class LaborUnionSystemStore { public: bool apply(const LaborUnionSystemOp&); bool erase(std::uint64_t); const LaborUnionSystemData* find(std::uint64_t) const; std::vector<LaborUnionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,LaborUnionSystemData> data_; };
}
