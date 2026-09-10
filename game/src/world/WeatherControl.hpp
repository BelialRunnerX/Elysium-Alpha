#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent late-game local weather modification requests, power cost, atmospheric limits, hazards, and faction policy.
struct WeatherControlCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct WeatherControlState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class WeatherControlSystem { public: bool submit(const WeatherControlCommand&); const WeatherControlState* find(std::uint64_t) const; std::vector<WeatherControlState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,WeatherControlState> map_; };
}
