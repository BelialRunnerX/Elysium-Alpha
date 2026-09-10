#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Resolve mod load ordering, dependencies, conflicts, optional features, and deterministic content registration sequence.
struct ModDependencyResolverOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ModDependencyResolverData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ModDependencyResolverStore { public: bool apply(const ModDependencyResolverOp&); bool erase(std::uint64_t); const ModDependencyResolverData* find(std::uint64_t) const; std::vector<ModDependencyResolverData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ModDependencyResolverData> data_; };
}
