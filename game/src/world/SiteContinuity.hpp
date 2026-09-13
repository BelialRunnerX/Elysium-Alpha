// Intended function: imported world implementation for SiteContinuity; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/InfrastructureJournal.hpp"
#include "world/PlanetSurface.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

// Fourth-Edition retirement/reclamation bridge. This module deliberately keeps
// long-lived identity independent of EnTT entity values and keeps spatial state
// sparse/address-keyed. It is a portable record/transaction seam for the
// coworker's entity/history stores, not a second hidden world manager.

enum class RetiredSiteCondition : std::uint8_t {
    Occupied = 0,
    Damaged = 1,
    Contaminated = 2,
    Abandoned = 3,
    Ruined = 4,
    Transformed = 5
};

enum class ContinuityEventType : std::uint8_t {
    FortressRetired = 0,
    PopulationBirths = 1,
    PopulationDeaths = 2,
    MigrationIn = 3,
    MigrationOut = 4,
    Production = 5,
    OwnershipChanged = 6,
    ThreatIncident = 7,
    InfrastructureDestroyed = 8,
    FigureBorn = 9,
    FigureDied = 10,
    ArtifactMoved = 11,
    InstitutionChanged = 12,
    SiteConditionChanged = 13,
    SiteReclaimed = 14,
    DirectOperativeVisit = 15
};

enum class ArtifactLocationKind : std::uint8_t {
    AtSite = 0,
    HeldByFigure = 1,
    InTransit = 2,
    Lost = 3,
    Destroyed = 4
};

enum class ContinuityInstitutionKind : std::uint8_t {
    Government = 0,
    Guild = 1,
    Faith = 2,
    Hospital = 3,
    Archive = 4,
    Military = 5,
    Trade = 6,
    Other = 7
};

enum class SiteActivationMode : std::uint8_t {
    FortressReclaim = 0,
    DirectOperativeVisit = 1
};

struct ContinuityHistoryEvent {
    std::uint64_t eventId{};
    std::uint64_t day{};
    ContinuityEventType type{ContinuityEventType::FortressRetired};
    std::uint64_t subjectStableId{};
    std::uint64_t secondaryStableId{};
    std::int64_t amount{};
    std::uint64_t causeEventId{};

    friend bool operator==(const ContinuityHistoryEvent&, const ContinuityHistoryEvent&) = default;
};

struct NamedFigureRecord {
    std::uint64_t stableId{};
    std::uint64_t currentSiteId{};
    std::uint64_t organizationId{};
    std::uint64_t parentAStableId{};
    std::uint64_t parentBStableId{};
    std::uint32_t ageDays{};
    bool alive{true};
    std::vector<std::uint64_t> historyRefs;

    friend bool operator==(const NamedFigureRecord&, const NamedFigureRecord&) = default;
};

struct InstitutionContinuityRecord {
    std::uint64_t stableId{};
    std::uint64_t siteId{};
    std::uint64_t organizationId{};
    ContinuityInstitutionKind kind{ContinuityInstitutionKind::Other};
    bool active{true};
    std::vector<std::uint64_t> historyRefs;

    friend bool operator==(const InstitutionContinuityRecord&, const InstitutionContinuityRecord&) = default;
};

struct ArtifactContinuityRecord {
    std::uint64_t artifactId{};
    ArtifactLocationKind locationKind{ArtifactLocationKind::AtSite};
    std::uint64_t siteId{};
    std::uint64_t holderStableId{};
    std::uint64_t ownerOrganizationId{};
    std::optional<SurfaceCellAddress> anchor;
    std::vector<std::uint64_t> historyRefs;

    friend bool operator==(const ArtifactContinuityRecord&, const ArtifactContinuityRecord&) = default;
};

struct RetiredMacroCellState {
    SurfaceCellAddress address{};
    BlockType type{BlockType::Air};
    bool playerPlaced{};

    friend bool operator==(const RetiredMacroCellState&, const RetiredMacroCellState&) = default;
};

struct RetiredMicroCellState {
    SurfaceCellAddress address{};
    int microIndex{};
    BlockType type{BlockType::Air};

    friend bool operator==(const RetiredMicroCellState&, const RetiredMicroCellState&) = default;
};

// Persistent inputs supplied by citizen/history/governance agents when a site
// is retired. The standalone prototype does not yet own those full systems, so
// the bridge accepts their durable records instead of inventing parallel ECS
// state.
struct ActiveSiteContinuityInput {
    std::uint64_t siteId{};
    std::uint64_t ownerOrganizationId{};
    std::uint64_t claimBeaconStableId{};
    std::uint64_t currentDay{};
    int anonymousPopulation{};
    std::int64_t productionStock{};
    float contamination{};
    std::vector<NamedFigureRecord> namedFigures;
    std::vector<InstitutionContinuityRecord> institutions;
    std::vector<ArtifactContinuityRecord> artifacts;
    std::vector<ContinuityHistoryEvent> priorHistory;

    // Optional site-owned spatial scope. Empty preserves the standalone
    // prototype behavior and captures every currently touched chunk because
    // v0.20 has one active site context. A multi-site planet should supply the
    // exact sparse set of chunks owned by this site so retirement does not
    // duplicate unrelated player edits into multiple site records. The set is
    // address keyed; no dense planet mask is permitted.
    std::vector<PlanetChunkAddress> spatialChunks;
};

struct RetiredSiteRecord {
    static constexpr std::uint32_t SchemaVersion = 1;

    std::uint32_t schemaVersion{SchemaVersion};
    std::uint64_t siteId{};
    std::uint64_t planetSeed{};
    int generatorVersion{PlanetSurface::GeneratorVersion};
    std::uint64_t generatorFingerprint{PlanetSurface::GeneratorFingerprint};
    PlanetClass planetClass{PlanetClass::Temperate};
    float referenceRadius{48.0f};

    std::uint64_t retiredDay{};
    std::uint64_t strategicDay{};
    std::uint64_t ownerOrganizationId{};
    std::uint64_t claimBeaconStableId{};
    RetiredSiteCondition condition{RetiredSiteCondition::Occupied};

    int anonymousPopulation{};
    std::int64_t productionStock{};
    float contamination{};
    double birthAccumulator{};
    double deathAccumulator{};
    double migrationAccumulator{};
    double productionAccumulator{};
    double ownershipPressureAccumulator{};
    std::uint32_t threatDamageCount{};

    // Stable-ID allocation is local to this retirement record only for records
    // created while the site is remote. It never exposes or serializes an ECS
    // handle and is deterministic from site identity + serial.
    std::uint64_t nextRemoteIdentitySerial{1};
    std::uint64_t nextEventSerial{1};

    std::vector<NamedFigureRecord> namedFigures;
    std::vector<InstitutionContinuityRecord> institutions;
    std::vector<ArtifactContinuityRecord> artifacts;
    std::vector<ContinuityHistoryEvent> history;

    // Sparse, address-keyed spatial snapshot. These are the retirement-time
    // player-authored deltas, not a materialized procedural planet.
    std::vector<RetiredMacroCellState> macroCells;
    std::vector<RetiredMicroCellState> microCells;

    // Stable infrastructure upserts plus later strategic tombstones. Geometry
    // remains in macro/micro world deltas; object records never own voxels.
    std::vector<InfrastructureJournalRecord> infrastructureJournal;
};

struct StrategicAdvanceInputs {
    // Demographic rates are intentionally tuning inputs. Accumulators make the
    // result independent of whether 100 days are advanced in one call or 100
    // one-day calls.
    double birthsPerThousandPerDay{0.25};
    double deathsPerThousandPerDay{0.10};
    double migrationPeoplePerDay{}; // signed
    double productionPerPersonPerDay{0.08};

    // Threat roll is deterministic per site/day. A threat can damage one
    // stable infrastructure dependency-closure but does not invent voxel edits.
    double threatChancePerDay{}; // [0,1]
    int maxPopulationLossPerThreat{2};
    float contaminationPerThreat{};

    // Political takeover is an explicit strategic pressure rather than a magic
    // random owner swap. Once accumulated pressure reaches one, ownership moves
    // to the specified organization and a Chronicle event is emitted.
    std::uint64_t occupyingOrganizationId{};
    double ownershipPressurePerDay{};
};

struct SiteActivationPlan {
    SiteActivationMode mode{SiteActivationMode::FortressReclaim};
    std::uint64_t siteId{};
    bool activateAnonymousPopulation{};
    int anonymousPopulationToPromote{};
    std::vector<std::uint64_t> namedFigureStableIds;
    std::vector<std::uint64_t> artifactIds;
    std::vector<std::uint64_t> institutionStableIds;
};

struct SiteContinuityAudit {
    bool valid{};
    int survivingInfrastructureObjects{};
    int infrastructureTombstones{};
    int namedFiguresAtSite{};
    int artifactsAtSite{};
    int activeInstitutions{};
    std::string error;
};

class SiteContinuitySystem {
public:
    // Freeze active local truth into a strategic record. This does not perform
    // filesystem publication; Agent-3/coworker persistence owns the atomic
    // transaction around this payload.
    static bool retire(const PlanetSurface& planet,
                       const SurfaceInfrastructure& infrastructure,
                       const ActiveSiteContinuityInput& input,
                       RetiredSiteRecord& out,
                       std::string* error = nullptr);

    // Strategic behavioral-LOD advancement. Results are deterministic from the
    // retired record, day, and supplied pressures; no wall clock/global RNG is
    // consulted.
    static bool advance(RetiredSiteRecord& site,
                        std::uint32_t days,
                        const StrategicAdvanceInputs& inputs,
                        std::string* error = nullptr);

    static std::uint64_t recordNamedDescendantBirth(RetiredSiteRecord& site,
                                                     std::uint64_t parentAStableId,
                                                     std::uint64_t parentBStableId = 0,
                                                     std::uint64_t organizationId = 0,
                                                     std::string* error = nullptr);
    static bool recordFigureDeath(RetiredSiteRecord& site,
                                  std::uint64_t figureStableId,
                                  std::string* error = nullptr);
    static bool moveArtifact(RetiredSiteRecord& site,
                             std::uint64_t artifactId,
                             ArtifactLocationKind location,
                             std::uint64_t siteId,
                             std::uint64_t holderStableId = 0,
                             std::optional<SurfaceCellAddress> anchor = std::nullopt,
                             std::string* error = nullptr);
    static bool setInstitutionActive(RetiredSiteRecord& site,
                                     std::uint64_t institutionStableId,
                                     bool active,
                                     std::string* error = nullptr);
    static bool setCondition(RetiredSiteRecord& site,
                             RetiredSiteCondition condition,
                             std::string* error = nullptr);

    // Strategic destruction uses stable infrastructure identity. Dependents are
    // tombstoned explicitly so reclamation cannot resurrect a dangling airlock
    // assembly/automation rule merely because its owner object disappeared.
    static bool destroyInfrastructure(RetiredSiteRecord& site,
                                      InfrastructureRecordKind kind,
                                      std::uint64_t stableId,
                                      std::string* error = nullptr);

    // Optional explicit remote world damage/repair. The mutation updates the
    // final sparse retirement snapshot and is history-visible through the
    // caller-supplied cause event if desired; procedural baseline is untouched.
    static bool setRemoteMacroCell(RetiredSiteRecord& site,
                                   SurfaceCellAddress address,
                                   BlockType type,
                                   bool playerPlaced,
                                   std::string* error = nullptr);

    static SiteActivationPlan activationPlan(const RetiredSiteRecord& site,
                                             SiteActivationMode mode);

    // Rebuild detailed spatial/infrastructure state from generator + deltas and
    // perform a deterministic consistency audit before returning an activation
    // plan. Existing identical infrastructure is treated idempotently; a live
    // object with the same StableId but different persistent state is an
    // explicit conflict, never a silent duplicate.
    static bool reclaim(RetiredSiteRecord& site,
                        PlanetSurface& planet,
                        SurfaceInfrastructure& infrastructure,
                        SiteActivationMode mode,
                        SiteActivationPlan* plan = nullptr,
                        SiteContinuityAudit* audit = nullptr,
                        std::string* error = nullptr);

    static SiteContinuityAudit audit(const RetiredSiteRecord& site);
    // Human-readable strategic/continuity inspector used by tooling and the
    // future Galactic Chronicle/why-inspector surfaces. This reports durable
    // facts only; it never materializes a hidden live simulation.
    static std::string inspect(const RetiredSiteRecord& site);
    static bool validate(const RetiredSiteRecord& site, std::string* error = nullptr);

    // Deterministic narrow codec for this subsystem payload. The outer save
    // transaction/checksum/generation policy remains owned by persistence.
    static std::string serialize(const RetiredSiteRecord& site);
    static std::optional<RetiredSiteRecord> deserialize(std::string_view text,
                                                         std::string* error = nullptr);
};

const char* retiredSiteConditionName(RetiredSiteCondition condition);
const char* continuityEventTypeName(ContinuityEventType type);

} // namespace elysium
