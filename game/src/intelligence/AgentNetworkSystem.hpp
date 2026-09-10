#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track covert agents, handlers, covers, assignments, risk, intel quality, compromise, and extraction.
struct AgentNetworkSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct AgentNetworkSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class AgentNetworkSystemStore { public: bool apply(const AgentNetworkSystemOp&); bool erase(std::uint64_t); const AgentNetworkSystemData* find(std::uint64_t) const; std::vector<AgentNetworkSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,AgentNetworkSystemData> data_; };
}
