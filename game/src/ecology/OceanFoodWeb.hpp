#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track aggregate marine primary production, grazers, predators, fisheries, pollution, temperature, and collapse risk.
struct OceanFoodWebOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct OceanFoodWebData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class OceanFoodWebStore { public: bool apply(const OceanFoodWebOp&); bool erase(std::uint64_t); const OceanFoodWebData* find(std::uint64_t) const; std::vector<OceanFoodWebData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,OceanFoodWebData> data_; };
}
