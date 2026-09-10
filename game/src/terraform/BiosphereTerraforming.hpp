#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track staged microbial, flora, fauna, soil, and biome introduction with compatibility and ecological risk.
struct BiosphereTerraformingOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct BiosphereTerraformingData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class BiosphereTerraformingStore { public: bool apply(const BiosphereTerraformingOp&); bool erase(std::uint64_t); const BiosphereTerraformingData* find(std::uint64_t) const; std::vector<BiosphereTerraformingData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BiosphereTerraformingData> data_; };
}
