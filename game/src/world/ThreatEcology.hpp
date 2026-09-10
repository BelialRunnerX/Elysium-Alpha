// Intended function: imported world implementation for ThreatEcology; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/PlanetSurface.hpp"
#include "world/SurfaceSiege.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

// Fourth Edition Part 28 threat taxonomy. The enum is a diagnostic/content
// category, not an AI behavior switch: active behavior remains composed by the
// combat/environment/navigation systems that consume commands from this seam.
enum class ThreatFamily : std::uint8_t {
    WildlifePredator = 0,
    Raider = 1,
    ImperialEnforcement = 2,
    RiftHorror = 3,
    Titan = 4,
    InfiltratorSyndrome = 5,
    MachineCorruption = 6,
    EnvironmentalCatastrophe = 7,
    CivilUnrest = 8
};

enum class ThreatRepresentation : std::uint8_t {
    StrategicRemote = 0,
    ActiveEncounter = 1
};

enum class ThreatLifecycle : std::uint8_t {
    Dormant = 0,
    Signaled = 1,
    Approaching = 2,
    Engaged = 3,
    Retreating = 4,
    Migrating = 5,
    Contained = 6,
    Defeated = 7,
    Dead = 8,
    Resolved = 9
};

enum class ThreatHistoryEventKind : std::uint8_t {
    Appearance = 0,
    Warning = 1,
    Battle = 2,
    Rampage = 3,
    Breach = 4,
    Retreat = 5,
    Migration = 6,
    Containment = 7,
    Defeat = 8,
    Death = 9
};

enum class RiftBodyPlan : std::uint8_t {
    Quadruped = 0,
    Serpentine = 1,
    Radial = 2,
    Flyer = 3,
    Burrower = 4,
    ManyLimbed = 5,
    Amorphous = 6
};

enum class RiftTissue : std::uint8_t {
    Flesh = 0,
    Chitin = 1,
    Crystal = 2,
    Metal = 3,
    Glass = 4,
    PlasmaSheath = 5,
    Fungal = 6,
    VoidComposite = 7
};

enum class RiftScale : std::uint8_t {
    LargePredator = 0,
    Siege = 1,
    Titan = 2
};

enum class RiftLocomotion : std::uint8_t {
    Walk = 0,
    Climb = 1,
    Burrow = 2,
    Fly = 3,
    PhaseStep = 4,
    Swim = 5,
    WallCrawl = 6
};

enum class RiftDefense : std::uint8_t {
    ArmorPlates = 0,
    Regeneration = 1,
    ShieldOrgan = 2,
    Dispersal = 3,
    Camouflage = 4,
    ReactiveCarapace = 5
};

enum class RiftAttackVerb : std::uint8_t {
    Breach = 0,
    Grab = 1,
    Spit = 2,
    Beam = 3,
    Cloud = 4,
    Charge = 5,
    SummonSpawn = 6,
    DrainPower = 7,
    CorruptMachines = 8
};

enum class RiftEmission : std::uint8_t {
    AcidMist = 0,
    Spores = 1,
    Radiation = 2,
    NeuralEffect = 3,
    Heat = 4,
    CryogenicWake = 5,
    DimensionalDistortion = 6
};

enum class RiftVulnerability : std::uint8_t {
    Material = 0,
    Environment = 1,
    BodyRegion = 2,
    ExposedPhase = 3,
    SoundSignal = 4,
    PowerState = 5
};

const char* threatFamilyName(ThreatFamily value);
const char* threatLifecycleName(ThreatLifecycle value);
const char* riftBodyPlanName(RiftBodyPlan value);
const char* riftTissueName(RiftTissue value);
const char* riftScaleName(RiftScale value);
const char* riftLocomotionName(RiftLocomotion value);
const char* riftDefenseName(RiftDefense value);
const char* riftAttackVerbName(RiftAttackVerb value);
const char* riftEmissionName(RiftEmission value);
const char* riftVulnerabilityName(RiftVulnerability value);

// Generator identity is save/history compatibility data. Axes are derived from
// independent labels, so adding a future axis need not reorder existing axes.
inline constexpr std::uint32_t kRiftHorrorGeneratorVersion = 1;
inline constexpr std::uint64_t kRiftHorrorGeneratorFingerprint = 0x454C5952483031ULL; // "ELYRH01"

struct RiftHorrorDescriptor {
    std::uint32_t generatorVersion{kRiftHorrorGeneratorVersion};
    std::uint64_t generatorFingerprint{kRiftHorrorGeneratorFingerprint};
    std::uint64_t stableId{};
    std::uint64_t originSiteId{};
    std::uint64_t generationSerial{};
    RiftBodyPlan bodyPlan{RiftBodyPlan::Quadruped};
    RiftTissue tissue{RiftTissue::Flesh};
    RiftScale scale{RiftScale::LargePredator};
    RiftLocomotion locomotion{RiftLocomotion::Walk};
    RiftDefense defense{RiftDefense::ArmorPlates};
    RiftAttackVerb primaryAttack{RiftAttackVerb::Breach};
    RiftAttackVerb secondaryAttack{RiftAttackVerb::Grab};
    RiftEmission emission{RiftEmission::AcidMist};
    RiftVulnerability vulnerability{RiftVulnerability::Material};
    float collisionRadiusMeters{1.6f};
    float combatPower{10.0f};
    float breachPower{4.0f};
    float strategicPower{8.0f};
    std::string name;
    std::string signatureBehavior;
    std::string counterplayHint;

    bool operator==(const RiftHorrorDescriptor&) const = default;
};

class RiftHorrorGenerator {
public:
    static RiftHorrorDescriptor generate(std::uint64_t worldSeed,
                                         std::uint64_t originSiteId,
                                         std::uint64_t generationSerial,
                                         std::uint32_t generatorVersion = kRiftHorrorGeneratorVersion);
};

struct ThreatHistoryEvent {
    std::uint64_t eventId{};
    std::uint64_t threatStableId{};
    std::uint64_t siteId{};
    std::uint64_t campaignTick{};
    ThreatHistoryEventKind kind{ThreatHistoryEventKind::Appearance};
    std::uint64_t relatedStableId{};
    std::string summary;

    bool operator==(const ThreatHistoryEvent&) const = default;
};

struct ThreatRecord {
    std::uint64_t stableId{};
    ThreatFamily family{ThreatFamily::WildlifePredator};
    ThreatLifecycle lifecycle{ThreatLifecycle::Dormant};
    ThreatRepresentation representation{ThreatRepresentation::StrategicRemote};
    std::uint64_t siteId{};
    std::uint64_t originSiteId{};
    std::uint64_t targetStableId{};
    float warningSecondsRemaining{};
    float healthFraction{1.0f};
    float strategicPower{1.0f};
    bool namedHistoricalFigure{};
    std::uint32_t nextHistorySerial{1};
    std::optional<RiftHorrorDescriptor> riftHorror;

    bool operator==(const ThreatRecord&) const = default;
};

struct ThreatWarning {
    std::uint64_t threatStableId{};
    float secondsUntilImpact{};
    std::string headline;
    std::string signal;
    std::string counterplay;
};

// Every major threat family exposes a readable pre-impact signal.  These are
// presentation/response contracts, not spawning rules; the authoritative
// source system still owns when a threat actually exists.
struct ThreatSignalPolicy {
    float minimumReadableWarningSeconds{10.0f};
    std::string signal;
    std::string counterplay;
    bool requiresWarningBeforeIrreversibleImpact{true};
};

ThreatSignalPolicy threatSignalPolicy(ThreatFamily family);

enum class ThreatCommandKind : std::uint8_t {
    SetMilitaryAlert = 0,
    SpawnOrPromoteCombatActor = 1,
    RequestPath = 2,
    DamageStructure = 3,
    DamageActor = 4,
    DrainPower = 5,
    CorruptMachine = 6,
    EmitEnvironmentalHazard = 7,
    ApplyStrategicSiteDamage = 8,
    RetreatActor = 9
};

struct ThreatCommand {
    ThreatCommandKind kind{ThreatCommandKind::SetMilitaryAlert};
    std::uint64_t threatStableId{};
    std::uint64_t targetStableId{};
    std::optional<SurfaceCellAddress> targetCell;
    RiftEmission emission{RiftEmission::AcidMist};
    float magnitude{};
    float radiusMeters{};

    bool operator==(const ThreatCommand&) const = default;
};

struct ThreatUpdateResult {
    bool impactNow{};
    std::vector<ThreatCommand> commands;
    std::vector<ThreatHistoryEvent> history;
};

enum class ThreatResolution : std::uint8_t {
    Retreat = 0,
    Migrate = 1,
    Contain = 2,
    Defeat = 3,
    Kill = 4,
    Resolve = 5
};

struct RemoteThreatContext {
    float siteDefensePower{};
    float populationExposure{1.0f};
    float containmentPower{};
};

struct RemoteThreatOutcome {
    float siteDamage{};
    float threatHealthLost{};
    bool forcedRetreat{};
    bool contained{};
    bool defeated{};

    bool operator==(const RemoteThreatOutcome&) const = default;
};

// Stateless lifecycle helpers. The caller owns ThreatRecord storage and appends
// returned history into the Chronicle/event store. The system owns no ECS
// entities, pathfinder state, voxel data, room graph, or environment fields.
class ThreatLifecycleSystem {
public:
    // Creates a generic persistent threat record for non-Rift families without
    // stealing authority from the system that detected/owns the source event.
    // stableId must come from that authoritative source and must be durable.
    static ThreatUpdateResult createGenericThreat(ThreatRecord& outRecord,
                                                   std::uint64_t stableId,
                                                   ThreatFamily family,
                                                   std::uint64_t originSiteId,
                                                   std::uint64_t targetSiteId,
                                                   float strategicPower,
                                                   float warningSeconds,
                                                   std::uint64_t campaignTick,
                                                   bool namedHistoricalFigure = false);

    static ThreatUpdateResult createRiftThreat(ThreatRecord& outRecord,
                                               const RiftHorrorDescriptor& descriptor,
                                               std::uint64_t targetSiteId,
                                               float warningSeconds,
                                               std::uint64_t campaignTick);

    static ThreatWarning warningFor(const ThreatRecord& record);

    static ThreatUpdateResult advanceApproach(ThreatRecord& record,
                                              float dt,
                                              bool siteDetailed,
                                              const std::optional<SurfaceCellAddress>& spawnCell,
                                              std::uint64_t campaignTick);

    static ThreatUpdateResult promoteToActive(ThreatRecord& record,
                                              const SurfaceCellAddress& spawnCell,
                                              std::uint64_t campaignTick);

    static void demoteToStrategic(ThreatRecord& record);

    static ThreatUpdateResult resolve(ThreatRecord& record,
                                      ThreatResolution resolution,
                                      std::uint64_t campaignTick,
                                      std::uint64_t relatedStableId = 0);

    static RemoteThreatOutcome resolveRemoteInterval(ThreatRecord& record,
                                                     const RemoteThreatContext& context,
                                                     float elapsedHours,
                                                     std::uint64_t campaignTick,
                                                     std::vector<ThreatHistoryEvent>* history = nullptr);
};

enum class ThreatDefenseNodeKind : std::uint8_t {
    Gate = 0,
    Wall = 1,
    Utility = 2,
    ExposedShaft = 3,
    Tunnel = 4,
    Airlock = 5,
    Terrain = 6,
    Defense = 7,
    Population = 8,
    Objective = 9
};

enum class ThreatObjectiveKind : std::uint8_t {
    Approach = 0,
    Breach = 1,
    Infiltrate = 2,
    DisableUtility = 3,
    DisableDefense = 4,
    AttackPopulation = 5,
    SeizeObjective = 6
};

struct ThreatDefenseNode {
    std::uint64_t stableNodeId{};
    ThreatDefenseNodeKind kind{ThreatDefenseNodeKind::Terrain};
    SurfaceCellAddress address{};
    std::uint64_t targetStableId{};
    float hardness{1.0f};
    float exposure{1.0f};
    float utilityValue{};
    float defenseCoverage{};
    float accessCost{1.0f};
};

struct ThreatDefenseGraph {
    // The graph is supplied by the fortress/spatial systems as an already
    // bounded coarse view. Threat planning never enumerates planet columns.
    std::vector<ThreatDefenseNode> nodes;
};

struct ThreatSiegePlan {
    ThreatObjectiveKind objective{ThreatObjectiveKind::Approach};
    std::uint64_t targetNodeId{};
    std::uint64_t targetStableId{};
    SurfaceCellAddress targetCell{};
    float score{};
    std::string reason;
};

struct ThreatPlannerTuning {
    std::size_t maxNodesExamined{64};
    std::size_t maxPlans{3};
};

struct ThreatPlannerTelemetry {
    std::size_t nodesExamined{};
    std::size_t nodesRejectedByCapability{};
    std::size_t plansGenerated{};
};

// Coarse siege capabilities are derived from family + generated threat
// composition.  The planner uses this profile to avoid omnipotent enemies:
// dig/build/breach/infiltration remain faction/tier/creature dependent.
struct ThreatCapabilityProfile {
    std::uint8_t siegeTier{};
    bool canBreach{};
    bool canInfiltrate{};
    bool canDisableUtility{};
    bool canDisableDefense{};
    bool canAttackPopulation{};
    bool canSeizeObjective{};
    bool canBurrow{};
    bool canPhase{};
    bool canFly{};
    bool canDrainPower{};
    bool canCorruptMachines{};
    float structuralPowerScale{1.0f};

    bool operator==(const ThreatCapabilityProfile&) const = default;
};

ThreatCapabilityProfile threatCapabilityProfile(const ThreatRecord& threat);
bool threatCanPursueObjective(const ThreatCapabilityProfile& profile, ThreatObjectiveKind objective);

class ThreatSiegePlanner {
public:
    explicit ThreatSiegePlanner(ThreatPlannerTuning tuning = {});

    std::vector<ThreatSiegePlan> plan(const ThreatRecord& threat,
                                     const ThreatDefenseGraph& graph);

    std::vector<ThreatCommand> commandsForPlan(const ThreatRecord& threat,
                                               const ThreatSiegePlan& plan) const;

    const ThreatPlannerTelemetry& telemetry() const { return telemetry_; }

private:
    ThreatPlannerTuning tuning_{};
    ThreatPlannerTelemetry telemetry_{};
};

// Generic mirror of the already-existing Register Action director. It allows
// fortress threat UI/history/alert code to reason about Imperial enforcement
// without replacing or duplicating SurfaceSiegeDirector.
ThreatRecord mirrorImperialThreat(const SurfaceSiegeState& state, std::uint64_t siteId);
ThreatWarning imperialThreatWarning(const SurfaceSiegeState& state);

std::string serializeRiftHorrorDescriptor(const RiftHorrorDescriptor& descriptor);
std::optional<RiftHorrorDescriptor> deserializeRiftHorrorDescriptor(std::string_view text,
                                                                    std::string* error = nullptr);
std::string serializeThreatRecord(const ThreatRecord& record);
std::optional<ThreatRecord> deserializeThreatRecord(std::string_view text,
                                                    std::string* error = nullptr);

} // namespace elysium
