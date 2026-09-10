#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track long underground expeditions with navigation breadcrumbs, atmosphere, supplies, hazards, discoveries, and rescue hooks.
struct DeepCaveExpeditionOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct DeepCaveExpeditionData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class DeepCaveExpeditionStore { public: bool apply(const DeepCaveExpeditionOp&); bool erase(std::uint64_t); const DeepCaveExpeditionData* find(std::uint64_t) const; std::vector<DeepCaveExpeditionData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DeepCaveExpeditionData> data_; };
}
