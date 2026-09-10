#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track introduced species establishment, spread, competition, predation, control projects, and ecological consequences.
struct InvasiveSpeciesSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct InvasiveSpeciesSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class InvasiveSpeciesSystemStore { public: bool apply(const InvasiveSpeciesSystemOp&); bool erase(std::uint64_t); const InvasiveSpeciesSystemData* find(std::uint64_t) const; std::vector<InvasiveSpeciesSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,InvasiveSpeciesSystemData> data_; };
}
