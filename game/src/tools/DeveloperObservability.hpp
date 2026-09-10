// Intended function: imported tools implementation for DeveloperObservability; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "core/Diagnostics.hpp"
#include "core/JobSystem.hpp"
#include "render/PlanetSurfaceRenderer.hpp"
#include "world/InfrastructureJournal.hpp"
#include "world/SurfaceChunkPersistence.hpp"
#include "world/SurfaceIndustry.hpp"
#include "world/SurfaceNavigation.hpp"
#include "world/SurfaceWorldRead.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace elysium {

// Developer-facing stable object classification. The lookup deliberately
// operates on StableId and authoritative subsystem snapshots, never transient ECS
// entity handles or renderer handles.
enum class StableObjectKind : std::uint8_t {
    Missing = 0,
    Machine = 1,
    Portal = 2,
    AirlockAssembly = 3,
    AutomationRule = 4
};

struct StableObjectInspection {
    std::uint64_t stableId{};
    StableObjectKind kind{StableObjectKind::Missing};
    bool loaded{};
    SurfaceCellAddress ownerAddress{};
    std::string label;
};

// One common shape for the spec's "Why" panel. It is a reconstructed view;
// none of these strings are authoritative simulation state.
struct WhyInspection {
    StableObjectInspection subject;
    std::vector<std::string> currentState;
    std::vector<std::string> relevantInputs;
    std::vector<std::string> lastDecisions;
    std::vector<std::string> blockers;
    std::vector<std::string> dependencies;
    std::vector<std::string> remediation;
};

struct RoomAtmosphereInspection {
    std::uint64_t sourceStableId{};
    bool sourceLoaded{};
    SurfaceCellAddress anchor{};
    bool sealed{};
    bool truncated{};
    bool leakedToSky{};
    bool hasLeakFrontier{};
    SurfaceCellAddress leakFrontier{};
    std::size_t reachableCells{};
    float pressure{};
    float oxygen{};
    bool enabled{};
    bool powered{};
    std::string cause;
};

struct PowerNodeInspection {
    std::uint64_t stableId{};
    MachineType type{MachineType::StorageCrate};
    bool enabled{};
    bool powered{};
    float generation{};
    float demand{};
    int priority{};
    float storedEnergy{};
    float storageCapacity{};
};

struct PowerNetworkInspection {
    std::uint64_t networkId{};
    float generation{};
    float demand{};
    float supplied{};
    float storedEnergy{};
    float storageCapacity{};
    bool brownout{};
    std::vector<PowerNodeInspection> nodes;
};

struct PowerGraphInspection {
    PowerNetworkSummary authoritativeSummary{};
    std::vector<PowerNetworkInspection> networks;
};

struct LogisticsEdgeInspection {
    std::uint64_t transportStableId{};
    MachineType transportType{MachineType::Conveyor};
    std::uint64_t sourceStableId{};
    std::uint64_t targetStableId{};
    std::uint64_t alternateTargetStableId{};
    bool sourceLoaded{};
    bool targetLoaded{};
    bool alternateLoaded{};
    bool enabled{};
    bool powered{};
    bool blocked{};
    int sourceStacks{};
    int targetStacks{};
    int targetStackCapacity{};
    std::string bottleneck;
};

struct LogisticsGraphInspection {
    std::vector<LogisticsEdgeInspection> edges;
};

struct PathInspection {
    std::uint64_t worldRevision{};
    std::uint64_t cacheRevisionBefore{};
    std::uint64_t cacheRevisionAfter{};
    bool cacheWasStale{};
    bool cacheInvalidatedByQuery{};
    bool found{};
    bool fromCache{};
    int expanded{};
    int maxExpanded{};
    std::vector<SurfaceCellAddress> routeColumns;
    std::string cause;
};

struct ChunkInspection {
    PlanetChunkAddress address{};
    std::uint64_t chunkRevision{};
    int macroEdits{};
    int placedMarkers{};
    int refinedCells{};
    std::size_t microOverrides{};
    EditInfluenceSummary influence{};
};

struct PersistenceDeltaCell {
    SurfaceCellAddress address{};
    BlockType baseline{BlockType::Air};
    BlockType current{BlockType::Air};
    bool playerPlaced{};
};

struct PersistenceDiffInspection {
    PlanetChunkAddress address{};
    std::uint64_t worldRevision{};
    std::vector<PersistenceDeltaCell> macroDeltas;
    int refinedCells{};
    std::size_t microOverrides{};
    std::vector<std::uint64_t> machineIds;
    std::vector<std::uint64_t> portalIds;
};

struct InfrastructureJournalInspection {
    int upserts{};
    int tombstones{};
    int invalidRecords{};
    std::vector<std::string> issues;
};

struct WorkerTelemetryInspection {
    JobSystemStats jobs{};
};

struct RendererTelemetryInspection {
    int pendingJobs{};
    int dirtyChunks{};
    int rebuiltChunksLastSync{};
    int quads{};
    int triangles{};
    int fullDetailChunks{};
    int nearFieldChunks{};
    int farFieldChunks{};
    SurfaceChunkCacheStats cache{};
};

// Adapter contract for fortress jobs/work orders. Agent 43 does not own the
// simulation state itself; future job systems provide this immutable snapshot
// and the debugger explains it without re-running assignment logic.
struct JobCandidateDiagnostic {
    std::uint64_t workerStableId{};
    bool eligible{};
    float score{};
    std::vector<std::string> rejectionReasons;
};

struct ReservationDiagnostic {
    std::uint64_t resourceStableId{};
    std::uint64_t ownerJobStableId{};
    bool acquired{};
    std::string failureReason;
};

struct JobDiagnosticSnapshot {
    std::uint64_t jobStableId{};
    std::string jobType;
    std::string state;
    std::vector<JobCandidateDiagnostic> candidates;
    std::vector<ReservationDiagnostic> reservations;
    std::vector<std::uint64_t> unmetDependencyIds;
    bool pathRequested{};
    bool pathFound{};
    std::string pathFailureReason;
    std::string explicitFailureReason;
};

struct JobDebugInspection {
    std::uint64_t jobStableId{};
    std::string summary;
    std::vector<std::string> blockers;
    std::vector<std::uint64_t> eligibleWorkers;
};



struct ComponentDiagnosticValue {
    std::string component;
    std::string value;
};

struct EntityDiagnosticSnapshot {
    std::uint64_t stableId{};
    bool loaded{};
    bool remote{};
    std::string kind;
    std::string shard;
    std::vector<ComponentDiagnosticValue> components;
};

struct EntityInspection {
    std::uint64_t stableId{};
    bool loaded{};
    bool remote{};
    std::string kind;
    std::string shard;
    std::vector<ComponentDiagnosticValue> components;
    std::string lookupNote;
};

struct GraphicsTelemetrySnapshot {
    std::uint64_t residentGpuBytes{};
    std::uint64_t uploadedBytesThisFrame{};
    int drawCalls{};
    int meshUploads{};
    int meshDestroys{};
    int triangles{};
    std::vector<std::string> backendWarnings;
};

struct GraphicsTelemetryInspection {
    GraphicsTelemetrySnapshot snapshot{};
};

// Snapshot adapters for Fourth Edition domains that are not owned by the
// observability layer. Their source systems fill these immutable records; the
// tools only sort/explain them. This lets future citizen/history/medical code
// integrate without a parallel debug database.
struct CitizenDiagnosticSnapshot {
    std::uint64_t stableId{};
    bool loaded{};
    bool remote{};
    std::string name;
    std::string state;
    std::uint64_t currentJobStableId{};
    float stress{};
    float focus{1.0f};
    std::vector<std::string> influentialNeeds;
    std::vector<std::string> influentialMemories;
    std::vector<std::string> capabilityLimits;
    std::vector<std::string> explicitAttentionReasons;
};

struct CitizenInspection {
    std::uint64_t stableId{};
    bool loaded{};
    bool remote{};
    std::string summary;
    std::vector<std::string> attentionReasons;
};

struct RelationshipEdgeDiagnostic {
    std::uint64_t sourceStableId{};
    std::uint64_t targetStableId{};
    std::string type;
    float affinity{};
    float trust{};
    float respect{};
    float fear{};
    std::string grievance;
};

struct RelationshipGraphInspection {
    std::uint64_t focusStableId{};
    std::vector<RelationshipEdgeDiagnostic> edges;
    std::vector<std::uint64_t> referencedStableIds;
};

struct EnvironmentalVolumeDiagnosticSnapshot {
    std::uint64_t stableId{};
    bool loaded{};
    std::string kind;
    SurfaceCellAddress anchor{};
    std::size_t cells{};
    bool bounded{};
    bool truncated{};
    float pressure{};
    float oxygen{};
    float temperature{};
    float contamination{};
    std::vector<std::string> activeSources;
};

struct EnvironmentalVolumeInspection {
    std::uint64_t stableId{};
    std::string summary;
    std::vector<std::string> hazards;
};

struct BodyPartDiagnostic {
    std::string part;
    float condition{1.0f};
    bool missing{};
    std::vector<std::string> wounds;
    std::vector<std::string> capabilityLosses;
    std::string prosthetic;
};

struct MedicalDiagnosticSnapshot {
    std::uint64_t citizenStableId{};
    bool loaded{};
    float bloodOrVitalReserve{1.0f};
    float oxygen{1.0f};
    float pain{};
    float shock{};
    bool urgent{};
    std::vector<BodyPartDiagnostic> bodyParts;
    std::vector<std::string> diagnoses;
    std::vector<std::string> treatmentPlan;
};

struct MedicalInspection {
    std::uint64_t citizenStableId{};
    bool urgent{};
    std::vector<std::string> blockers;
    std::vector<std::string> treatmentPlan;
};

struct MilitaryReadinessSnapshot {
    std::uint64_t squadStableId{};
    int assignedMembers{};
    int presentMembers{};
    int equippedMembers{};
    int trainedMembers{};
    int requiredAmmo{};
    int availableAmmo{};
    bool alertActive{};
    std::vector<std::string> explicitFaults;
};

struct MilitaryReadinessInspection {
    std::uint64_t squadStableId{};
    float readiness{};
    std::vector<std::string> blockers;
};

struct ChronicleEventDiagnostic {
    std::uint64_t eventStableId{};
    std::uint64_t timeKey{};
    std::string type;
    std::uint64_t siteStableId{};
    std::vector<std::uint64_t> participantStableIds;
    std::string summary;
    std::vector<std::uint64_t> causalEventIds;
};

struct ChronicleInspection {
    std::uint64_t focusStableId{};
    std::vector<ChronicleEventDiagnostic> events;
};

struct WorldgenSurfaceSample {
    SurfaceCellAddress surface{};
    BlockType material{BlockType::Air};
};

struct WorldgenWindowInspection {
    CubeFace face{CubeFace::PositiveZ};
    int uBegin{};
    int vBegin{};
    int width{};
    int height{};
    bool truncated{};
    std::vector<WorldgenSurfaceSample> samples;
};

struct CrossSectionSample {
    SurfaceCellAddress address{};
    BlockType material{BlockType::Air};
};

struct CrossSectionInspection {
    CubeFace face{CubeFace::PositiveZ};
    int fixedV{};
    int uBegin{};
    int width{};
    int radialBegin{};
    int radialEnd{};
    bool truncated{};
    std::vector<CrossSectionSample> samples;
};

struct MeshPacketInspection {
    PlanetChunkAddress address{};
    bool fieldProxy{};
    int fieldStep{};
    int vertices{};
    int quads{};
    int triangles{};
    int macroQuads{};
    int microQuads{};
    int aoDarkenedCorners{};
    std::vector<MaterialRange> materialRanges;
};

struct ChunkRecordInspection {
    int recordFormatVersion{SurfaceChunkStore::RecordFormatVersion};
    int planetSlot{};
    std::uint64_t planetSeed{};
    int generatorVersion{};
    std::uint64_t generatorFingerprint{};
    std::uint64_t saveGeneration{};
    PlanetChunkAddress address{};
    int macroEdits{};
    int placedMarkers{};
    int refinedCells{};
    std::size_t microOverrides{};
    std::vector<std::uint64_t> machineIds;
    std::vector<std::uint64_t> portalIds;
    std::vector<std::uint64_t> airlockIds;
    std::vector<std::uint64_t> automationRuleIds;
};

struct FramePhaseTelemetrySnapshot {
    double frameMs{};
    double snapshotMs{};
    double senseMs{};
    double planMs{};
    double resolveMs{};
    double commitMs{};
    double persistMs{};
    double presentMs{};
    std::uint64_t ecsEntities{};
    std::uint64_t persistentCommands{};
};

struct FramePhaseTelemetryInspection {
    FramePhaseTelemetrySnapshot snapshot{};
    double simulationMs{};
    double accountedMs{};
};

struct RecordedTraceEvent {
    std::uint64_t sequence{};
    DiagnosticTraceEvent event;
};

// Thread-safe bounded recorder for optional development traces. Overflow drops
// oldest diagnostic events only; it cannot affect simulation state.
class SystemTraceRecorder final : public IDiagnosticTraceSink {
public:
    explicit SystemTraceRecorder(std::size_t capacity = 4096);
    void record(DiagnosticTraceEvent event) override;
    std::vector<RecordedTraceEvent> snapshot() const;
    std::vector<RecordedTraceEvent> forStableId(std::uint64_t stableId) const;
    void clear();
    std::size_t dropped() const;

private:
    std::size_t capacity_{};
    mutable std::mutex mutex_;
    std::deque<RecordedTraceEvent> events_;
    std::uint64_t nextSequence_{1};
    std::size_t dropped_{};
};

class DeveloperObservability {
public:
    static StableObjectInspection lookup(const SurfaceInfrastructure& infrastructure,
                                         std::uint64_t stableId);
    static WhyInspection why(const PlanetSurface& planet,
                             const SurfaceInfrastructure& infrastructure,
                             std::uint64_t stableId,
                             int maxRoomCells = 8192);
    static RoomAtmosphereInspection room(const PlanetSurface& planet,
                                         const SurfaceInfrastructure& infrastructure,
                                         std::uint64_t atmosphereUnitStableId,
                                         int maxRoomCells = 8192);
    static PowerGraphInspection power(const SurfaceInfrastructure& infrastructure);
    static LogisticsGraphInspection logistics(const SurfaceInfrastructure& infrastructure);
    static PathInspection path(const PlanetSurface& planet,
                               const SurfaceNavigationService& navigation,
                               const SurfaceWorldReadService& read,
                               Vec3 start,
                               Vec3 goal,
                               float hoverOffset = 1.8f,
                               float agentRadius = 0.38f,
                               float maxStepHeight = 1.6f);
    static ChunkInspection chunk(const PlanetSurface& planet,
                                 const PlanetChunkAddress& address);
    static PersistenceDiffInspection persistenceDiff(const PlanetSurface& planet,
                                                     const SurfaceInfrastructure& infrastructure,
                                                     const PlanetChunkAddress& address);
    static InfrastructureJournalInspection journal(const std::vector<InfrastructureJournalRecord>& records);
    static WorkerTelemetryInspection workers(const JobSystem& jobs);
    static RendererTelemetryInspection renderer(const PlanetSurfaceRenderer& renderer);
    static JobDebugInspection job(const JobDiagnosticSnapshot& snapshot);
    static EntityInspection entity(const EntityDiagnosticSnapshot& snapshot);
    static GraphicsTelemetryInspection graphics(const GraphicsTelemetrySnapshot& snapshot);
    static CitizenInspection citizen(const CitizenDiagnosticSnapshot& snapshot);
    static RelationshipGraphInspection relationships(std::uint64_t focusStableId,
                                                       const std::vector<RelationshipEdgeDiagnostic>& edges);
    static EnvironmentalVolumeInspection environment(const EnvironmentalVolumeDiagnosticSnapshot& snapshot);
    static MedicalInspection medical(const MedicalDiagnosticSnapshot& snapshot);
    static MilitaryReadinessInspection military(const MilitaryReadinessSnapshot& snapshot);
    static ChronicleInspection chronicle(std::uint64_t focusStableId,
                                         const std::vector<ChronicleEventDiagnostic>& events,
                                         std::size_t limit = 256);
    static WorldgenWindowInspection worldgenWindow(const PlanetSurface& planet,
                                                   CubeFace face,
                                                   int uBegin,
                                                   int vBegin,
                                                   int width,
                                                   int height,
                                                   std::size_t maxSamples = 4096);
    static CrossSectionInspection crossSection(const PlanetSurface& planet,
                                               CubeFace face,
                                               int fixedV,
                                               int uBegin,
                                               int width,
                                               int radialBegin,
                                               int radialEnd,
                                               std::size_t maxSamples = 8192);
    static MeshPacketInspection mesh(const PlanetSurface& planet,
                                     const PlanetChunkAddress& address,
                                     bool fieldProxy = false,
                                     int fieldStep = 4);
    static ChunkRecordInspection chunkRecord(const SurfaceChunkRecord& record);
    static FramePhaseTelemetryInspection framePhases(const FramePhaseTelemetrySnapshot& snapshot);

    static std::string toText(const WhyInspection& report);
    static std::string toText(const PowerGraphInspection& report);
    static std::string toText(const LogisticsGraphInspection& report);
    static std::string toText(const PathInspection& report);
    static std::string toText(const PersistenceDiffInspection& report);
    static std::string toText(const SystemTraceRecorder& recorder);
    static std::string toText(const EntityInspection& report);
    static std::string toText(const GraphicsTelemetryInspection& report);
    static std::string toText(const CitizenInspection& report);
    static std::string toText(const RelationshipGraphInspection& report);
    static std::string toText(const EnvironmentalVolumeInspection& report);
    static std::string toText(const MedicalInspection& report);
    static std::string toText(const MilitaryReadinessInspection& report);
    static std::string toText(const ChronicleInspection& report);
    static std::string toText(const WorldgenWindowInspection& report);
    static std::string toText(const CrossSectionInspection& report);
    static std::string toText(const MeshPacketInspection& report);
    static std::string toText(const ChunkRecordInspection& report);
    static std::string toText(const FramePhaseTelemetryInspection& report);
};

const char* stableObjectKindName(StableObjectKind kind);
std::string surfaceAddressText(SurfaceCellAddress address);
std::string chunkAddressText(PlanetChunkAddress address);

} // namespace elysium
