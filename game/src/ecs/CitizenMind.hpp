// Intended function: imported ecs implementation for CitizenMind; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "core/JobSystem.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace elysium {

// Agent 18 mind/social model. These are deliberately narrow, EnTT-ready data
// components with no behavior hidden in component methods. A live citizen's
// StableId is supplied by the identity layer; it is never an entt::entity.
using CitizenStableId = std::uint64_t;
using ChronicleEventId = std::uint64_t;
using SimTimeMs = std::int64_t;

enum class NeedType : std::uint8_t {
    Sleep = 0,
    Food,
    Safety,
    SocialContact,
    Purpose,
    Philosophy,
    Creativity,
    Learning,
    Family,
    Solitude,
    Excitement,
    Count
};

enum class PersonalityFacet : std::uint8_t {
    RiskTolerance = 0,
    Sociability,
    Patience,
    Discipline,
    Anxiety,
    Anger,
    Altruism,
    Curiosity,
    Orderliness,
    Count
};

enum class ValueDimension : std::uint8_t {
    Family = 0,
    Law,
    Independence,
    Empire,
    Tradition,
    Craft,
    Knowledge,
    Nature,
    Wealth,
    MartialHonor,
    Count
};

enum class PreferenceCategory : std::uint8_t {
    Material = 0,
    Food,
    Color,
    Creature,
    ArtMotif,
    Environment,
    Activity,
    Person,
    Other
};

struct Personality {
    std::array<float, static_cast<std::size_t>(PersonalityFacet::Count)> facets{};
};

struct Values {
    std::array<float, static_cast<std::size_t>(ValueDimension::Count)> importance{};
};

struct PreferenceEntry {
    PreferenceCategory category{PreferenceCategory::Other};
    std::string key; // Stable namespaced content/person key when applicable.
    float affinity{}; // [-1, 1]

    friend bool operator==(const PreferenceEntry&, const PreferenceEntry&) = default;
};

struct Preferences {
    std::vector<PreferenceEntry> entries; // COLD component / side-store payload.
};

struct NeedDimensionState {
    float strength{0.5f};    // [0,1], how strongly this citizen cares about the need.
    float satisfaction{1.0f}; // [0,1], 1 means fully satisfied.

    friend bool operator==(const NeedDimensionState&, const NeedDimensionState&) = default;
};

struct Needs {
    std::array<NeedDimensionState, static_cast<std::size_t>(NeedType::Count)> dimensions{};
};

enum class ThoughtKind : std::uint8_t {
    NeedDiscomfort = 0,
    NeedRelief,
    Fear,
    Grief,
    Anger,
    Frustration,
    Pride,
    Comfort,
    Gratitude,
    Interest,
    Inspiration,
    Social,
    Conflict,
    General
};

struct ThoughtRecord {
    ChronicleEventId sourceEventId{};
    SimTimeMs createdAt{};
    ThoughtKind kind{ThoughtKind::General};
    float valence{};   // [-1,1]
    float intensity{}; // [0,1]
    std::optional<NeedType> need;
    CitizenStableId other{};
    std::string topic;

    friend bool operator==(const ThoughtRecord&, const ThoughtRecord&) = default;
};

struct CurrentThoughts {
    std::vector<ThoughtRecord> entries;
};

struct MemoryRecord {
    ChronicleEventId sourceEventId{};
    SimTimeMs formedAt{};
    ThoughtKind kind{ThoughtKind::General};
    float valence{};     // [-1,1]
    float strength{};    // [0,1] at formation; effective strength decays on read.
    float significance{}; // [0,1]
    bool traumatic{};
    std::optional<NeedType> need;
    CitizenStableId other{};
    std::string topic;

    friend bool operator==(const MemoryRecord&, const MemoryRecord&) = default;
};

struct Memories {
    std::vector<MemoryRecord> entries; // COLD component / side-store payload.
};

enum class StressBand : std::uint8_t {
    Stable = 0,
    Strained,
    Distressed,
    Crisis,
    Recovery
};

enum class CrisisResponse : std::uint8_t {
    Panic = 0,
    Rage,
    Shutdown,
    Flight,
    Obsession
};

struct StressState {
    float accumulated{}; // [0,100], no hidden happiness clamp.
    StressBand band{StressBand::Stable};
    StressBand previousBand{StressBand::Stable};
    SimTimeMs bandChangedAt{};
    CrisisResponse lastCrisisResponse{CrisisResponse::Panic};
};

struct Focus {
    float value{1.0f}; // DERIVED cache, [0.15,1]. Not save truth.
};

struct CopingProfile {
    // Learned/available coping affinities added to trait-derived crisis scores.
    std::array<float, 5> responseAffinity{}; // indexed by CrisisResponse, [-1,1]
};

struct CitizenMindState {
    CitizenStableId stableId{};
    Personality personality;
    Values values;
    Preferences preferences;
    Needs needs;
    StressState stress;
    CurrentThoughts thoughts;
    Memories memories;
    CopingProfile coping;
    Focus focus;
};

enum class MindEventType : std::uint8_t {
    NeedThreshold = 0,
    Danger,
    Injury,
    Loss,
    Achievement,
    Comfort,
    SocialInteraction,
    Conflict,
    CreativeAct,
    Learning,
    Philosophy,
    Discovery,
    WorkSuccess,
    WorkFailure,
    RelationshipChange,
    General
};

struct ReactionModifier {
    std::optional<PersonalityFacet> facet;
    float facetWeight{}; // [-1,1], positive means high facet intensifies reaction.
    std::optional<ValueDimension> value;
    float valueWeight{}; // [-1,1]
    std::optional<PreferenceCategory> preferenceCategory;
    std::string preferenceKey;
    float preferenceWeight{}; // [-1,1]
};

struct MindEvent {
    ChronicleEventId eventId{};
    SimTimeMs time{};
    MindEventType type{MindEventType::General};
    float valence{}; // [-1,1]
    float intensity{0.5f}; // [0,1]
    float chronicleImportance{}; // [0,1]
    std::optional<NeedType> need;
    CitizenStableId other{};
    ThoughtKind thoughtKind{ThoughtKind::General};
    std::string topic;
    ReactionModifier reaction;
};

enum class MindDomainEventType : std::uint8_t {
    NeedThresholdCrossed = 0,
    ThoughtCreated,
    MemoryFormed,
    StressBandChanged,
    CrisisIntentCreated
};

struct MindDomainEvent {
    MindDomainEventType type{MindDomainEventType::ThoughtCreated};
    CitizenStableId subject{};
    ChronicleEventId sourceEventId{};
    SimTimeMs time{};
    float value{};
    std::optional<NeedType> need;
    StressBand stressBand{StressBand::Stable};
    CrisisResponse crisisResponse{CrisisResponse::Panic};
};

struct MindUpdateResult {
    float stressDelta{};
    bool thoughtCreated{};
    bool memoryFormed{};
    bool stressBandChanged{};
    bool crisisIntentCreated{};
    CrisisResponse crisisResponse{CrisisResponse::Panic};
    std::vector<MindDomainEvent> events;
};

enum class MindWorkBlocker : std::uint8_t {
    None = 0,
    Crisis,
    Distressed,
    ExhaustedNeed,
    UnsafeNeed,
    LowFocus
};

struct WorkReadiness {
    bool canWork{true};
    MindWorkBlocker blocker{MindWorkBlocker::None};
    std::optional<NeedType> need;
    float severity{};
};

enum class InterventionKind : std::uint8_t {
    Sleep = 0,
    Food,
    Safety,
    SocialContact,
    Purpose,
    Philosophy,
    Creativity,
    Learning,
    Family,
    Solitude,
    Excitement,
    CounselingOrCoping,
    ResolveGrievance,
    MemorialOrGriefSupport,
    MedicalSafety,
    QuietRecovery
};

struct MindDiagnosticReason {
    enum class Kind : std::uint8_t { UnmetNeed = 0, Memory, Thought, StressBand } kind{Kind::UnmetNeed};
    float weight{};
    std::optional<NeedType> need;
    ChronicleEventId eventId{};
    std::string topic;
};

struct MindDiagnostic {
    CitizenStableId stableId{};
    StressBand band{StressBand::Stable};
    float stress{};
    float focus{};
    WorkReadiness work;
    std::vector<MindDiagnosticReason> reasons;
    std::vector<InterventionKind> interventions;
};

class CitizenMindSystem {
public:
    // Tuning values, intentionally centralized and inspectable.
    static constexpr float kNeedProblemThreshold = 0.35f;
    static constexpr float kNeedReliefThreshold = 0.65f;
    static constexpr float kMemoryPromotionThreshold = 0.55f;
    static constexpr std::size_t kMaxCurrentThoughts = 24;
    static constexpr std::size_t kMaxMemories = 128;

    static void normalize(CitizenMindState& state);
    static float preferenceAffinity(const Preferences& preferences,
                                    PreferenceCategory category,
                                    std::string_view key);
    static bool setPreference(Preferences& preferences, PreferenceEntry entry, std::string* error = nullptr);

    static MindUpdateResult updateNeed(CitizenMindState& state,
                                       NeedType need,
                                       float newSatisfaction,
                                       SimTimeMs now,
                                       ChronicleEventId eventId);
    static MindUpdateResult applyEvent(CitizenMindState& state, const MindEvent& event);
    static MindUpdateResult tick(CitizenMindState& state, SimTimeMs now, SimTimeMs elapsedMs);

    static float effectiveMemoryStrength(const MemoryRecord& memory, SimTimeMs now);
    static float recomputeFocus(CitizenMindState& state);
    static CrisisResponse selectCrisisResponse(const CitizenMindState& state);
    static WorkReadiness evaluateWorkReadiness(const CitizenMindState& state);
    static MindDiagnostic inspect(const CitizenMindState& state, SimTimeMs now,
                                  std::size_t maxReasons = 6);

    // Stable-ID-sorted owner snapshot + independent worker updates. No structural
    // ECS mutation occurs on workers; callers commit any emitted intents/events.
    static std::vector<MindUpdateResult> tickBatch(JobSystem& jobs,
                                                   std::vector<CitizenMindState>& states,
                                                   SimTimeMs now,
                                                   SimTimeMs elapsedMs);
};

const char* needTypeName(NeedType value);
const char* stressBandName(StressBand value);
const char* crisisResponseName(CrisisResponse value);
const char* interventionName(InterventionKind value);
const char* mindWorkBlockerName(MindWorkBlocker value);
std::string formatMindDiagnostic(const MindDiagnostic& diagnostic);

// ---------------------------------------------------------------------------
// Sparse persistent social graph.
// ---------------------------------------------------------------------------

enum class RelationshipType : std::uint32_t {
    Parent = 1u << 0u,
    Child = 1u << 1u,
    Sibling = 1u << 2u,
    Partner = 1u << 3u,
    Friend = 1u << 4u,
    Rival = 1u << 5u,
    Mentor = 1u << 6u,
    Commander = 1u << 7u,
    Coworker = 1u << 8u,
    Creditor = 1u << 9u,
    ReligiousRelation = 1u << 10u,
    Guardian = 1u << 11u,
    Ward = 1u << 12u
};

constexpr std::uint32_t relationshipMask(RelationshipType type) {
    return static_cast<std::uint32_t>(type);
}

struct RelationshipEdge {
    CitizenStableId source{};
    CitizenStableId target{};
    std::uint32_t typeTags{};
    float affinity{};    // [-1,1]
    float trust{};       // [0,1]
    float respect{};     // [0,1]
    float fear{};        // [0,1]
    float grievance{};   // [0,1]
    float familiarity{}; // [0,1]
    SimTimeMs lastInteraction{};
    std::vector<ChronicleEventId> eventRefs;

    friend bool operator==(const RelationshipEdge&, const RelationshipEdge&) = default;
};

struct RelationshipDelta {
    float affinity{};
    float trust{};
    float respect{};
    float fear{};
    float grievance{};
    float familiarity{};
    std::uint32_t addTypeTags{};
    std::uint32_t removeTypeTags{};
};

enum class SocialDomainEventType : std::uint8_t {
    RelationshipChanged = 0,
    PartnershipFormed,
    GrievanceCreated,
    MentorshipEstablished
};

struct SocialDomainEvent {
    SocialDomainEventType type{SocialDomainEventType::RelationshipChanged};
    CitizenStableId source{};
    CitizenStableId target{};
    ChronicleEventId chronicleEvent{};
    SimTimeMs time{};
};

class RelationshipStore {
public:
    static constexpr std::size_t kMaxEventRefsPerEdge = 32;

    bool upsert(RelationshipEdge edge, std::string* error = nullptr);
    bool apply(CitizenStableId source,
               CitizenStableId target,
               const RelationshipDelta& delta,
               SimTimeMs now,
               ChronicleEventId eventId = 0,
               std::string* error = nullptr);

    bool linkParentChild(CitizenStableId parent, CitizenStableId child,
                         SimTimeMs now, ChronicleEventId eventId = 0,
                         std::string* error = nullptr);
    bool linkSiblings(CitizenStableId a, CitizenStableId b,
                      SimTimeMs now, ChronicleEventId eventId = 0,
                      std::string* error = nullptr);
    bool linkGuardianWard(CitizenStableId guardian, CitizenStableId ward,
                          SimTimeMs now, ChronicleEventId eventId = 0,
                          std::string* error = nullptr);
    bool linkPartner(CitizenStableId a, CitizenStableId b,
                     SimTimeMs now, ChronicleEventId eventId = 0,
                     std::string* error = nullptr);
    bool linkFriend(CitizenStableId a, CitizenStableId b,
                    SimTimeMs now, ChronicleEventId eventId = 0,
                    std::string* error = nullptr);
    bool linkCoworkers(CitizenStableId a, CitizenStableId b,
                       SimTimeMs now, ChronicleEventId eventId = 0,
                       std::string* error = nullptr);
    bool linkRivals(CitizenStableId a, CitizenStableId b,
                    SimTimeMs now, ChronicleEventId eventId = 0,
                    std::string* error = nullptr);
    bool linkMentor(CitizenStableId mentor, CitizenStableId student,
                    SimTimeMs now, ChronicleEventId eventId = 0,
                    std::string* error = nullptr);

    // Establish a new generation without assuming a human two-parent family model.
    // Species/life-cycle code supplies whichever durable parent, sibling and guardian
    // identities are appropriate. Duplicate IDs are accepted and canonicalized.
    bool linkFamilyGeneration(CitizenStableId child,
                              std::span<const CitizenStableId> parents,
                              std::span<const CitizenStableId> siblings,
                              std::span<const CitizenStableId> guardians,
                              SimTimeMs now, ChronicleEventId eventId = 0,
                              std::string* error = nullptr);

    const RelationshipEdge* find(CitizenStableId source, CitizenStableId target) const;
    std::optional<RelationshipEdge> effective(CitizenStableId source,
                                               CitizenStableId target,
                                               SimTimeMs now) const;
    std::vector<RelationshipEdge> edgesFor(CitizenStableId stableId, SimTimeMs now) const;
    std::vector<RelationshipEdge> allEdges() const;
    std::size_t size() const;
    void clear();

    const std::vector<SocialDomainEvent>& emittedEvents() const { return emittedEvents_; }
    void clearEmittedEvents() { emittedEvents_.clear(); }

private:
    std::vector<RelationshipEdge> edges_; // always sorted by (source,target)
    std::vector<SocialDomainEvent> emittedEvents_;

    std::vector<RelationshipEdge>::iterator lowerBound(CitizenStableId source, CitizenStableId target);
    std::vector<RelationshipEdge>::const_iterator lowerBound(CitizenStableId source, CitizenStableId target) const;
    void emitLinkEvents(const RelationshipEdge& before, const RelationshipEdge& after,
                        ChronicleEventId eventId, SimTimeMs now);
};

bool validateRelationshipEdge(const RelationshipEdge& edge, std::string* error = nullptr);

// ---------------------------------------------------------------------------
// Explicit versioned persistence. Focus is derived and is intentionally not
// serialized. Codecs write fields explicitly; no raw struct/EnTT identity is
// ever written.
// ---------------------------------------------------------------------------

struct CitizenMindRecord {
    static constexpr std::uint32_t SchemaVersion = 1;
    std::uint32_t schemaVersion{SchemaVersion};
    CitizenMindState state;
};

bool encodeCitizenMindRecord(const CitizenMindRecord& record,
                             std::vector<std::uint8_t>& out,
                             std::string* error = nullptr,
                             bool includeCurrentThoughts = true);
bool decodeCitizenMindRecord(std::span<const std::uint8_t> bytes,
                             CitizenMindRecord& out,
                             std::string* error = nullptr);

bool encodeRelationshipStore(const RelationshipStore& store,
                             std::vector<std::uint8_t>& out,
                             std::string* error = nullptr);
bool decodeRelationshipStore(std::span<const std::uint8_t> bytes,
                             RelationshipStore& out,
                             std::string* error = nullptr);

// Stable canonical bytes used by worker-count determinism tests and snapshots.
std::vector<std::uint8_t> encodeMindBatchCanonical(const std::vector<CitizenMindState>& states,
                                                   std::string* error = nullptr);

} // namespace elysium
