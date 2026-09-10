// Intended function: explicit cargo-rail graph, consists and shipment scheduling for high-throughput base logistics without global inventory search.
#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
namespace elysium{
struct RailNode{std::uint64_t nodeId{},address{};std::uint8_t platformCount{1};};
struct RailSegment{std::uint64_t segmentId{},fromNode{},toNode{};float lengthMeters{},speedLimit{20};bool bidirectional{true};};
struct RailConsist{std::uint64_t consistId{},currentNode{};std::uint32_t capacity{},load{};float maxSpeed{15};bool assigned{};};
struct RailShipment{std::uint64_t shipmentId{},sourceNode{},targetNode{};std::uint32_t itemId{},quantity{};std::uint8_t priority{};};
struct RailMission{std::uint64_t shipmentId{},consistId{};std::vector<std::uint64_t>nodeRoute;std::uint32_t itemId{},quantity{};};
class CargoRailSystem{public:void upsertNode(RailNode n);void upsertSegment(RailSegment s);void upsertConsist(RailConsist c);bool submit(RailShipment s);std::vector<RailMission>plan(std::size_t maxMissions);void complete(std::uint64_t consistId);private:std::optional<std::vector<std::uint64_t>>route(std::uint64_t a,std::uint64_t b)const;std::unordered_map<std::uint64_t,RailNode>nodes_;std::unordered_map<std::uint64_t,RailSegment>segments_;std::unordered_map<std::uint64_t,RailConsist>consists_;std::vector<RailShipment>shipments_;};
}
