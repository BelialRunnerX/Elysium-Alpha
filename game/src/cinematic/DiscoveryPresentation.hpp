#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build presentation cues for first-contact, ruins, artifacts, species, planets, technologies, and Rift discoveries.
struct DiscoveryPresentationOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct DiscoveryPresentationData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class DiscoveryPresentationStore { public: bool apply(const DiscoveryPresentationOp&); bool erase(std::uint64_t); const DiscoveryPresentationData* find(std::uint64_t) const; std::vector<DiscoveryPresentationData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DiscoveryPresentationData> data_; };
}
