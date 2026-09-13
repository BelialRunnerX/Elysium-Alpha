#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build atmosphere, climate, water, soil, biosphere, infrastructure, progress, cost, and risk projections for terraforming.
struct TerraformScreenModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct TerraformScreenModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class TerraformScreenModelStore { public: bool apply(const TerraformScreenModelOp&); bool erase(std::uint64_t); const TerraformScreenModelData* find(std::uint64_t) const; std::vector<TerraformScreenModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TerraformScreenModelData> data_; };
}
