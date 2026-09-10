#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build Register Actions, warrants, standing, suspicion, Praetor pressure, Imperial facilities, and campaign projections.
struct EmpireScreenModelCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct EmpireScreenModelState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class EmpireScreenModelSystem { public: bool submit(const EmpireScreenModelCommand&); const EmpireScreenModelState* find(std::uint64_t) const; std::vector<EmpireScreenModelState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,EmpireScreenModelState> map_; };
}
