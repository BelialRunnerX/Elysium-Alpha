#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent neutron-star radiation, magnetic hazards, pulsar timing, resources, and navigation/science hooks.
struct NeutronStarSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct NeutronStarSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class NeutronStarSystemStore { public: bool apply(const NeutronStarSystemOp&); bool erase(std::uint64_t); const NeutronStarSystemData* find(std::uint64_t) const; std::vector<NeutronStarSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,NeutronStarSystemData> data_; };
}
