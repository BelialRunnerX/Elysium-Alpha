#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Allocate civilization research capacity across technology domains, strategic needs, captured tech, and institutions.
struct StrategicResearchOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct StrategicResearchData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class StrategicResearchStore { public: bool apply(const StrategicResearchOp&); bool erase(std::uint64_t); const StrategicResearchData* find(std::uint64_t) const; std::vector<StrategicResearchData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StrategicResearchData> data_; };
}
