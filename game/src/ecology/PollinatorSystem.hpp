#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track pollinator abundance, habitat, seasonality, crop dependencies, toxins, and ecosystem-service effects.
struct PollinatorSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct PollinatorSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class PollinatorSystemStore { public: bool apply(const PollinatorSystemOp&); bool erase(std::uint64_t); const PollinatorSystemData* find(std::uint64_t) const; std::vector<PollinatorSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PollinatorSystemData> data_; };
}
