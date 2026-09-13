#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track orbital/space tugs, tow assignments, mass limits, salvage, docking assistance, and emergency rescue.
struct TugSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct TugSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class TugSystemSystem { public: bool submit(const TugSystemCommand&); const TugSystemState* find(std::uint64_t) const; std::vector<TugSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TugSystemState> map_; };
}
