// Intended function: imported travel implementation for Vehicles; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/PlanetSurface.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

enum class VehicleType : std::uint8_t {
    ScoutRover = 0,
    CargoCrawler = 1,
    MiningRig = 2,
    AmphibiousSkiff = 3,
    HoverSled = 4,
    SiegeHauler = 5
};

struct VehicleDefinition {
    VehicleType type{};
    std::string_view name;
    float cruiseSpeedMps{};
    int cargoSlots{};
    float maxSlopeDegrees{};
    float maxRoughness{};
    float maxWaterDepthMeters{};
    bool amphibious{};
    bool hover{};
    float minGravityG{};
    float maxGravityG{};
    float energyPerKm{};
    float hull{};
    float industrialSuspicionPerKm{};
};

const VehicleDefinition& vehicleDefinition(VehicleType type);
const std::vector<VehicleDefinition>& vehicleRegistry();

struct VehicleCargoStack {
    int itemId{};
    int count{};

    bool operator==(const VehicleCargoStack&) const = default;
};

struct VehicleState {
    std::uint64_t stableId{};
    VehicleType type{VehicleType::ScoutRover};
    std::uint32_t systemId{};
    std::uint32_t planetId{};
    SurfaceCellAddress address{};
    float hull{};
    float energy{}; // abstract battery/fuel units
    float maintenance{1.0f};
    std::uint64_t ownerStableId{};
    std::uint64_t crewStableId{};
    bool autonomous{};
    SurfaceCellAddress routeTarget{};
    bool hasRouteTarget{};
    std::vector<VehicleCargoStack> cargo;

    bool operator==(const VehicleState&) const = default;
};

enum class VehicleMoveFailure : std::uint8_t {
    None = 0,
    InvalidState,
    WrongPlanet,
    SlopeTooSteep,
    TerrainTooRough,
    WaterTooDeep,
    GravityUnsupported,
    WeatherUnsafe,
    CargoOverCapacity,
    InsufficientEnergy,
    NoCrewOrAutonomy
};

const char* vehicleMoveFailureName(VehicleMoveFailure failure);

struct VehicleMoveContext {
    std::uint32_t systemId{};
    std::uint32_t planetId{};
    SurfaceCellAddress from{};
    SurfaceCellAddress to{}; // chosen by the shared navigation service
    float segmentMeters{1.0f};
    float slopeDegrees{};
    float roughness{};
    float waterDepthMeters{};
    float gravityG{1.0f};
    float weatherSeverity{};
};

struct VehicleMoveResult {
    bool allowed{};
    VehicleMoveFailure failure{VehicleMoveFailure::None};
    std::string reason;
    float speedMps{};
    float energyCost{};
    float suspicionGenerated{};
};

class VehicleSystem {
public:
    VehicleState create(std::uint64_t stableId,
                        VehicleType type,
                        std::uint32_t systemId,
                        std::uint32_t planetId,
                        SurfaceCellAddress address) const;

    VehicleMoveResult evaluateMove(const VehicleState& vehicle,
                                   const VehicleMoveContext& context) const;

    // Commit uses a next address supplied by shared world/navigation services.
    // The vehicle system intentionally owns no private terrain grid or seam map.
    bool commitMove(VehicleState& vehicle,
                    const VehicleMoveContext& context,
                    VehicleMoveResult* result = nullptr) const;

    static int cargoUsedSlots(const VehicleState& vehicle);
    static int cargoInsert(VehicleState& vehicle,int itemId,int count);
    static int cargoExtract(VehicleState& vehicle,int itemId,int count);
};

} // namespace elysium
