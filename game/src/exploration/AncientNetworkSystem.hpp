#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track discovered ancient relays/gates/sites, activation prerequisites, links, hazards, ownership, and progressive understanding.
struct AncientNetworkSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct AncientNetworkSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class AncientNetworkSystemStore { public: bool apply(const AncientNetworkSystemOp&); bool erase(std::uint64_t); const AncientNetworkSystemData* find(std::uint64_t) const; std::vector<AncientNetworkSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,AncientNetworkSystemData> data_; };
}
