#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track declining populations, local extinction, genetic bottlenecks, protected reserves, reintroduction, and Chronicle events.
struct ExtinctionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ExtinctionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ExtinctionSystemStore { public: bool apply(const ExtinctionSystemOp&); bool erase(std::uint64_t); const ExtinctionSystemData* find(std::uint64_t) const; std::vector<ExtinctionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ExtinctionSystemData> data_; };
}
