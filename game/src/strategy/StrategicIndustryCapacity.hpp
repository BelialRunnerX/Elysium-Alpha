#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate extractive, refining, fabrication, shipbuilding, repair, and military production capacity.
struct StrategicIndustryCapacityOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct StrategicIndustryCapacityData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class StrategicIndustryCapacityStore { public: bool apply(const StrategicIndustryCapacityOp&); bool erase(std::uint64_t); const StrategicIndustryCapacityData* find(std::uint64_t) const; std::vector<StrategicIndustryCapacityData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StrategicIndustryCapacityData> data_; };
}
