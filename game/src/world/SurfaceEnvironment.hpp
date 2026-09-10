// Intended function: imported world implementation for SurfaceEnvironment; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/PlanetSurface.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace elysium {

// Stable deterministic ordering for sparse cube-sphere addresses. The ordering
// is a runtime/service detail, not a serialized dense planet index.
struct SurfaceCellAddressLess {
    bool operator()(const SurfaceCellAddress& a, const SurfaceCellAddress& b) const noexcept;
};

struct SurfaceRoomDescriptor {
    std::uint64_t runtimeId{};
    SurfaceCellAddress anchor{};
    bool sealed{};
    bool unbounded{};
    bool budgetExhausted{};
    std::uint64_t geometryRevision{};
    std::vector<SurfaceCellAddress> cells;
};

struct SurfaceRoomGraphTelemetry {
    std::uint64_t geometryRevision{};
    int cachedRooms{};
    int sealedRooms{};
    int unboundedRooms{};
    int visitedCellsLastQuery{};
    int cacheHits{};
    int cacheMisses{};
    int invalidations{};
};

// Sparse, bounded connected-gas query/cache. Geometry changes invalidate the
// sparse cache deterministically, so split/merge consumers never retain stale
// room membership. A fill is sealed only when it terminates before its budget.
class SurfaceRoomGraph {
public:
    explicit SurfaceRoomGraph(std::uint64_t worldSeed = 0) : worldSeed_(worldSeed) {}

    SurfaceRoomDescriptor query(const PlanetSurface& planet,
                                SurfaceCellAddress start,
                                int maxCells = 8192);
    void invalidate();
    SurfaceRoomGraphTelemetry telemetry() const { return telemetry_; }

private:
    std::uint64_t worldSeed_{};
    std::uint64_t cachedRevision_{};
    std::vector<SurfaceRoomDescriptor> rooms_;
    std::map<SurfaceCellAddress, std::size_t, SurfaceCellAddressLess> membership_;
    SurfaceRoomGraphTelemetry telemetry_{};

    void refreshRevision(const PlanetSurface& planet);
    std::uint64_t makeRuntimeRoomId(const SurfaceRoomDescriptor& room) const;
};

enum class SurfaceGasLinkType : std::uint8_t {
    Vent = 0,
    Duct = 1,
    Filter = 2,
    Pump = 3
};

struct SurfaceGasState {
    // Scalars are normalized partial-pressure-like values. oxygen/smoke/toxin
    // are always clamped to [0, pressure].
    float pressure{};
    float oxygen{};
    float smoke{};
    float toxin{};

    bool breathable() const { return pressure >= 0.55f && oxygen >= 0.45f && smoke < 0.20f && toxin < 0.10f; }
};

struct SurfaceGasRegionState {
    std::uint64_t runtimeRoomId{};
    SurfaceCellAddress anchor{};
    bool sealed{};
    bool unbounded{};
    int cellCount{};
    SurfaceGasState gas{};
};

struct SurfaceGasLink {
    std::uint64_t stableId{};
    SurfaceGasLinkType type{SurfaceGasLinkType::Duct};
    SurfaceCellAddress a{};
    SurfaceCellAddress b{};
    bool enabled{true};
    bool powered{true};
    float flowPerSecond{1.0f};
    float filterEfficiency{0.80f};
};

enum class SurfaceFluidKind : std::uint8_t {
    Water = 0,
    Acid = 1,
    Lava = 2,
    Industrial = 3,
    Cryofluid = 4
};

struct SurfaceFluidCell {
    SurfaceFluidKind kind{SurfaceFluidKind::Water};
    float amount{};       // [0,1] cell fill fraction.
    float temperatureC{20.0f};
};

struct SurfaceFluidPump {
    std::uint64_t stableId{};
    SurfaceCellAddress from{};
    SurfaceCellAddress to{};
    bool enabled{true};
    bool powered{true};
    float ratePerSecond{1.0f};
    std::optional<SurfaceFluidKind> kindFilter;
};

enum class SurfaceContaminant : std::uint8_t {
    Radiation = 0,
    Toxin = 1,
    Spores = 2,
    Corrosive = 3,
    Rift = 4,
    Count = 5
};

constexpr std::size_t kSurfaceContaminantCount = static_cast<std::size_t>(SurfaceContaminant::Count);

struct SurfaceContaminationCell {
    std::array<float, kSurfaceContaminantCount> intensity{};
};

struct SurfaceFireCell {
    float intensity{};
    float fuelSeconds{};
};

enum class SurfaceEnvironmentEventKind : std::uint8_t {
    PressureLoss = 0,
    Fire = 1,
    ToxicGas = 2,
    Radiation = 3,
    Contamination = 4,
    Flood = 5,
    FluidPumpBlocked = 6
};

struct SurfaceEnvironmentEvent {
    SurfaceEnvironmentEventKind kind{SurfaceEnvironmentEventKind::PressureLoss};
    SurfaceCellAddress cell{};
    float severity{};
    std::uint64_t sourceStableId{};
};

struct SurfaceEnvironmentTelemetry {
    int gasRegions{};
    int sealedGasRegions{};
    int gasLinks{};
    int gasTransfers{};
    int activeFluidCells{};
    int fluidTransfers{};
    int activeThermalCells{};
    int activeFires{};
    int contaminationCells{};
    int eventsEmitted{};
    int boundedWorkUnits{};
    bool fluidSettled{};
};

struct SurfaceEnvironmentInspection {
    SurfaceCellAddress cell{};
    SurfaceRoomDescriptor room{};
    SurfaceGasState gas{};
    bool hasGas{};
    std::optional<SurfaceFluidCell> fluid;
    float temperatureC{20.0f};
    SurfaceContaminationCell contamination{};
    float fireIntensity{};
    std::string read;
    std::string decided;
    std::string changed;
};

struct SurfaceEnvironmentSnapshot {
    struct GasSeed {
        SurfaceCellAddress anchor{};
        SurfaceGasState gas{};
    };
    struct FluidRecord {
        SurfaceCellAddress cell{};
        SurfaceFluidCell fluid{};
    };
    struct ThermalRecord {
        SurfaceCellAddress cell{};
        float temperatureC{};
    };
    struct ContaminationRecord {
        SurfaceCellAddress cell{};
        SurfaceContaminationCell contamination{};
    };
    struct FireRecord {
        SurfaceCellAddress cell{};
        SurfaceFireCell fire{};
    };

    std::vector<GasSeed> gasSeeds;
    std::vector<SurfaceGasLink> gasLinks;
    std::vector<FluidRecord> fluids;
    std::vector<SurfaceFluidPump> fluidPumps;
    std::vector<ThermalRecord> thermal;
    std::vector<ContaminationRecord> contamination;
    std::vector<FireRecord> fires;
};

// Portable bounded environmental kernel. It owns only sparse field state and
// stable infrastructure records; it never owns EnTT entities, renderer handles,
// or dense planet-sized arrays. In the authoritative build, device records are
// expected to be represented by ECS entities/components while these fields stay
// specialized spatial data.
class SurfaceEnvironmentSystem {
public:
    explicit SurfaceEnvironmentSystem(std::uint64_t worldSeed = 0,
                                      int roomBudget = 8192,
                                      int maxFluidTransfersPerTick = 2048,
                                      int maxThermalCellsPerTick = 4096);

    SurfaceRoomGraph& roomGraph() { return roomGraph_; }
    const SurfaceRoomGraph& roomGraph() const { return roomGraph_; }

    bool setRoomGas(const PlanetSurface& planet, SurfaceCellAddress anchor, SurfaceGasState gas);
    std::optional<SurfaceGasRegionState> gasAt(const PlanetSurface& planet, SurfaceCellAddress cell);
    bool injectGas(const PlanetSurface& planet,
                   SurfaceCellAddress anchor,
                   float pressure,
                   float oxygen,
                   float smoke = 0.0f,
                   float toxin = 0.0f);

    std::uint64_t addGasLink(SurfaceGasLinkType type,
                             SurfaceCellAddress a,
                             SurfaceCellAddress b,
                             float flowPerSecond = 1.0f,
                             float filterEfficiency = 0.80f);
    bool restoreGasLink(const SurfaceGasLink& link);
    bool removeGasLink(std::uint64_t stableId);
    SurfaceGasLink* findGasLink(std::uint64_t stableId);

    void setFluid(SurfaceCellAddress cell, SurfaceFluidKind kind, float amount, float temperatureC = 20.0f);
    std::optional<SurfaceFluidCell> fluidAt(SurfaceCellAddress cell) const;
    float totalFluidAmount() const;
    std::uint64_t addFluidPump(SurfaceCellAddress from,
                               SurfaceCellAddress to,
                               float ratePerSecond = 1.0f,
                               std::optional<SurfaceFluidKind> kindFilter = std::nullopt);
    bool restoreFluidPump(const SurfaceFluidPump& pump);
    SurfaceFluidPump* findFluidPump(std::uint64_t stableId);

    void addHeat(SurfaceCellAddress cell, float deltaC);
    float temperatureAt(SurfaceCellAddress cell) const;
    bool ignite(const PlanetSurface& planet, SurfaceCellAddress solidCell, float intensity = 1.0f, float fuelSeconds = 8.0f);

    void contaminate(SurfaceCellAddress cell, SurfaceContaminant contaminant, float amount);
    void decontaminate(SurfaceCellAddress cell, SurfaceContaminant contaminant, float amount);
    SurfaceContaminationCell contaminationAt(SurfaceCellAddress cell) const;

    void update(PlanetSurface& planet, float dt);
    std::vector<SurfaceEnvironmentEvent> consumeEvents();
    SurfaceEnvironmentTelemetry telemetry() const { return telemetry_; }
    SurfaceEnvironmentInspection inspect(const PlanetSurface& planet, SurfaceCellAddress cell);

    SurfaceEnvironmentSnapshot snapshot() const;
    bool restore(const PlanetSurface& planet, const SurfaceEnvironmentSnapshot& snapshot, std::string* error = nullptr);

private:
    struct GasRegion {
        SurfaceRoomDescriptor room{};
        SurfaceGasState gas{};
    };

    std::uint64_t worldSeed_{};
    std::uint64_t nextSerial_{1};
    int roomBudget_{8192};
    int maxFluidTransfersPerTick_{2048};
    int maxThermalCellsPerTick_{4096};
    std::uint64_t gasGeometryRevision_{};
    SurfaceRoomGraph roomGraph_;
    std::vector<GasRegion> gasRegions_;
    std::vector<SurfaceGasLink> gasLinks_;
    std::map<SurfaceCellAddress, SurfaceFluidCell, SurfaceCellAddressLess> fluids_;
    std::vector<SurfaceFluidPump> fluidPumps_;
    std::map<SurfaceCellAddress, float, SurfaceCellAddressLess> thermal_;
    std::map<SurfaceCellAddress, SurfaceContaminationCell, SurfaceCellAddressLess> contamination_;
    std::map<SurfaceCellAddress, SurfaceFireCell, SurfaceCellAddressLess> fires_;
    std::vector<SurfaceEnvironmentEvent> events_;
    SurfaceEnvironmentTelemetry telemetry_{};
    bool fluidSettled_{true};

    std::uint64_t allocateStableId();
    static SurfaceGasState sanitizeGas(SurfaceGasState gas);
    static bool gasNonZero(const SurfaceGasState& gas);
    static bool flammable(BlockType block);
    static float ambientTemperature(const PlanetSurface& planet);

    void reconcileGasTopology(const PlanetSurface& planet);
    std::optional<std::size_t> gasRegionContaining(SurfaceCellAddress cell) const;
    std::optional<std::size_t> gasRegionForAnchor(const PlanetSurface& planet, SurfaceCellAddress anchor);
    void updateGasLinks(const PlanetSurface& planet, float dt);
    void updateOpenGas(float dt);
    void updateFluidPumps(const PlanetSurface& planet, float dt);
    void updateFluids(const PlanetSurface& planet, float dt);
    void updateThermal(const PlanetSurface& planet, float dt);
    void updateFire(const PlanetSurface& planet, float dt);
    void updateContamination(float dt);
    void emitEvent(SurfaceEnvironmentEventKind kind,
                   SurfaceCellAddress cell,
                   float severity,
                   std::uint64_t sourceStableId = 0);
};

} // namespace elysium
