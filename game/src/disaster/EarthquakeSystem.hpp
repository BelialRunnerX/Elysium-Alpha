#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate deterministic tectonic events with shaking, structural damage, landslides, utility failures, and aftershocks.
struct EarthquakeSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct EarthquakeSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class EarthquakeSystemStore { public: bool apply(const EarthquakeSystemOp&); bool erase(std::uint64_t); const EarthquakeSystemData* find(std::uint64_t) const; std::vector<EarthquakeSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,EarthquakeSystemData> data_; };
}
