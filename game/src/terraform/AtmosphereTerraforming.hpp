#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track planetary atmosphere composition targets, injection/removal projects, leakage, and long-term progress.
struct AtmosphereTerraformingOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct AtmosphereTerraformingData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class AtmosphereTerraformingStore { public: bool apply(const AtmosphereTerraformingOp&); bool erase(std::uint64_t); const AtmosphereTerraformingData* find(std::uint64_t) const; std::vector<AtmosphereTerraformingData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,AtmosphereTerraformingData> data_; };
}
