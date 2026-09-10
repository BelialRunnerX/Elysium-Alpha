#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track family/dynastic continuity, offices, property, reputation, alliances, feuds, inheritance, and historical significance.
struct DynastySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct DynastySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class DynastySystemStore { public: bool apply(const DynastySystemOp&); bool erase(std::uint64_t); const DynastySystemData* find(std::uint64_t) const; std::vector<DynastySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DynastySystemData> data_; };
}
