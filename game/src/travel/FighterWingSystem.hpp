#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track fighter wings, pilots, craft condition, formations, missions, readiness, replacements, and casualties.
struct FighterWingSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct FighterWingSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class FighterWingSystemSystem { public: bool submit(const FighterWingSystemCommand&); const FighterWingSystemState* find(std::uint64_t) const; std::vector<FighterWingSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FighterWingSystemState> map_; };
}
