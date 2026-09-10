#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track vegetation/building fire fronts, wind, fuel, suppression, smoke, evacuation, ecology, and infrastructure damage.
struct WildfireSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct WildfireSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class WildfireSystemStore { public: bool apply(const WildfireSystemOp&); bool erase(std::uint64_t); const WildfireSystemData* find(std::uint64_t) const; std::vector<WildfireSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,WildfireSystemData> data_; };
}
