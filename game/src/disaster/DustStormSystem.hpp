#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate dust/sand storms affecting visibility, machinery, solar power, health, travel, and erosion.
struct DustStormSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct DustStormSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class DustStormSystemStore { public: bool apply(const DustStormSystemOp&); bool erase(std::uint64_t); const DustStormSystemData* find(std::uint64_t) const; std::vector<DustStormSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DustStormSystemData> data_; };
}
