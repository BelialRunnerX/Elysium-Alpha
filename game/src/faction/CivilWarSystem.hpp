#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent internal faction conflict with sides, claims, forces, settlements, diplomacy, objectives, and postwar settlement.
struct CivilWarSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct CivilWarSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class CivilWarSystemSystem { public: bool submit(const CivilWarSystemCommand&); const CivilWarSystemState* find(std::uint64_t) const; std::vector<CivilWarSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CivilWarSystemState> map_; };
}
