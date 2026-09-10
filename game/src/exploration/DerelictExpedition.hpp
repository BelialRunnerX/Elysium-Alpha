#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track boarding/exploration state through derelict ships/stations with compartments, hazards, power, loot, survivors, and story facts.
struct DerelictExpeditionOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct DerelictExpeditionData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class DerelictExpeditionStore { public: bool apply(const DerelictExpeditionOp&); bool erase(std::uint64_t); const DerelictExpeditionData* find(std::uint64_t) const; std::vector<DerelictExpeditionData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DerelictExpeditionData> data_; };
}
