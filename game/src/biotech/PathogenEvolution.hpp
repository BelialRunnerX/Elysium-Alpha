#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track bounded pathogen strains, mutations, transmission traits, immunity pressure, and outbreak lineage.
struct PathogenEvolutionOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct PathogenEvolutionData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class PathogenEvolutionStore { public: bool apply(const PathogenEvolutionOp&); bool erase(std::uint64_t); const PathogenEvolutionData* find(std::uint64_t) const; std::vector<PathogenEvolutionData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PathogenEvolutionData> data_; };
}
