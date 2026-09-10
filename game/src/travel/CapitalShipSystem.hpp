#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track capital-ship sections, command, power, heat, weapons, hangars, crew, damage, and strategic readiness.
struct CapitalShipSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct CapitalShipSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class CapitalShipSystemSystem { public: bool submit(const CapitalShipSystemCommand&); const CapitalShipSystemState* find(std::uint64_t) const; std::vector<CapitalShipSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CapitalShipSystemState> map_; };
}
