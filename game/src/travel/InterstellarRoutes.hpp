// Intended function: bounded star-system route planning with jump range, fuel cost, hazard and Imperial-pressure weighting.
#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
namespace elysium{
struct StarRouteNode{std::uint32_t systemId{};double x{},y{},z{};float hazard{},imperialPressure{};};
struct InterstellarRouteRequest{std::uint32_t from{},to{};double jumpRangeLy{50};double fuelPerLy{1};double fuelAvailable{1000};std::uint32_t maxExpanded{4096};float hazardWeight{1},imperialWeight{1};};
struct InterstellarRoute{bool found{};double distanceLy{},fuelCost{},weightedCost{};std::vector<std::uint32_t>systems;};
class InterstellarRoutePlanner{public:void upsert(StarRouteNode node);InterstellarRoute find(const InterstellarRouteRequest&request)const;private:std::unordered_map<std::uint32_t,StarRouteNode>nodes_;};
}
