#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track military occupation, resistance, compliance, administration, supply, legitimacy, and transition outcomes.
struct OccupationSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct OccupationSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class OccupationSystemSystem { public: bool submit(const OccupationSystemCommand&); const OccupationSystemState* find(std::uint64_t) const; std::vector<OccupationSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,OccupationSystemState> map_; };
}
