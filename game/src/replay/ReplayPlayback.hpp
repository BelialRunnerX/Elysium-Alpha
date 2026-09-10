#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Feed recorded deterministic commands back into simulation while exposing divergence checkpoints and stop conditions.
struct ReplayPlaybackOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ReplayPlaybackData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ReplayPlaybackStore { public: bool apply(const ReplayPlaybackOp&); bool erase(std::uint64_t); const ReplayPlaybackData* find(std::uint64_t) const; std::vector<ReplayPlaybackData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ReplayPlaybackData> data_; };
}
