#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track soil fertility, moisture, microbes, organic matter, salinity, toxins, erosion, and plant-support capacity.
struct SoilEcologyOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SoilEcologyData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SoilEcologyStore { public: bool apply(const SoilEcologyOp&); bool erase(std::uint64_t); const SoilEcologyData* find(std::uint64_t) const; std::vector<SoilEcologyData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SoilEcologyData> data_; };
}
