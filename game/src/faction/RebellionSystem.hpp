#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track grievances, rebel cells, support, organization, demands, escalation, insurgency, and negotiated outcomes.
struct RebellionSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct RebellionSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class RebellionSystemSystem { public: bool submit(const RebellionSystemCommand&); const RebellionSystemState* find(std::uint64_t) const; std::vector<RebellionSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RebellionSystemState> map_; };
}
