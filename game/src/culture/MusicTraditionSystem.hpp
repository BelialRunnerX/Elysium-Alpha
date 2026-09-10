#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track musical traditions, instruments, genres, performers, events, diffusion, and cultural-memory hooks.
struct MusicTraditionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct MusicTraditionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class MusicTraditionSystemStore { public: bool apply(const MusicTraditionSystemOp&); bool erase(std::uint64_t); const MusicTraditionSystemData* find(std::uint64_t) const; std::vector<MusicTraditionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MusicTraditionSystemData> data_; };
}
