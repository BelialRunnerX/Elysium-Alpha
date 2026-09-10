#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent candidate eligibility, campaigns, voting blocs, turnout, results, disputes, and succession hooks.
struct ElectionSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ElectionSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ElectionSystemSystem { public: bool submit(const ElectionSystemCommand&); const ElectionSystemState* find(std::uint64_t) const; std::vector<ElectionSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ElectionSystemState> map_; };
}
