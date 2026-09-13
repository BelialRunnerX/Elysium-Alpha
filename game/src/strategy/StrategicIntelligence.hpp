#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate scouting, spy reports, sensor coverage, rumor confidence, and information age for AI planning.
struct StrategicIntelligenceOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct StrategicIntelligenceData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class StrategicIntelligenceStore { public: bool apply(const StrategicIntelligenceOp&); bool erase(std::uint64_t); const StrategicIntelligenceData* find(std::uint64_t) const; std::vector<StrategicIntelligenceData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StrategicIntelligenceData> data_; };
}
