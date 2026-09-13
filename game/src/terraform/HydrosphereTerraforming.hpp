#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track water import, reservoirs, evaporation, freezing, routing, and global hydrosphere targets.
struct HydrosphereTerraformingOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct HydrosphereTerraformingData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class HydrosphereTerraformingStore { public: bool apply(const HydrosphereTerraformingOp&); bool erase(std::uint64_t); const HydrosphereTerraformingData* find(std::uint64_t) const; std::vector<HydrosphereTerraformingData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,HydrosphereTerraformingData> data_; };
}
