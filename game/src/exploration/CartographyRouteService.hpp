#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <vector>
namespace elysium::exploration {
struct KnownResourceSummary{std::uint64_t resourceContentId{};double abundanceLow{},abundanceHigh{},confidence{};};
struct KnownChart{std::uint64_t systemId{},planetId{},revision{1};bool positionKnown{},stationAccessKnown{},stationAccessible{},suspicionKnown{};double knownSuspicion{};std::vector<KnownResourceSummary>resources;std::vector<std::uint64_t>knownHazardContentIds;};
struct MapAnnotation{std::uint64_t stableId{},systemId{},planetId{},locationKey{};std::uint64_t authorStableId{};};
struct NeedFilter{std::optional<std::uint64_t>resourceContentId;double minimumAbundance{},minimumConfidence{};std::vector<std::uint64_t>excludedHazards;bool requireAccessibleStation{};std::optional<double>maximumKnownSuspicion;};
struct NeedMatch{std::uint64_t systemId{},planetId{};double abundanceLow{},abundanceHigh{},confidence{};};
struct KnownRouteEdge{std::uint64_t fromSystemId{},toSystemId{};double distanceLy{},hazardRisk{},interdictionFriction{};bool activeGate{},known{true};};
struct RouteNodeKnowledge{std::uint64_t systemId{};bool suspicionKnown{};double suspicion{};};
struct RouteEstimateRequest{std::uint64_t originSystemId{},destinationSystemId{};double jumpRangeLy{};std::uint64_t availableFuelCells{},fuelCellsPerJump{1};double hazardWeight{1.0},interdictionWeight{1.0},suspicionWeight{1.0};};
struct RouteEstimate{bool found{};std::vector<std::uint64_t>systems;std::uint64_t jumps{},fuelCellsRequired{};double distanceLy{},risk{},cost{};};
class CartographyRouteService{public:
 bool publish(KnownChart);bool annotate(MapAnnotation);bool publish(KnownRouteEdge);bool publish(RouteNodeKnowledge);[[nodiscard]]std::vector<NeedMatch>need(const NeedFilter&)const;[[nodiscard]]RouteEstimate route(const RouteEstimateRequest&)const;[[nodiscard]]const KnownChart*chart(std::uint64_t systemId,std::uint64_t planetId)const;
private:std::map<std::pair<std::uint64_t,std::uint64_t>,KnownChart>charts_;std::map<std::uint64_t,MapAnnotation>annotations_;std::map<std::pair<std::uint64_t,std::uint64_t>,KnownRouteEdge>edges_;std::map<std::uint64_t,RouteNodeKnowledge>nodes_;
};
} // namespace elysium::exploration
