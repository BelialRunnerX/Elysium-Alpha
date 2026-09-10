#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Coordinate refinery chains, input buffers, catalysts, byproducts, heat, maintenance, and output routing across stable machines.
struct RefineryNetworkOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RefineryNetworkData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RefineryNetworkStore { public: bool apply(const RefineryNetworkOp&); bool erase(std::uint64_t); const RefineryNetworkData* find(std::uint64_t) const; std::vector<RefineryNetworkData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RefineryNetworkData> data_; };
}
