#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track myths, legendary figures, creation stories, sacred sites, reinterpretations, and narrative references.
struct MythologySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct MythologySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class MythologySystemStore { public: bool apply(const MythologySystemOp&); bool erase(std::uint64_t); const MythologySystemData* find(std::uint64_t) const; std::vector<MythologySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MythologySystemData> data_; };
}
