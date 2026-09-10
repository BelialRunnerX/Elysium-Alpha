#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Manage modular sealed habitat pieces, interfaces, pressure zones, service ports, and construction state.
struct HabitatModuleSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct HabitatModuleSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class HabitatModuleSystemStore { public: bool apply(const HabitatModuleSystemOp&); bool erase(std::uint64_t); const HabitatModuleSystemData* find(std::uint64_t) const; std::vector<HabitatModuleSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,HabitatModuleSystemData> data_; };
}
