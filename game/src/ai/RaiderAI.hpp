#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate target selection, scouting, intimidation, attack, looting, kidnapping, retreat, and hideout behavior.
struct RaiderAIOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RaiderAIData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RaiderAIStore { public: bool apply(const RaiderAIOp&); bool erase(std::uint64_t); const RaiderAIData* find(std::uint64_t) const; std::vector<RaiderAIData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RaiderAIData> data_; };
}
