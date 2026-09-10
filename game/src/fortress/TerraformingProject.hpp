#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Coordinate atmosphere, ocean, temperature, soil, ecology, infrastructure, and civilization-scale terraforming milestones.
struct TerraformingProjectCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct TerraformingProjectState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class TerraformingProjectSystem { public: bool submit(const TerraformingProjectCommand&); const TerraformingProjectState* find(std::uint64_t) const; std::vector<TerraformingProjectState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TerraformingProjectState> map_; };
}
