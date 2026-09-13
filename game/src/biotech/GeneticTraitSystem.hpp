#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent heritable genetic traits, compatibility, expression, mutation hooks, and species-specific bounds.
struct GeneticTraitSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct GeneticTraitSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class GeneticTraitSystemStore { public: bool apply(const GeneticTraitSystemOp&); bool erase(std::uint64_t); const GeneticTraitSystemData* find(std::uint64_t) const; std::vector<GeneticTraitSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,GeneticTraitSystemData> data_; };
}
