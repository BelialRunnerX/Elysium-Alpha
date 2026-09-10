#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track generated comet orbits, volatile resources, hazards, mining opportunities, and temporary mission windows.
struct CometSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct CometSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class CometSystemSystem { public: bool submit(const CometSystemCommand&); const CometSystemState* find(std::uint64_t) const; std::vector<CometSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CometSystemState> map_; };
}
