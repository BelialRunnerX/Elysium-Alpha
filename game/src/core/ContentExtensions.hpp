// Intended function: imported core implementation for ContentExtensions; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

// Save/mod/network-facing content identity. The canonical string is the
// persistent identity. Runtime positions in a registry are deliberately not
// persistent and may change when optional content is added or removed.
class ContentId {
public:
    ContentId() = default;
    explicit ContentId(std::string canonical);

    static std::optional<ContentId> parse(std::string_view text, std::string* error = nullptr);
    static bool valid(std::string_view text);

    const std::string& str() const { return value_; }
    bool empty() const { return value_.empty(); }

    // Stable deterministic hash for ordering/fingerprints only. Saves still
    // encode the canonical string so a hash collision cannot alias content.
    std::uint64_t stableHash() const;

    friend bool operator==(const ContentId&, const ContentId&) = default;
    friend bool operator<(const ContentId& a, const ContentId& b) { return a.value_ < b.value_; }

private:
    std::string value_;
};

enum class ExtensionPoint : std::uint8_t {
    BlockMaterial = 0,
    RaceClass,
    RuneTrinket,
    RecipeProcess,
    Bestiary,
    RewardProvider,
    PoiKit,
    DungeonRoom,
    BiomeAddon,
    ContractTemplate,
    PersistentObject,
    Script
};

struct ContentDescriptor {
    ContentId id;
    ExtensionPoint extensionPoint{ExtensionPoint::BlockMaterial};
    std::uint32_t definitionVersion{1};
    std::uint64_t deterministicFingerprint{};

    friend bool operator==(const ContentDescriptor&, const ContentDescriptor&) = default;
};

enum class ContentResolution : std::uint8_t {
    Resolved = 0,
    UnresolvedPlaceholder = 1
};

struct ResolvedContentRef {
    ContentId requestedId;
    ContentResolution resolution{ContentResolution::UnresolvedPlaceholder};
    const ContentDescriptor* descriptor{};
};

// Registration may happen in any optional-content load order. freeze() sorts by
// canonical persistent ContentId, validates duplicates, and produces a stable
// manifest. The returned runtime ordinal is explicitly transient and must never
// be serialized.
class ContentRegistry {
public:
    bool registerContent(ContentDescriptor descriptor, std::string* error = nullptr);
    bool freeze(std::string* error = nullptr);
    bool frozen() const { return frozen_; }

    ResolvedContentRef resolve(const ContentId& id) const;
    std::optional<std::size_t> runtimeOrdinal(const ContentId& id) const;
    const std::vector<ContentDescriptor>& ordered() const { return ordered_; }
    std::uint64_t manifestFingerprint() const;

private:
    bool frozen_{};
    std::vector<ContentDescriptor> pending_;
    std::vector<ContentDescriptor> ordered_;
};

struct ContentManifestEntry {
    ContentId id;
    std::uint32_t definitionVersion{1};
    std::uint64_t deterministicFingerprint{};
};

struct ContentManifestCompatibility {
    std::vector<ContentId> missingIds;            // preserve as placeholders
    std::vector<ContentId> changedDefinitions;    // same ID, incompatible definition fingerprint/version
    bool compatibleWithoutMigration{};
};

std::vector<ContentManifestEntry> captureContentManifest(const ContentRegistry& registry);
ContentManifestCompatibility compareContentManifest(const std::vector<ContentManifestEntry>& saved,
                                                    const ContentRegistry& current);

// Persistent extension objects use stable object identity + namespaced object
// type + explicit schema. Unknown object types remain parseable opaque records;
// they are not reassigned or discarded merely because a content pack is absent.
struct ExtensionSchemaDescriptor {
    ContentId objectType;
    std::uint32_t currentVersion{1};
    std::uint32_t oldestReadableVersion{1};
};

class ExtensionSchemaRegistry {
public:
    bool registerSchema(ExtensionSchemaDescriptor descriptor, std::string* error = nullptr);
    bool freeze(std::string* error = nullptr);
    const ExtensionSchemaDescriptor* find(const ContentId& type) const;
    bool canRead(const ContentId& type, std::uint32_t version) const;

private:
    bool frozen_{};
    std::vector<ExtensionSchemaDescriptor> pending_;
    std::vector<ExtensionSchemaDescriptor> ordered_;
};

struct PersistentExtensionObject {
    std::uint64_t stableObjectId{};
    ContentId objectType;
    std::uint32_t schemaVersion{1};
    std::string payload; // opaque to the engine-level envelope
};

bool validatePersistentExtensionObject(const PersistentExtensionObject& object, std::string* error = nullptr);
std::string serializePersistentExtensionObject(const PersistentExtensionObject& object);
std::optional<PersistentExtensionObject> deserializePersistentExtensionObject(std::string_view text,
                                                                              std::string* error = nullptr);

enum class PersistentMutationKind : std::uint8_t {
    WorldDelta = 0,
    UpsertObject,
    TombstoneObject,
    InventoryTransaction,
    CombatOutcome,
    StandingChange
};

// Scripts/plugins never receive a mutable world pointer through this API. They
// can submit intent records; the authoritative owner phase validates and commits
// them through the same world/ECS/persistence seams as native systems.
struct ModMutationCommand {
    ContentId originMod;
    std::uint64_t originSequence{};
    PersistentMutationKind kind{PersistentMutationKind::WorldDelta};
    std::uint64_t stableTargetId{};
    ContentId payloadSchema;
    std::uint32_t payloadSchemaVersion{1};
    std::string payload;
};

class ModCommandBuffer {
public:
    bool submit(ModMutationCommand command, std::string* error = nullptr);
    std::vector<ModMutationCommand> drainDeterministic();
    std::size_t size() const { return pending_.size(); }

private:
    std::vector<ModMutationCommand> pending_;
};

enum class ReplicationRecordKind : std::uint8_t {
    ChunkDelta = 0,
    ObjectUpsert,
    ObjectTombstone,
    HistoricalEvent,
    AuthoritativeEntityState
};

// Future networking envelope only. It intentionally contains stable IDs,
// versioned schemas and opaque payloads; no entt::entity/GPU/runtime handle can
// cross this boundary by type.
struct ReplicationRecord {
    ReplicationRecordKind kind{ReplicationRecordKind::ChunkDelta};
    std::uint64_t streamStableId{}; // stable world/site/chunk/event stream identity
    std::uint64_t sequence{};
    std::uint64_t objectStableId{}; // zero when not object-scoped
    ContentId payloadSchema;
    std::uint32_t payloadSchemaVersion{1};
    std::string payload;
};

bool validateReplicationRecord(const ReplicationRecord& record, std::string* error = nullptr);
std::string serializeReplicationRecord(const ReplicationRecord& record);
std::optional<ReplicationRecord> deserializeReplicationRecord(std::string_view text,
                                                              std::string* error = nullptr);

} // namespace elysium
