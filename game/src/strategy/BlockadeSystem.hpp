#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track blockade coverage, interception probability, smuggling, shortages, relief attempts, and diplomatic consequences.
struct BlockadeSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct BlockadeSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class BlockadeSystemSystem { public: bool submit(const BlockadeSystemCommand&); const BlockadeSystemState* find(std::uint64_t) const; std::vector<BlockadeSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BlockadeSystemState> map_; };
}
