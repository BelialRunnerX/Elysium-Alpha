#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate context-specific strategic war goals from claims, grievances, resources, threats, ideology, and history.
struct WarObjectiveGeneratorCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct WarObjectiveGeneratorState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class WarObjectiveGeneratorSystem { public: bool submit(const WarObjectiveGeneratorCommand&); const WarObjectiveGeneratorState* find(std::uint64_t) const; std::vector<WarObjectiveGeneratorState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,WarObjectiveGeneratorState> map_; };
}
