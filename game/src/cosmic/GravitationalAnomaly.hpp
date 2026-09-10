#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent anomalous gravity regions affecting travel, physics presentation, sensors, and procedural narrative.
struct GravitationalAnomalyOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct GravitationalAnomalyData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class GravitationalAnomalyStore { public: bool apply(const GravitationalAnomalyOp&); bool erase(std::uint64_t); const GravitationalAnomalyData* find(std::uint64_t) const; std::vector<GravitationalAnomalyData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,GravitationalAnomalyData> data_; };
}
