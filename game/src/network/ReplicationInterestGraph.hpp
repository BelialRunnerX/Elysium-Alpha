#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Compute future network interest sets from stable spatial shards, ownership, squad, UI inspection, and relevance.
struct ReplicationInterestGraphOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ReplicationInterestGraphData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ReplicationInterestGraphStore { public: bool apply(const ReplicationInterestGraphOp&); bool erase(std::uint64_t); const ReplicationInterestGraphData* find(std::uint64_t) const; std::vector<ReplicationInterestGraphData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ReplicationInterestGraphData> data_; };
}
