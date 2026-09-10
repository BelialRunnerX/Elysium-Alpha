#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate settlement crises from shortages, politics, disasters, disease, raids, accidents, and infrastructure failures.
struct ColonyCrisisGeneratorCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ColonyCrisisGeneratorState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ColonyCrisisGeneratorSystem { public: bool submit(const ColonyCrisisGeneratorCommand&); const ColonyCrisisGeneratorState* find(std::uint64_t) const; std::vector<ColonyCrisisGeneratorState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ColonyCrisisGeneratorState> map_; };
}
