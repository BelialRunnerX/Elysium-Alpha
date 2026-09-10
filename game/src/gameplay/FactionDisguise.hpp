#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track disguise quality, credentials, equipment, behavior exposure, recognition risk, and inspection outcomes.
struct FactionDisguiseCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct FactionDisguiseState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class FactionDisguiseSystem { public: bool submit(const FactionDisguiseCommand&); const FactionDisguiseState* find(std::uint64_t) const; std::vector<FactionDisguiseState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FactionDisguiseState> map_; };
}
