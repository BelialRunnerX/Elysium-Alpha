// Intended function: imported world implementation for PoiSites; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/PlanetSurface.hpp"
#include "world/PlanetTypes.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

// Major sites are assembled from authored semantic modules. Geometry/assets are
// deliberately outside this layer; this file owns deterministic site identity,
// composition, lifecycle and persistence-facing state only.
enum class SiteDomain : std::uint8_t { Surface = 0, Subsurface = 1, Orbit = 2, Space = 3, Anomalous = 4 };
enum class SiteFaction : std::uint8_t { None = 0, Frontier = 1, Unsworn = 2, Imperial = 3, Ancient = 4, Player = 5, Hostile = 6 };
enum class SiteArchitecture : std::uint8_t { Frontier = 0, Unsworn = 1, Imperial = 2, Ancient = 3, Industrial = 4, Rift = 5 };

enum class SiteFamily : std::uint8_t {
    SupplyCache = 0,
    CrashedShip,
    SurveyOutpost,
    UnswornCamp,
    MiningRig,
    Ruin,
    Monolith,
    RiftAnomaly,
    ImperialWaystation,
    AbandonedFarm,
    WeatherStation,
    SubsurfaceVault,
    BuriedReactor,
    ResearchHabitat,
    PlanetaryRelay,
    SmugglerDen,
    AncientObservatory,
    GeothermalPlant,
    FloodedComplex,
    OrbitalDebrisFall,
    TradingPost,
    ImperialStation,
    UnswornHaven,
    DerelictFreighter,
    DataBuoy,
    AsteroidRefinery,
    SilentColony,
    SiegeRuin,
    AnomalyLabyrinth,
    CourtAnchorage,
};

enum class SiteModuleKind : std::uint8_t {
    Entry = 0,
    Habitation,
    Storage,
    Power,
    Utility,
    Workshop,
    Research,
    Security,
    Trade,
    Medical,
    Hangar,
    Agriculture,
    Archive,
    Salvage,
    RiftPortal,
    Reactor,
    Sensor,
    Command,
};

enum class SiteLifecycleState : std::uint8_t {
    GeneratedUnknown = 0,
    ActiveStrategic,
    ActiveDetailed,
    BesiegedDisrupted,
    Abandoned,
    RuinedContaminated,
    OccupiedReused,
    Reclaimed,
};

enum class SiteObjectState : std::uint8_t {
    Intact = 0,
    Opened = 1,
    Looted = 2,
    Disabled = 3,
    Destroyed = 4,
    Tombstoned = 5,
};

enum class SettlementScale : std::uint8_t {
    None = 0,
    Camp,
    Hamlet,
    FrontierTown,
    ImperialEnclave,
    UnswornHaven,
    PlayerOutpost,
};

enum SettlementService : std::uint32_t {
    SettlementServiceNone = 0,
    SettlementServiceTrade = 1u << 0,
    SettlementServiceRumors = 1u << 1,
    SettlementServiceDirections = 1u << 2,
    SettlementServiceContracts = 1u << 3,
    SettlementServiceFood = 1u << 4,
    SettlementServiceRepair = 1u << 5,
    SettlementServiceSpecialist = 1u << 6,
    SettlementServiceMarket = 1u << 7,
    SettlementServiceShipPad = 1u << 8,
    SettlementServiceFaction = 1u << 9,
    SettlementServiceInspection = 1u << 10,
    SettlementServiceCharts = 1u << 11,
    SettlementServiceSalvage = 1u << 12,
    SettlementServiceBlackMarket = 1u << 13,
    SettlementServiceHousing = 1u << 14,
    SettlementServiceAutomation = 1u << 15,
};

struct SiteModuleSpec {
    SiteModuleKind kind{SiteModuleKind::Entry};
    bool required{true};
    std::uint8_t weight{255}; // optional-module inclusion weight / 255
};

struct SiteKitDefinition {
    SiteFamily family{SiteFamily::SupplyCache};
    SiteDomain domain{SiteDomain::Surface};
    SiteFaction defaultFaction{SiteFaction::None};
    SiteArchitecture architecture{SiteArchitecture::Frontier};
    SettlementScale settlementScale{SettlementScale::None};
    std::vector<SiteModuleSpec> modules;
};

struct SiteModuleInstance {
    std::uint64_t stableObjectId{};
    SiteModuleKind kind{SiteModuleKind::Entry};
    std::uint8_t variant{};
    std::uint8_t damage{}; // deterministic baseline damage 0..100

    friend bool operator==(const SiteModuleInstance&, const SiteModuleInstance&) = default;
};

struct SettlementProfile {
    SettlementScale scale{SettlementScale::None};
    int minPopulation{};
    int maxPopulation{};
    int baselinePopulation{};
    std::uint32_t services{SettlementServiceNone};
    SiteFaction faction{SiteFaction::None};

    bool has(SettlementService service) const { return (services & static_cast<std::uint32_t>(service)) != 0; }
    friend bool operator==(const SettlementProfile&, const SettlementProfile&) = default;
};

struct SiteDescriptor {
    std::uint64_t stableId{};
    SiteFamily family{SiteFamily::SupplyCache};
    SiteDomain domain{SiteDomain::Surface};
    SiteFaction owner{SiteFaction::None};
    SiteArchitecture architecture{SiteArchitecture::Frontier};
    SiteLifecycleState lifecycle{SiteLifecycleState::GeneratedUnknown};
    std::optional<SurfaceCellAddress> anchor;
    std::uint32_t strategicSlot{}; // orbit/space site slot; zero for surface sites
    std::uint8_t quarterTurns{};
    std::uint8_t damage{};
    std::uint8_t contamination{};
    std::uint64_t lootSeed{};
    std::uint64_t encounterSeed{};
    std::uint64_t historicalOwnerId{};
    SettlementProfile settlement{};
    std::vector<SiteModuleInstance> modules;

    friend bool operator==(const SiteDescriptor&, const SiteDescriptor&) = default;
};

struct SiteSpacingPolicy {
    int placementCellColumns{12};
    int minSpacingColumns{8};
    int maxSitesPerQuery{12};
    std::uint8_t candidateChance{96}; // /255; tuning
};

struct SiteSurfaceQuery {
    CubeFace face{CubeFace::PositiveZ};
    int uBegin{};
    int vBegin{};
    int uEnd{}; // exclusive
    int vEnd{}; // exclusive
};

struct SitePersistentState {
    std::uint64_t stableId{};
    SiteLifecycleState lifecycle{SiteLifecycleState::GeneratedUnknown};
    SiteFaction owner{SiteFaction::None};
    int populationOverride{-1};
    std::uint8_t addedDamage{};
    std::uint8_t contamination{};
    bool discovered{};
    std::uint64_t historicalOwnerId{};
    std::map<std::uint64_t, SiteObjectState> objectStates;
};

struct SiteHistoryEvent {
    std::uint64_t eventId{};
    std::uint64_t siteId{};
    SiteLifecycleState from{SiteLifecycleState::GeneratedUnknown};
    SiteLifecycleState to{SiteLifecycleState::GeneratedUnknown};
    SiteFaction actor{SiteFaction::None};
    std::uint64_t sequence{};
};

class SiteCatalog {
public:
    SiteCatalog();
    const SiteKitDefinition* find(SiteFamily family) const;
    const std::vector<SiteKitDefinition>& all() const { return kits_; }

private:
    std::vector<SiteKitDefinition> kits_;
};

class SiteGenerator {
public:
    static constexpr std::uint32_t GeneratorVersion = 1;
    static constexpr std::uint64_t GeneratorFingerprint = 0x504F495349544531ULL; // "POISITE1"

    explicit SiteGenerator(SiteCatalog catalog = {});

    std::vector<SiteDescriptor> generateSurface(const PlanetSurface& planet,
                                                const SiteSurfaceQuery& query,
                                                const SiteSpacingPolicy& policy = {}) const;
    std::vector<SiteDescriptor> generateStrategic(std::uint64_t systemSeed,
                                                  std::uint32_t bodyIndex,
                                                  int maxSites = 4) const;
    SiteDescriptor assemble(std::uint64_t worldSeed,
                            SiteFamily family,
                            std::optional<SurfaceCellAddress> anchor,
                            std::uint32_t strategicSlot = 0) const;

private:
    SiteCatalog catalog_;

    SiteFamily chooseSurfaceFamily(std::uint64_t h) const;
    SiteDescriptor assembleFromKit(std::uint64_t worldSeed,
                                   const SiteKitDefinition& kit,
                                   std::optional<SurfaceCellAddress> anchor,
                                   std::uint32_t strategicSlot) const;
};

enum class SiteIndexMergeResult : std::uint8_t { Inserted = 0, Duplicate = 1, StableIdConflict = 2 };

// Sparse stable-ID index used when independent streaming/query regions publish
// generated site descriptors. The index never owns voxel state and never uses
// transient ECS handles as identity. Conflicting descriptors with the same
// StableId are rejected loudly so generator/version mistakes cannot silently
// replace a previously accepted baseline.
class SiteIndex {
public:
    SiteIndexMergeResult merge(const SiteDescriptor& descriptor);
    std::size_t merge(const std::vector<SiteDescriptor>& descriptors, std::size_t* conflicts = nullptr);

    const SiteDescriptor* find(std::uint64_t stableId) const;
    std::vector<SiteDescriptor> all() const;
    std::vector<SiteDescriptor> resolved(const class SiteStateStore& state) const;
    std::vector<SiteDescriptor> querySurface(CubeFace face, int uBegin, int vBegin, int uEnd, int vEnd) const;

    std::size_t size() const { return sites_.size(); }
    bool empty() const { return sites_.empty(); }
    std::uint64_t fingerprint() const;

private:
    std::map<std::uint64_t, SiteDescriptor> sites_;
};

class SiteStateStore {
public:
    const SitePersistentState* find(std::uint64_t siteId) const;
    SitePersistentState& touch(const SiteDescriptor& baseline);
    SiteDescriptor apply(const SiteDescriptor& baseline) const;

    bool transition(const SiteDescriptor& baseline, SiteLifecycleState next, SiteFaction actor = SiteFaction::None);
    bool setObjectState(const SiteDescriptor& baseline, std::uint64_t objectId, SiteObjectState state);
    bool setPopulation(const SiteDescriptor& baseline, int population);
    bool setContamination(const SiteDescriptor& baseline, std::uint8_t contamination);
    void markDiscovered(const SiteDescriptor& baseline);

    const std::map<std::uint64_t, SitePersistentState>& states() const { return states_; }
    const std::vector<SiteHistoryEvent>& history() const { return history_; }

    std::string serialize() const;
    bool restore(std::string_view text, std::string* error = nullptr);

    static bool transitionAllowed(SiteLifecycleState from, SiteLifecycleState to);

private:
    std::map<std::uint64_t, SitePersistentState> states_;
    std::vector<SiteHistoryEvent> history_;
    std::uint64_t nextHistorySequence_{1};
};

SettlementProfile settlementProfile(SettlementScale scale, SiteFaction faction, std::uint64_t seed);
const char* siteFamilyName(SiteFamily family);
const char* siteLifecycleName(SiteLifecycleState state);

} // namespace elysium
