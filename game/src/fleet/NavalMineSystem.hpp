#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track deployable space mines, fields, identification, ownership, friend-or-foe policy, sweeping, and detonation events.
struct NavalMineSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct NavalMineSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class NavalMineSystemStore { public: bool apply(const NavalMineSystemOp&); bool erase(std::uint64_t); const NavalMineSystemData* find(std::uint64_t) const; std::vector<NavalMineSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,NavalMineSystemData> data_; };
}
