#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track command hierarchy and deterministic succession when commanders are absent, incapacitated, or reassigned.
struct FleetCommandSuccessionOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FleetCommandSuccessionData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FleetCommandSuccessionStore { public: bool apply(const FleetCommandSuccessionOp&); bool erase(std::uint64_t); const FleetCommandSuccessionData* find(std::uint64_t) const; std::vector<FleetCommandSuccessionData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FleetCommandSuccessionData> data_; };
}
