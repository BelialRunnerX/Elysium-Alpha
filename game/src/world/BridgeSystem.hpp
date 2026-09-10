#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent constructed spans with endpoints, support, load limits, damage, traffic, and repair hooks.
struct BridgeSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct BridgeSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class BridgeSystemStore { public: bool apply(const BridgeSystemOp&); bool erase(std::uint64_t); const BridgeSystemData* find(std::uint64_t) const; std::vector<BridgeSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BridgeSystemData> data_; };
}
