#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent discovered ancient transit gates, activation state, destination graph, hazards, ownership, and strategic value.
struct AncientGateNetworkCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct AncientGateNetworkState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class AncientGateNetworkSystem { public: bool submit(const AncientGateNetworkCommand&); const AncientGateNetworkState* find(std::uint64_t) const; std::vector<AncientGateNetworkState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,AncientGateNetworkState> map_; };
}
