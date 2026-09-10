#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Record deterministic high-level command/event streams and compatibility metadata for future debugging/playback.
struct ReplayRecorderOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ReplayRecorderData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ReplayRecorderStore { public: bool apply(const ReplayRecorderOp&); bool erase(std::uint64_t); const ReplayRecorderData* find(std::uint64_t) const; std::vector<ReplayRecorderData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ReplayRecorderData> data_; };
}
