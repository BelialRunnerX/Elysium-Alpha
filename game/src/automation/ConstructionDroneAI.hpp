#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate excavation, placement, welding, repair, hauling, and support tasks from construction work packages.
struct ConstructionDroneAICommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ConstructionDroneAIState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ConstructionDroneAISystem { public: bool submit(const ConstructionDroneAICommand&); const ConstructionDroneAIState* find(std::uint64_t) const; std::vector<ConstructionDroneAIState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ConstructionDroneAIState> map_; };
}
