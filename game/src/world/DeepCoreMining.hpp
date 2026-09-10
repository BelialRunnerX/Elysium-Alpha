#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent extreme-depth mining sites, heat, pressure, seismic risk, rare resources, lift logistics, and collapse hazards.
struct DeepCoreMiningCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct DeepCoreMiningState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class DeepCoreMiningSystem { public: bool submit(const DeepCoreMiningCommand&); const DeepCoreMiningState* find(std::uint64_t) const; std::vector<DeepCoreMiningState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DeepCoreMiningState> map_; };
}
