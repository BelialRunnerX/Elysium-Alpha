#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate nebula regions with visibility, sensors, radiation, resources, navigation, and settlement modifiers.
struct NebulaSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct NebulaSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class NebulaSystemSystem { public: bool submit(const NebulaSystemCommand&); const NebulaSystemState* find(std::uint64_t) const; std::vector<NebulaSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,NebulaSystemState> map_; };
}
