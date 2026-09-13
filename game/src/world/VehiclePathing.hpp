// Intended function: deterministic vehicle-lane graph with occupancy reservations for rovers, haulers and surface transit without reusing humanoid nav assumptions.
#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
namespace elysium{
struct VehicleLaneNode{std::uint64_t nodeId{},address{};float width{},maxSlope{},speedLimit{};};
struct VehicleLaneEdge{std::uint64_t toNode{};float distance{},hazard{};bool oneWay{};};
struct VehicleRoute{bool found{};float distance{},estimatedSeconds{};std::vector<std::uint64_t>nodes;};
class VehiclePathing{public:void upsertNode(VehicleLaneNode node);void setEdges(std::uint64_t node,std::vector<VehicleLaneEdge>edges);VehicleRoute route(std::uint64_t start,std::uint64_t goal,float vehicleWidth,float maxSlope)const;bool reserveNode(std::uint64_t node,std::uint64_t vehicle,std::uint64_t untilTick);void expire(std::uint64_t tick);private:struct Reservation{std::uint64_t vehicle{},until{};};std::unordered_map<std::uint64_t,VehicleLaneNode>nodes_;std::unordered_map<std::uint64_t,std::vector<VehicleLaneEdge>>edges_;std::unordered_map<std::uint64_t,Reservation>reservations_;};
}
