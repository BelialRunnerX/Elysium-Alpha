#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track jamming, spoofing, sensor degradation, communications interference, countermeasures, and cyber intrusion hooks.
struct ElectronicWarfareCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ElectronicWarfareState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ElectronicWarfareSystem { public: bool submit(const ElectronicWarfareCommand&); const ElectronicWarfareState* find(std::uint64_t) const; std::vector<ElectronicWarfareState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ElectronicWarfareState> map_; };
}
