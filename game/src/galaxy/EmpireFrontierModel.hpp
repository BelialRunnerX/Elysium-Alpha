#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track Imperial influence, patrol reach, registry pressure, infrastructure, resistance, and frontier permeability.
struct EmpireFrontierModelCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct EmpireFrontierModelState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class EmpireFrontierModelSystem { public: bool submit(const EmpireFrontierModelCommand&); const EmpireFrontierModelState* find(std::uint64_t) const; std::vector<EmpireFrontierModelState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,EmpireFrontierModelState> map_; };
}
