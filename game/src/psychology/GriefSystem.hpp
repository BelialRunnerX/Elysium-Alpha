#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track grief from deaths, losses, exile, destroyed homes, failed missions, and relationship significance over time.
struct GriefSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct GriefSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class GriefSystemStore { public: bool apply(const GriefSystemOp&); bool erase(std::uint64_t); const GriefSystemData* find(std::uint64_t) const; std::vector<GriefSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,GriefSystemData> data_; };
}
