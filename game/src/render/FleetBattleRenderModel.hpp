#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build renderer-neutral fleet formations, weapon fire, missiles, shields, damage, debris, and tactical marker records.
struct FleetBattleRenderModelCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct FleetBattleRenderModelState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class FleetBattleRenderModelSystem { public: bool submit(const FleetBattleRenderModelCommand&); const FleetBattleRenderModelState* find(std::uint64_t) const; std::vector<FleetBattleRenderModelState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FleetBattleRenderModelState> map_; };
}
