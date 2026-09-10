#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Schedule ceremonies, festivals, memorials, victories, discoveries, and cultural events with morale/history effects.
struct CelebrationSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CelebrationSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CelebrationSystemStore { public: bool apply(const CelebrationSystemOp&); bool erase(std::uint64_t); const CelebrationSystemData* find(std::uint64_t) const; std::vector<CelebrationSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CelebrationSystemData> data_; };
}
