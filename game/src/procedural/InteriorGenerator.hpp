#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate room graphs, doors, utilities, furniture anchors, damage, loot, and encounter hooks for authored structures.
struct InteriorGeneratorOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct InteriorGeneratorData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class InteriorGeneratorStore { public: bool apply(const InteriorGeneratorOp&); bool erase(std::uint64_t); const InteriorGeneratorData* find(std::uint64_t) const; std::vector<InteriorGeneratorData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,InteriorGeneratorData> data_; };
}
