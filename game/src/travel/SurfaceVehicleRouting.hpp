#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Plan bounded rover, truck, crawler, rail, and convoy routes across local navigation summaries.
struct SurfaceVehicleRoutingOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SurfaceVehicleRoutingData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SurfaceVehicleRoutingStore { public: bool apply(const SurfaceVehicleRoutingOp&); bool erase(std::uint64_t); const SurfaceVehicleRoutingData* find(std::uint64_t) const; std::vector<SurfaceVehicleRoutingData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SurfaceVehicleRoutingData> data_; };
}
