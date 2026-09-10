// Intended function: imported travel implementation for ShipTravel; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/PlanetSurface.hpp"
#include "world/SurfaceIndustry.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

// Stable content identity. Numeric values are save data; append only.
enum class ShipModuleId : std::uint16_t {
    WarpDriveI = 1,
    WarpDriveII = 2,
    WarpDriveIII = 3,
    WarpDriveIV = 4,
    PulseEngine = 5,
    AtmosphericThrusters = 6,
    VectorJets = 7,
    FuelTank = 8,
    CargoRack = 9,
    SurveyScanner = 10,
    DeepScanner = 11,
    AtmosphereSampler = 12,
    HullPlating = 13,
    ThermalShield = 14,
    RadiationBaffle = 15,
    AutopilotComputer = 16,
    NavigationCore = 17,
    EmergencyBeacon = 18,
    DockingCollar = 19,
    DroneBay = 20,
    HabitationPod = 21,
    SmallRefinery = 22,
    SmugglerHold = 23
};

enum class ShipModuleSlot : std::uint8_t {
    Propulsion = 0,
    Utility = 1,
    Science = 2,
    Defense = 3,
    Control = 4,
    Safety = 5,
    Industry = 6,
    Faction = 7
};

struct ShipModuleDefinition {
    ShipModuleId id{};
    std::string_view name;
    ShipModuleSlot slot{};
    int tier{};
    float mass{};
    float powerDraw{};
    float jumpRangeLy{};       // Highest installed warp drive wins.
    int fuelCapacityBonus{};
    int cargoSlotBonus{};
    float hullBonus{};
    float handlingBonus{};
    int scannerGradeBonus{};
    float thermalEntryMitigation{};
    float radiationEntryMitigation{};
    bool pulseTransit{};
    bool docking{};
    bool autopilot{};
    bool emergencyRecovery{};
    bool deepScan{};
    bool atmosphereScan{};
    bool droneSupport{};
    bool habitation{};
    bool fieldRefinery{};
    float inspectionExposureMultiplier{1.0f};
};

const ShipModuleDefinition* shipModuleDefinition(ShipModuleId id);
const std::vector<ShipModuleDefinition>& shipModuleRegistry();

struct ShipDerivedStats {
    int cargoSlots{12};
    float jumpRangeLy{};
    int fuelCapacity{4};
    float maxHull{200.0f};
    float atmosphericHandling{};
    int scannerGrade{1};
    float moduleMass{};
    float thermalEntryMitigation{};
    float radiationEntryMitigation{};
    float inspectionExposureMultiplier{1.0f};
    bool pulseTransit{};
    bool docking{};
    bool autopilot{};
    bool emergencyRecovery{};
    bool deepScan{};
    bool atmosphereScan{};
    bool droneSupport{};
    bool habitation{};
    bool fieldRefinery{};
};

struct ShipCargoStack {
    int itemId{};
    int count{};

    bool operator==(const ShipCargoStack&) const = default;
};

enum class ShipLocationKind : std::uint8_t {
    Surface = 0,
    Ascending = 1,
    Orbit = 2,
    InSystemTransit = 3,
    WarpTransit = 4,
    Descending = 5,
    Docked = 6
};

struct ShipLocation {
    ShipLocationKind kind{ShipLocationKind::Surface};
    std::uint32_t systemId{};
    std::uint32_t planetId{};
    SurfaceCellAddress surfaceAddress{};
    std::uint64_t dockedObjectStableId{};

    bool operator==(const ShipLocation&) const = default;
};

struct ShipTransitState {
    bool active{};
    ShipLocationKind phase{ShipLocationKind::Surface};
    float elapsedSeconds{};
    float durationSeconds{};
    std::uint32_t targetSystemId{};
    std::uint32_t targetPlanetId{};
    SurfaceCellAddress targetSurfaceAddress{};
    std::uint64_t targetDockStableId{};
    float distanceLy{};
    int reservedFuelCells{};

    bool operator==(const ShipTransitState&) const = default;
};

struct ShipState {
    std::uint64_t stableId{};
    ShipLocation location{};
    std::vector<ShipModuleId> installedModules;
    std::vector<ShipCargoStack> cargo;
    int fuelCells{4};
    float hull{200.0f};
    float maintenance{1.0f};
    std::uint64_t surfaceCargoLoaderStableId{};
    ShipTransitState transit{};

    bool operator==(const ShipState&) const = default;
};

ShipDerivedStats deriveShipStats(const ShipState& ship);
bool validateShipState(const ShipState& ship, std::string* error = nullptr);
int shipCargoCount(const ShipState& ship, int itemId);
int shipCargoUsedSlots(const ShipState& ship);
int shipCargoInsert(ShipState& ship, int itemId, int count);
int shipCargoExtract(ShipState& ship, int itemId, int count);

struct TakeoffEnvironment {
    float gravityG{1.0f};
    float stormSeverity{};       // [0,1]
    float thermalSeverity{};     // [0,1]
    float radiationSeverity{};   // [0,1]
    float cargoMassFraction{};   // [0,1], caller may derive from its item-mass service.
};

enum class TravelFailure : std::uint8_t {
    None = 0,
    Busy,
    NotOnSurface,
    NotInOrbit,
    MissingAtmosphericThrusters,
    MissingPulseEngine,
    MissingWarpDrive,
    MissingDockingCollar,
    CargoOverCapacity,
    FuelCapacityInvalid,
    InsufficientFuel,
    RangeExceeded,
    GravityTooHigh,
    StormTooSevere,
    HullTooDamaged,
    ThermalEntryUnsafe,
    RadiationEntryUnsafe,
    DestinationInvalid,
    LoaderUnavailable,
    LoaderUnpowered,
    LoaderNotDocked,
    CargoFull,
    InvalidState
};

const char* travelFailureName(TravelFailure failure);

struct TravelCheck {
    bool allowed{};
    TravelFailure failure{TravelFailure::None};
    std::string reason;
};

struct ShipTravelTelemetry {
    int takeoffsStarted{};
    int takeoffsCompleted{};
    int landingsStarted{};
    int landingsCompleted{};
    int inSystemTrips{};
    int warpTrips{};
    int fuelCellsConsumed{};
    int cargoItemsLoaded{};
    int cargoItemsUnloaded{};
    int rejectedCommands{};
};

// Owner-thread state machine. It never owns voxel storage, ECS handles, or a
// renderer. The caller may present Ascending/Descending/Orbit however it likes.
// State/location/fuel mutation is deterministic and only occurs through these
// explicit operations.
class ShipTravelSystem {
public:
    static constexpr float TakeoffSeconds = 2.5f;        // prototype tuning
    static constexpr float LandingSeconds = 2.5f;        // prototype tuning
    static constexpr float MinInSystemSeconds = 1.5f;    // prototype framing minimum
    static constexpr float WarpSeconds = 3.0f;           // prototype framing minimum
    static constexpr float MinimumOperationalHullFraction = 0.20f;

    TravelCheck checkTakeoff(const ShipState& ship, const TakeoffEnvironment& environment) const;
    bool beginTakeoff(ShipState& ship, const TakeoffEnvironment& environment, std::string* reason = nullptr);
    bool beginLanding(ShipState& ship,
                      std::uint32_t targetPlanetId,
                      SurfaceCellAddress targetSurfaceAddress,
                      const TakeoffEnvironment& entryEnvironment,
                      std::string* reason = nullptr);
    bool beginInSystemTravel(ShipState& ship,
                             std::uint32_t targetPlanetId,
                             float transitSeconds,
                             std::string* reason = nullptr);
    bool beginWarpTravel(ShipState& ship,
                         std::uint32_t targetSystemId,
                         float distanceLy,
                         std::string* reason = nullptr);
    bool dock(ShipState& ship, std::uint64_t orbitalStableId, std::string* reason = nullptr);
    bool undock(ShipState& ship, std::string* reason = nullptr);

    void update(ShipState& ship, float dt);

    // Live Cargo Loader binding. This intentionally does not make a cargo
    // loader's ordinary infrastructure route point at a ship ID; the existing
    // sparse infrastructure journal has its own closure rules. Docking binds a
    // stable loader identity on ShipState, and transfer remains local and
    // explicit. Matter is conserved if either endpoint rejects the move.
    bool bindSurfaceCargoLoader(ShipState& ship,
                                const SurfaceInfrastructure& infrastructure,
                                std::uint64_t loaderStableId,
                                std::string* reason = nullptr) const;
    int loadFromSurfaceCargoLoader(ShipState& ship,
                                   SurfaceInfrastructure& infrastructure,
                                   SurfaceIndustrySystem& industry,
                                   int itemId,
                                   int count,
                                   std::string* reason = nullptr);
    int unloadToSurfaceCargoLoader(ShipState& ship,
                                   SurfaceInfrastructure& infrastructure,
                                   SurfaceIndustrySystem& industry,
                                   int itemId,
                                   int count,
                                   std::string* reason = nullptr);

    ShipTravelTelemetry telemetry() const { return telemetry_; }
    void resetTelemetry() { telemetry_ = {}; }

private:
    ShipTravelTelemetry telemetry_{};

    bool reject(TravelFailure failure, std::string_view detail, std::string* reason);
    static int warpFuelCost(float distanceLy);
};

} // namespace elysium
