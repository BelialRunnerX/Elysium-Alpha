#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build localized presentation cues for actor death, incapacitation, memorialization, loot, and Chronicle significance.
struct DeathPresentationOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct DeathPresentationData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class DeathPresentationStore { public: bool apply(const DeathPresentationOp&); bool erase(std::uint64_t); const DeathPresentationData* find(std::uint64_t) const; std::vector<DeathPresentationData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DeathPresentationData> data_; };
}
