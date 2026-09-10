#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track faction messaging campaigns, audiences, credibility, media channels, counter-messaging, and opinion shifts.
struct PropagandaSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct PropagandaSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class PropagandaSystemSystem { public: bool submit(const PropagandaSystemCommand&); const PropagandaSystemState* find(std::uint64_t) const; std::vector<PropagandaSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PropagandaSystemState> map_; };
}
