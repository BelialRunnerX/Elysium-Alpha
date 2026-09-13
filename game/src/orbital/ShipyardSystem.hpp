#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Schedule ship construction, refit, repair, module installation, testing, crew provisioning, and launch readiness.
struct ShipyardSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ShipyardSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ShipyardSystemStore { public: bool apply(const ShipyardSystemOp&); bool erase(std::uint64_t); const ShipyardSystemData* find(std::uint64_t) const; std::vector<ShipyardSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipyardSystemData> data_; };
}
