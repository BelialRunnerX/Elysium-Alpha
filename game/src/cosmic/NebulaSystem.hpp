#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent nebula volumes with sensor attenuation, resources, weather-like space hazards, and exploration content.
struct NebulaSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct NebulaSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class NebulaSystemStore { public: bool apply(const NebulaSystemOp&); bool erase(std::uint64_t); const NebulaSystemData* find(std::uint64_t) const; std::vector<NebulaSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,NebulaSystemData> data_; };
}
