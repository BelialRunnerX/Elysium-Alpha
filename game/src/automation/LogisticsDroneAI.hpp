#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate pickup, delivery, recharge, queue, reroute, and failure intents from bounded logistics requests.
struct LogisticsDroneAICommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct LogisticsDroneAIState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class LogisticsDroneAISystem { public: bool submit(const LogisticsDroneAICommand&); const LogisticsDroneAIState* find(std::uint64_t) const; std::vector<LogisticsDroneAIState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,LogisticsDroneAIState> map_; };
}
