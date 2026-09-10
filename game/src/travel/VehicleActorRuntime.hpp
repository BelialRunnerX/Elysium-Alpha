#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace elysium::travel {

enum class VehicleCapability : std::uint32_t {
    Survey = 1u << 0,
    BulkCargo = 1u << 1,
    MobileMining = 1u << 2,
    Amphibious = 1u << 3,
    RoughTerrainHover = 1u << 4,
    SiegeLogistics = 1u << 5,
    Autonomous = 1u << 6,
};
constexpr std::uint32_t vehicleCapability(VehicleCapability c) noexcept { return static_cast<std::uint32_t>(c); }

struct VehicleArchetypeDefinition {
    std::uint64_t contentId{};
    std::uint32_t capabilities{};
    std::uint64_t cargoCapacity{};
    double energyCapacity{};
    double fuelCapacity{};
    double cruiseSpeedMps{};
    double maximumSlopeDegrees{};
    double maximumRoughness{};
    double maximumWaterDepthMeters{};
    double baseSignaturePerKm{};
};

struct VehicleModuleDefinition {
    std::uint64_t contentId{};
    std::uint32_t addCapabilities{};
    std::uint64_t cargoCapacityBonus{};
    double energyCapacityBonus{};
    double fuelCapacityBonus{};
    double speedMultiplier{1.0};
    double signatureMultiplier{1.0};
    double poweredEnergyPerKm{};
};

struct InstalledVehicleModule {
    std::uint64_t stableItemId{};
    std::uint64_t moduleContentId{};
    bool enabled{true};
};

struct VehicleCargoStack {
    std::uint64_t itemStableId{};
    std::uint64_t contentId{};
    std::uint64_t count{};
};

struct GroundVehicleState {
    std::uint64_t stableId{};
    std::uint64_t archetypeContentId{};
    std::uint64_t ownerStableId{};
    std::uint64_t driverStableId{};
    std::uint64_t worldId{};
    std::uint64_t locationKey{};
    std::uint64_t revision{1};
    double energy{};
    double fuel{};
    double hull{1.0};
    std::vector<InstalledVehicleModule> modules;
    std::vector<VehicleCargoStack> cargo;
};

struct VehicleDerivedState {
    bool valid{};
    std::uint32_t capabilities{};
    std::uint64_t cargoCapacity{};
    std::uint64_t cargoCount{};
    double energyCapacity{};
    double fuelCapacity{};
    double cruiseSpeedMps{};
    double signaturePerKm{};
    double poweredEnergyPerKm{};
};

enum class VehicleBlockReason : std::uint8_t {
    None,
    MissingVehicle,
    UnknownArchetype,
    UnknownModule,
    DuplicateModule,
    MalformedDefinition,
    NoDriverOrAutonomy,
    InsufficientEnergy,
    InsufficientFuel,
    CargoCapacityExceeded,
    SlopeUnsupported,
    RoughnessUnsupported,
    WaterUnsupported,
    WrongWorld,
    InvalidDistance,
};

struct VehicleTraversalContext {
    std::uint64_t worldId{};
    std::uint64_t destinationKey{};
    double distanceMeters{};
    double slopeDegrees{};
    double roughness{};
    double waterDepthMeters{};
};

struct VehicleTraversalCheck {
    bool allowed{};
    VehicleBlockReason reason{VehicleBlockReason::None};
    double energyRequired{};
    double fuelRequired{};
    double effectiveSpeedMps{};
    double standingSignal{};
};

struct VehicleStandingEmission {
    std::uint64_t sourceStableId{};
    std::uint64_t worldId{};
    std::uint64_t locationKey{};
    double magnitude{};
    std::uint64_t reasonContentId{};
};

struct VehicleSnapshot {
    std::vector<VehicleArchetypeDefinition> archetypes;
    std::vector<VehicleModuleDefinition> modules;
    std::vector<GroundVehicleState> vehicles;
};

class VehicleActorRuntime {
public:
    bool publish(VehicleArchetypeDefinition definition);
    bool publish(VehicleModuleDefinition definition);
    bool create(GroundVehicleState state);
    bool installModules(std::uint64_t vehicleStableId, std::vector<InstalledVehicleModule> modules);

    [[nodiscard]] const GroundVehicleState* find(std::uint64_t vehicleStableId) const;
    [[nodiscard]] VehicleDerivedState derive(std::uint64_t vehicleStableId) const;
    [[nodiscard]] VehicleTraversalCheck checkTraversal(std::uint64_t vehicleStableId, const VehicleTraversalContext& context) const;
    bool commitTraversal(std::uint64_t vehicleStableId, const VehicleTraversalContext& context, VehicleStandingEmission* emission = nullptr);

    std::uint64_t insertCargo(std::uint64_t vehicleStableId, VehicleCargoStack stack);
    std::uint64_t extractCargo(std::uint64_t vehicleStableId, std::uint64_t itemStableId, std::uint64_t count);

    [[nodiscard]] VehicleSnapshot snapshot() const;
    bool restore(const VehicleSnapshot& snapshot);

private:
    std::map<std::uint64_t, VehicleArchetypeDefinition> archetypes_;
    std::map<std::uint64_t, VehicleModuleDefinition> modules_;
    std::map<std::uint64_t, GroundVehicleState> vehicles_;
};

std::vector<VehicleArchetypeDefinition> makeDefaultVehicleArchetypes(std::uint64_t contentNamespaceSeed);

} // namespace elysium::travel
