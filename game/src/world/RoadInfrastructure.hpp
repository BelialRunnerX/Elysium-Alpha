#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace elysium::world {

enum class InfrastructureKind : std::uint8_t { Road, Bridge, Causeway, Ramp };
struct RoadSegment {
    std::uint64_t stableId{};
    std::uint64_t worldId{};
    std::uint64_t fromKey{};
    std::uint64_t toKey{};
    std::uint64_t regionKey{};
    InfrastructureKind kind{InfrastructureKind::Road};
    double widthMeters{};
    double maximumLoad{};
    double speedMultiplier{1.0};
    double reliabilityBonus{};
    bool enabled{true};
};
struct VehicleTraversalProfile {
    double widthMeters{};
    double mass{};
    double maximumSlopeDegrees{};
    bool canFord{};
    bool hover{};
};
struct TerrainEdgeSample {
    std::uint64_t worldId{};
    std::uint64_t fromKey{};
    std::uint64_t toKey{};
    std::uint64_t regionKey{};
    double baseCost{};
    double slopeDegrees{};
    double roughness{};
    double gapMeters{};
    double waterDepthMeters{};
};
enum class RoadRouteReason : std::uint8_t { None, InvalidInput, WidthBlocked, LoadBlocked, BridgeRequired, SlopeBlocked, TerrainBlocked };
struct RoadCostResult {
    bool traversable{};
    RoadRouteReason reason{RoadRouteReason::None};
    double cost{};
    double speedMultiplier{1.0};
    double reliability{};
    std::uint64_t infrastructureStableId{};
    std::string explanation;
};
struct InfrastructureEditResult { bool changed{}; std::vector<std::uint64_t> invalidatedRegionKeys; };

class RoadInfrastructure {
public:
    InfrastructureEditResult upsert(RoadSegment segment);
    InfrastructureEditResult erase(std::uint64_t stableId);
    [[nodiscard]] RoadCostResult evaluate(const TerrainEdgeSample& edge, const VehicleTraversalProfile& vehicle) const;
    [[nodiscard]] std::vector<RoadSegment> snapshot() const;
    bool restore(const std::vector<RoadSegment>& segments);
private:
    std::map<std::uint64_t,RoadSegment> segments_;
};

} // namespace elysium::world
