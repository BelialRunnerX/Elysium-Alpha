#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate cave chambers, micro-biomes, hazards, resources, water, organisms, and ancient features from chunk seeds.
struct CaveBiomeGeneratorOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CaveBiomeGeneratorData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CaveBiomeGeneratorStore { public: bool apply(const CaveBiomeGeneratorOp&); bool erase(std::uint64_t); const CaveBiomeGeneratorData* find(std::uint64_t) const; std::vector<CaveBiomeGeneratorData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CaveBiomeGeneratorData> data_; };
}
