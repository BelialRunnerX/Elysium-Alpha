// Intended function: imported world implementation for WorldMemoryPersistence; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/CubeSphere.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace elysium {

// Fourth Edition Appendix J persistence seam. These are durable save identities,
// never runtime EnTT handles, pointers, dense planet indices, or renderer IDs.
using EntityStableId = std::uint64_t;
using SiteId = std::uint64_t;
using OrganizationId = std::uint64_t;
using ArtifactId = std::uint64_t;
using HistoricalEventId = std::uint64_t;

enum class PersistentEntityKind : std::uint8_t {
    Citizen = 0,
    UniqueItem,
    Machine,
    Vehicle,
    Institution,
    JobOrder,
    Relationship,
    Site,
    Organization,
    Artifact
};

enum class PersistentShardState : std::uint8_t {
    Active = 0,
    FortressRemote,
    PlanetRemote,
    SystemStrategic,
    GalaxyHistorical
};

enum class ChronicleKnowledge : std::uint8_t {
    Public = 0,
    PlayerKnown,
    FactionKnown,
    Secret
};

struct PersistentContentRef {
    std::string id;
    std::uint32_t schemaVersion{1};

    bool operator==(const PersistentContentRef&) const = default;
};

// Components are intentionally opaque to the persistence layer. Owning systems
// provide versioned codecs; this store only preserves stable component schema IDs
// and their bytes/text payloads. Unknown component kinds can therefore round-trip
// without becoming behavior switches in persistence code.
struct PersistentComponentBlob {
    std::string componentId;
    std::uint32_t schemaVersion{1};
    std::string payload;

    bool operator==(const PersistentComponentBlob&) const = default;
};

struct PersistentSpatialLocation {
    std::uint32_t systemIndex{};
    std::uint32_t planetIndex{};
    SiteId siteId{};
    std::optional<PlanetChunkAddress> chunk;
    double x{};
    double y{};
    double z{};

    bool operator==(const PersistentSpatialLocation&) const = default;
};

struct PersistentEntityRecord {
    static constexpr std::uint32_t SchemaVersion = 1;

    std::uint32_t schemaVersion{SchemaVersion};
    EntityStableId stableId{};
    PersistentEntityKind kind{PersistentEntityKind::Citizen};
    PersistentSpatialLocation location{};
    PersistentShardState shardState{PersistentShardState::Active};
    std::vector<PersistentContentRef> contentRefs;
    std::vector<PersistentComponentBlob> components;
    std::vector<EntityStableId> relationshipRefs;
    std::vector<HistoricalEventId> historyRefs;

    bool operator==(const PersistentEntityRecord&) const = default;
};

struct ChronicleParticipant {
    EntityStableId stableId{};
    std::string role;

    bool operator==(const ChronicleParticipant&) const = default;
};

struct ChronicleLocation {
    std::uint32_t systemIndex{};
    std::uint32_t planetIndex{};
    SiteId siteId{};
    std::optional<PlanetChunkAddress> chunk;

    bool operator==(const ChronicleLocation&) const = default;
};

struct ChronicleEventRecord {
    static constexpr std::uint32_t SchemaVersion = 1;

    std::uint32_t schemaVersion{SchemaVersion};
    HistoricalEventId eventId{};
    std::uint64_t timestamp{};
    std::string typeId;
    ChronicleLocation location{};
    std::vector<ChronicleParticipant> participants;
    std::vector<ArtifactId> objectRefs;
    std::vector<OrganizationId> organizationRefs;
    std::vector<HistoricalEventId> causeRefs;
    std::string payload;
    ChronicleKnowledge knowledge{ChronicleKnowledge::Public};
    std::uint8_t importance{};

    bool operator==(const ChronicleEventRecord&) const = default;
};

// Reclamation must reconstruct the original procedural baseline before applying
// spatial deltas. This record makes the generator contract explicit per site.
struct SiteBaselineRecord {
    SiteId siteId{};
    std::uint64_t planetSeed{};
    int generatorVersion{};
    std::uint64_t generatorFingerprint{};

    bool operator==(const SiteBaselineRecord&) const = default;
};

struct WorldMemorySnapshot {
    static constexpr std::uint32_t SchemaVersion = 1;

    std::uint32_t schemaVersion{SchemaVersion};
    std::uint64_t saveGeneration{};
    std::vector<SiteBaselineRecord> baselines;
    std::vector<PersistentEntityRecord> entities;
    std::vector<ChronicleEventRecord> events;

    bool operator==(const WorldMemorySnapshot&) const = default;
};


struct WorldMemoryDiff {
    std::vector<SiteId> addedBaselines;
    std::vector<SiteId> removedBaselines;
    std::vector<SiteId> changedBaselines;
    std::vector<EntityStableId> addedEntities;
    std::vector<EntityStableId> removedEntities;
    std::vector<EntityStableId> changedEntities;
    std::vector<HistoricalEventId> addedEvents;
    std::vector<HistoricalEventId> removedEvents;
    std::vector<HistoricalEventId> changedEvents;

    bool logicalChanges() const;
};

struct ReclamationAuditResult {
    bool valid{};
    SiteBaselineRecord baseline{};
    std::vector<EntityStableId> siteEntities;
    std::vector<HistoricalEventId> siteEvents;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

// Tooling/retire-reclaim helpers. Diffing is stable-ID based and ignores only
// saveGeneration when deciding whether logical world memory changed. The audit
// verifies that a site can reconstruct its pinned procedural baseline and that
// its local history references are internally resolvable before promotion.
WorldMemoryDiff diffWorldMemorySnapshots(const WorldMemorySnapshot& before,
                                         const WorldMemorySnapshot& after);
ReclamationAuditResult auditWorldMemoryForReclamation(const WorldMemorySnapshot& snapshot,
                                                       SiteId siteId);

struct WorldMemoryLoadResult {
    bool loaded{};
    bool usedPreviousGeneration{};
    std::string error;
    WorldMemorySnapshot snapshot{};
};

// Explicit content-ID rename table required by Third Edition Q.5. Resolution is
// deterministic and transitive; cycles, invalid IDs, and conflicting aliases are
// rejected loudly.
class ContentIdAliasTable {
public:
    bool addAlias(std::string from, std::string to, std::string* error = nullptr);
    std::optional<std::string> resolve(std::string_view id, std::string* error = nullptr) const;
    bool empty() const { return aliases_.empty(); }

private:
    std::unordered_map<std::string,std::string> aliases_;
};

bool validateWorldMemorySnapshot(const WorldMemorySnapshot& snapshot, std::string* error = nullptr);
std::string serializeWorldMemorySnapshot(const WorldMemorySnapshot& snapshot);
WorldMemoryLoadResult parseWorldMemorySnapshot(std::string_view payload);

// Migrates saved content refs and Chronicle event type IDs through an explicit
// alias table. Stable IDs and event identities are never rewritten.
bool applyContentAliases(WorldMemorySnapshot& snapshot,
                         const ContentIdAliasTable& aliases,
                         std::string* error = nullptr);

// Snapshot store for persistent entities + Chronicle records. File publication
// uses the same checked atomic-generation primitive as the global manifest and
// retains one previous known-good generation for crash/corruption recovery.
class WorldMemoryStore {
public:
    explicit WorldMemoryStore(std::filesystem::path path);

    bool save(const WorldMemorySnapshot& snapshot, std::string* error = nullptr) const;
    WorldMemoryLoadResult load(std::optional<std::uint64_t> expectedSaveGeneration = std::nullopt) const;

    // Rewrites a validated snapshot atomically into canonical ordering without
    // changing logical state. One equivalent previous generation is retained as
    // the crash fallback, matching the save-compaction contract.
    bool compact(std::optional<std::uint64_t> expectedSaveGeneration = std::nullopt,
                 std::string* error = nullptr) const;

    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
};

} // namespace elysium
