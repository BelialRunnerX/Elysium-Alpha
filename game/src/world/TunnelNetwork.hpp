#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent authored underground connections, portals, ventilation, hazards, structural state, and transit metadata.
struct TunnelNetworkOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct TunnelNetworkData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class TunnelNetworkStore { public: bool apply(const TunnelNetworkOp&); bool erase(std::uint64_t); const TunnelNetworkData* find(std::uint64_t) const; std::vector<TunnelNetworkData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TunnelNetworkData> data_; };
}
