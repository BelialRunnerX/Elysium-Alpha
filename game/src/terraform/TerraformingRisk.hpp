#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate runaway greenhouse, ecosystem collapse, invasive species, toxicity, and infrastructure failure risk.
struct TerraformingRiskOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct TerraformingRiskData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class TerraformingRiskStore { public: bool apply(const TerraformingRiskOp&); bool erase(std::uint64_t); const TerraformingRiskData* find(std::uint64_t) const; std::vector<TerraformingRiskData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TerraformingRiskData> data_; };
}
