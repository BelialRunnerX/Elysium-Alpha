#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build renderer-neutral overlays for claims, routes, resources, hazards, weather, factions, utilities, and missions.
struct MapOverlayModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct MapOverlayModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class MapOverlayModelStore { public: bool apply(const MapOverlayModelOp&); bool erase(std::uint64_t); const MapOverlayModelData* find(std::uint64_t) const; std::vector<MapOverlayModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MapOverlayModelData> data_; };
}
