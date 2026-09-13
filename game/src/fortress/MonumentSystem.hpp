#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent statues, memorials, trophies, shrines, plaques, and named structures linked to Chronicle facts.
struct MonumentSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct MonumentSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class MonumentSystemStore { public: bool apply(const MonumentSystemOp&); bool erase(std::uint64_t); const MonumentSystemData* find(std::uint64_t) const; std::vector<MonumentSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MonumentSystemData> data_; };
}
