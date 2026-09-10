#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent black-hole strategic objects, gravity hazards, accretion, navigation exclusions, and science opportunities.
struct BlackHoleSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct BlackHoleSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class BlackHoleSystemStore { public: bool apply(const BlackHoleSystemOp&); bool erase(std::uint64_t); const BlackHoleSystemData* find(std::uint64_t) const; std::vector<BlackHoleSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BlackHoleSystemData> data_; };
}
