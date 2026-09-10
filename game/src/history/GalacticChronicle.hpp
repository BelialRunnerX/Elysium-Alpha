// Intended function: imported history implementation for GalacticChronicle; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "core/JobSystem.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

using HistoryStableId = std::uint64_t;
using SiteId = std::uint64_t;
using OrganizationId = std::uint64_t;
using ArtifactId = std::uint64_t;
using HistoricalEventId = std::uint64_t;

inline constexpr std::uint32_t kHistoryGeneratorVersion = 1;
inline constexpr std::uint64_t kHistoryGeneratorFingerprint = 0x454C594849535431ULL; // "ELYHIST1"

enum class OrganizationKind : std::uint8_t {
    CivilizationAdministration = 0,
    LocalGovernment = 1,
    Guild = 2,
    TradeNetwork = 3,
    Faith = 4,
    MilitaryFormation = 5,
    ResearchCircle = 6,
    SmugglerNetwork = 7
};

enum class SiteStatus : std::uint8_t {
    Active = 0,
    Claimed = 1,
    Retired = 2,
    Abandoned = 3,
    Destroyed = 4,
    Contaminated = 5,
    Reclaimed = 6
};

enum class HistoryEventType : std::uint16_t {
    Birth = 0,
    Naming,
    ComingOfAge,
    Death,
    Disappearance,
    Transformation,
    RelationshipFormed,
    RelationshipDissolved,
    Mentorship,
    Rivalry,
    Oath,
    Betrayal,
    SiteFounded,
    SiteExpanded,
    SiteRenamed,
    SiteClaimed,
    SiteAbandoned,
    SiteDestroyed,
    SiteContaminated,
    SiteReclaimed,
    OrganizationFounded,
    OrganizationDissolved,
    OfficeAssumed,
    OfficeRemoved,
    Schism,
    WarDeclared,
    Battle,
    Raid,
    Siege,
    Occupation,
    Liberation,
    Armistice,
    Treaty,
    Embargo,
    Migration,
    ArtifactCreated,
    ArtifactStolen,
    ArtifactGifted,
    ArtifactInherited,
    ArtifactLost,
    ArtifactRecovered,
    ArtifactDestroyed,
    Discovery,
    Filing,
    ScientificBreakthrough,
    ForbiddenProtocolLearned,
    ExpeditionResult,
    Crime,
    Accusation,
    Investigation,
    Conviction,
    Exile,
    Pardon,
    MegathreatAppeared,
    MegathreatRampage,
    MegathreatDefeated,
    MegathreatImprisoned,
    PlanetaryDisaster,
    ReactorAccident,
    SyndromeOutbreak,
    EcologicalCollapse,
    ImperialInspection,
    ImperialWarrant,
    RegisterAction,
    CourtRuling,
    Amnesty,
    Annexation,
    FirstLanding,
    FirstClaim,
    FortressRetired,
    FortressReclaimed,
    DirectOperativeIntervention,
    PlayerMilestone
};


enum class DiplomaticStance : std::uint8_t {
    Unknown = 0,
    Peace = 1,
    Treaty = 2,
    Embargo = 3,
    War = 4
};

struct StrategicRelationSummary {
    OrganizationId left{};
    OrganizationId right{};
    DiplomaticStance stance{DiplomaticStance::Unknown};
    HistoricalEventId lastEventId{};
    std::uint64_t lastChangedTimestamp{};

    friend bool operator==(const StrategicRelationSummary&, const StrategicRelationSummary&) = default;
};

struct SiteOwnershipChange {
    HistoricalEventId eventId{};
    std::uint64_t timestamp{};
    OrganizationId previousOwner{};
    OrganizationId newOwner{};
    HistoryEventType type{HistoryEventType::SiteClaimed};

    friend bool operator==(const SiteOwnershipChange&, const SiteOwnershipChange&) = default;
};

struct ArtifactOwnershipChange {
    HistoricalEventId eventId{};
    std::uint64_t timestamp{};
    HistoryStableId previousOwner{};
    HistoryStableId newOwner{};
    HistoryEventType type{HistoryEventType::ArtifactGifted};

    friend bool operator==(const ArtifactOwnershipChange&, const ArtifactOwnershipChange&) = default;
};

const char* historyEventTypeName(HistoryEventType type);
const char* organizationKindName(OrganizationKind kind);

struct HistoryLocation {
    std::uint32_t systemIndex{0xFFFFFFFFu};
    std::int32_t planetIndex{-1};
    SiteId siteId{};

    friend bool operator==(const HistoryLocation&, const HistoryLocation&) = default;
};

struct HistoryParticipant {
    HistoryStableId stableId{};
    std::string role;

    friend bool operator==(const HistoryParticipant&, const HistoryParticipant&) = default;
};

struct HistoryPayloadField {
    std::string key;
    std::string value;

    friend bool operator==(const HistoryPayloadField&, const HistoryPayloadField&) = default;
};

struct HistoricalEventDraft {
    std::uint64_t timestamp{};
    HistoryEventType type{HistoryEventType::PlayerMilestone};
    HistoryLocation location{};
    std::vector<HistoryParticipant> participants;
    std::vector<ArtifactId> artifacts;
    std::vector<OrganizationId> organizations;
    std::vector<HistoricalEventId> causeRefs;
    std::vector<HistoryPayloadField> payload;
    std::uint32_t knowledgeVisibility{0xFFFFFFFFu};
    std::uint8_t importance{1};
};

struct HistoricalEventRecord : HistoricalEventDraft {
    HistoricalEventId eventId{};

    friend bool operator==(const HistoricalEventRecord&, const HistoricalEventRecord&) = default;
};

struct CivilizationRecord {
    OrganizationId id{};
    std::string name;
    std::string culture;
    std::string government;
    std::uint32_t originSystem{};
    std::uint16_t technologyLevel{};
    std::uint16_t economicStrength{};
    std::vector<SiteId> territorySites;
    std::vector<OrganizationId> organizations;

    friend bool operator==(const CivilizationRecord&, const CivilizationRecord&) = default;
};

struct SiteRecord {
    SiteId id{};
    std::string name;
    std::uint32_t systemIndex{};
    std::int32_t planetIndex{-1};
    OrganizationId ownerOrganizationId{};
    SiteStatus status{SiteStatus::Active};
    std::uint32_t populationEstimate{};

    friend bool operator==(const SiteRecord&, const SiteRecord&) = default;
};

struct OrganizationRecord {
    OrganizationId id{};
    OrganizationKind kind{OrganizationKind::Guild};
    std::string name;
    OrganizationId civilizationId{};
    SiteId homeSiteId{};
    std::string doctrine;
    std::uint16_t strategicStrength{};
    std::uint16_t readiness{};

    friend bool operator==(const OrganizationRecord&, const OrganizationRecord&) = default;
};

struct HistoricalFigureRecord {
    HistoryStableId stableId{};
    std::string name;
    SiteId originSiteId{};
    SiteId currentSiteId{};
    OrganizationId organizationId{};
    std::uint64_t birthTimestamp{};
    std::uint32_t significanceScore{};
    bool alive{true};

    friend bool operator==(const HistoricalFigureRecord&, const HistoricalFigureRecord&) = default;
};

struct ArtifactRecord {
    ArtifactId id{};
    std::string name;
    HistoryStableId makerStableId{};
    HistoryStableId currentOwnerStableId{};
    SiteId currentSiteId{};
    HistoricalEventId creationEventId{};

    friend bool operator==(const ArtifactRecord&, const ArtifactRecord&) = default;
};

struct ChronicleQuery {
    std::optional<std::uint64_t> fromTimestamp;
    std::optional<std::uint64_t> toTimestamp;
    std::optional<HistoryEventType> type;
    std::optional<std::uint32_t> systemIndex;
    std::optional<SiteId> siteId;
    std::optional<HistoryStableId> participantStableId;
    std::optional<ArtifactId> artifactId;
    std::optional<OrganizationId> organizationId;
};

enum class ChronicleSearchKind : std::uint8_t {
    Civilization = 0,
    Site = 1,
    Organization = 2,
    HistoricalFigure = 3,
    Artifact = 4
};

struct ChronicleSearchHit {
    ChronicleSearchKind kind{ChronicleSearchKind::Site};
    std::uint64_t id{};
    std::string name;
};

struct ChronicleComparison {
    std::vector<HistoricalEventId> leftOnly;
    std::vector<HistoricalEventId> rightOnly;
    std::vector<HistoricalEventId> shared;
};

struct HistoricalSignificanceInput {
    bool named{};
    bool holdsOffice{};
    bool ownsArtifact{};
    bool majorCombat{};
    bool relatedToHistoricalFigure{};
    bool unusualTransformation{};
    bool playerInteraction{};
    bool authorship{};
    bool discovery{};
    bool leadership{};
    bool majorEventParticipant{};
};

class HistoricalSignificancePolicy {
public:
    explicit HistoricalSignificancePolicy(std::uint32_t promotionThreshold = 10)
        : promotionThreshold_(promotionThreshold) {}

    std::uint32_t score(const HistoricalSignificanceInput& input) const;
    bool shouldPromote(const HistoricalSignificanceInput& input) const;
    std::uint32_t threshold() const { return promotionThreshold_; }

private:
    std::uint32_t promotionThreshold_{};
};

// Authoritative append-oriented historical store. Maps below are durable entity
// records; query indexes are derived exclusively from events and can be rebuilt
// at any time. No EnTT entity values or dense planetary indices are stored.
class GalacticChronicle {
public:
    explicit GalacticChronicle(std::uint64_t historySeed = 0,
                               std::uint32_t generatorVersion = kHistoryGeneratorVersion,
                               std::uint64_t generatorFingerprint = kHistoryGeneratorFingerprint);

    std::uint64_t historySeed() const { return historySeed_; }
    std::uint32_t generatorVersion() const { return generatorVersion_; }
    std::uint64_t generatorFingerprint() const { return generatorFingerprint_; }

    bool registerCivilization(CivilizationRecord record, std::string* error = nullptr);
    bool registerSite(SiteRecord record, std::string* error = nullptr);
    bool registerOrganization(OrganizationRecord record, std::string* error = nullptr);
    bool registerFigure(HistoricalFigureRecord record, std::string* error = nullptr);
    bool registerArtifact(ArtifactRecord record, std::string* error = nullptr);

    const CivilizationRecord* civilization(OrganizationId id) const;
    const SiteRecord* site(SiteId id) const;
    const OrganizationRecord* organization(OrganizationId id) const;
    const HistoricalFigureRecord* figure(HistoryStableId id) const;
    const ArtifactRecord* artifact(ArtifactId id) const;

    HistoricalEventId append(HistoricalEventDraft draft, std::string* error = nullptr);
    HistoricalEventId appendPlayerMilestone(std::uint64_t timestamp,
                                            HistoryEventType type,
                                            HistoryStableId playerStableId,
                                            SiteId siteId,
                                            std::vector<HistoryPayloadField> payload = {},
                                            std::string* error = nullptr);
    HistoricalEventId appendFortressRetired(std::uint64_t timestamp,
                                             SiteId siteId,
                                             HistoryStableId actorStableId,
                                             std::string* error = nullptr);
    HistoricalEventId appendFortressReclaimed(std::uint64_t timestamp,
                                               SiteId siteId,
                                               HistoryStableId actorStableId,
                                               std::string* error = nullptr);

    HistoricalEventId appendSiteOwnershipChange(std::uint64_t timestamp,
                                                HistoryEventType type,
                                                SiteId siteId,
                                                OrganizationId newOwnerOrganizationId,
                                                std::vector<HistoryParticipant> participants = {},
                                                std::vector<HistoricalEventId> causeRefs = {},
                                                std::string* error = nullptr);
    HistoricalEventId appendOrganizationSchism(std::uint64_t timestamp,
                                                OrganizationId parentOrganizationId,
                                                OrganizationRecord child,
                                                SiteId siteId = 0,
                                                std::string* error = nullptr);

    const HistoricalEventRecord* event(HistoricalEventId id) const;
    const std::vector<HistoricalEventRecord>& events() const { return events_; }

    std::vector<HistoricalEventRecord> query(const ChronicleQuery& query) const;
    std::vector<HistoricalEventRecord> eventsForFigure(HistoryStableId id) const;
    std::vector<HistoricalEventRecord> eventsForSite(SiteId id) const;
    std::vector<HistoricalEventRecord> eventsForArtifact(ArtifactId id) const;
    std::vector<HistoricalEventRecord> eventsForOrganization(OrganizationId id) const;
    std::vector<SiteOwnershipChange> siteOwnershipTimeline(SiteId id) const;
    std::vector<ArtifactOwnershipChange> artifactOwnershipTimeline(ArtifactId id) const;
    StrategicRelationSummary relationBetween(OrganizationId left, OrganizationId right) const;
    std::vector<StrategicRelationSummary> relationsForOrganization(OrganizationId id) const;
    std::vector<OrganizationId> organizationLineage(OrganizationId id) const;
    std::vector<HistoricalEventRecord> traceCauses(HistoricalEventId eventId,
                                                    std::size_t maxDepth = 64) const;
    ChronicleComparison compareEntities(HistoryStableId left,
                                        HistoryStableId right) const;
    std::vector<ChronicleSearchHit> search(std::string_view text) const;

    void rebuildIndexes();
    std::uint64_t deterministicDigest() const;

    std::string serialize() const;
    bool restore(std::string_view text, std::string* error = nullptr);

private:
    std::uint64_t historySeed_{};
    std::uint32_t generatorVersion_{};
    std::uint64_t generatorFingerprint_{};
    std::uint64_t nextEventSerial_{1};

    std::map<OrganizationId, CivilizationRecord> civilizations_;
    std::map<SiteId, SiteRecord> sites_;
    std::map<OrganizationId, OrganizationRecord> organizations_;
    std::map<HistoryStableId, HistoricalFigureRecord> figures_;
    std::map<ArtifactId, ArtifactRecord> artifacts_;
    std::vector<HistoricalEventRecord> events_;

    std::map<HistoricalEventId, std::size_t> eventById_;
    std::map<HistoryStableId, std::vector<std::size_t>> participantIndex_;
    std::map<SiteId, std::vector<std::size_t>> siteIndex_;
    std::map<ArtifactId, std::vector<std::size_t>> artifactIndex_;
    std::map<OrganizationId, std::vector<std::size_t>> organizationIndex_;
    std::map<HistoryEventType, std::vector<std::size_t>> typeIndex_;

    HistoricalEventId allocateEventId(const HistoricalEventDraft& draft);
    bool validateDraft(HistoricalEventDraft& draft, std::string* error) const;
    void indexEvent(std::size_t index);
    void applyEventToDurableSummaries(const HistoricalEventRecord& event);
};

struct HistoryOpportunity {
    std::uint64_t physicalSiteKey{};
    std::uint32_t systemIndex{};
    std::int32_t planetIndex{-1};
    std::uint16_t habitability{};
    std::uint16_t resourceRichness{};
};

struct WorldHistoryGenerationConfig {
    std::uint64_t galaxySeed{};
    std::uint32_t historyVersion{kHistoryGeneratorVersion};
    std::uint64_t historyFingerprint{kHistoryGeneratorFingerprint};
    std::uint32_t civilizationCount{4};
};

// Deterministic strategic history generator. Physical galaxy generation is an
// upstream input: this class consumes stable opportunity descriptors but never
// rewrites terrain, planets or generator versions.
class WorldHistoryGenerator {
public:
    explicit WorldHistoryGenerator(WorldHistoryGenerationConfig config);

    GalacticChronicle generateFoundation(std::vector<HistoryOpportunity> opportunities) const;
    void simulateEpoch(JobSystem& jobs,
                       GalacticChronicle& chronicle,
                       std::uint32_t epochIndex,
                       std::uint64_t epochStartTimestamp) const;

private:
    WorldHistoryGenerationConfig config_{};
};

} // namespace elysium
