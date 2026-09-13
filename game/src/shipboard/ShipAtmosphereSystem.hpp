#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track bounded compartment atmosphere, leaks, vents, scrubbers, fires, decompression, and crew survival hooks.
struct ShipAtmosphereSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ShipAtmosphereSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ShipAtmosphereSystemStore { public: bool apply(const ShipAtmosphereSystemOp&); bool erase(std::uint64_t); const ShipAtmosphereSystemData* find(std::uint64_t) const; std::vector<ShipAtmosphereSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipAtmosphereSystemData> data_; };
}
