#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track candidates, offices, voting blocs, turnout, legitimacy, terms, and transition of elected authority.
struct ElectionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ElectionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ElectionSystemStore { public: bool apply(const ElectionSystemOp&); bool erase(std::uint64_t); const ElectionSystemData* find(std::uint64_t) const; std::vector<ElectionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ElectionSystemData> data_; };
}
