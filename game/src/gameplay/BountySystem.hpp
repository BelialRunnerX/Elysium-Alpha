#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track faction and settlement bounties, targets, crimes, evidence, rewards, expiry, and capture/death conditions.
struct BountySystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct BountySystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class BountySystemSystem { public: bool submit(const BountySystemCommand&); const BountySystemState* find(std::uint64_t) const; std::vector<BountySystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BountySystemState> map_; };
}
