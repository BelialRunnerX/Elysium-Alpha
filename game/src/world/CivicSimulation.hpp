// Intended function: imported world implementation for CivicSimulation; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace elysium {

using CivicStableId = std::uint64_t;

// Runtime policy/status values are persisted by numeric value. Append new values;
// do not renumber existing entries once saves ship.
enum class InstitutionType : std::uint8_t {
    Cantina = 0,
    Shrine = 1,
    Guildhall = 2,
    Archive = 3,
    Hospital = 4,
    Memorial = 5,
    Forum = 6,
    Academy = 7
};

enum class CivicNeed : std::uint8_t {
    Social = 0,
    Reflection = 1,
    Learning = 2,
    Health = 3,
    Grief = 4,
    Civic = 5,
    Training = 6,
    Count = 7
};

enum class LegalStatus : std::uint8_t {
    Visitor = 0,
    GuestWorker = 1,
    Resident = 2,
    Citizen = 3,
    ProtectedRefugee = 4,
    Restricted = 5,
    Exiled = 6
};

enum class ArrivalType : std::uint8_t {
    Founder = 0,
    Migrant = 1,
    Refugee = 2,
    Contractor = 3,
    PerformerScholar = 4,
    MercenaryHunter = 5,
    TraderCrew = 6,
    ImperialOfficial = 7,
    Infiltrator = 8,
    PilgrimGuild = 9
};

enum class RelationshipType : std::uint8_t {
    Partner = 0,
    Parent = 1,
    Child = 2,
    Sibling = 3,
    Friend = 4,
    Rival = 5,
    Mentor = 6,
    Coworker = 7,
    Household = 8
};

enum class CivicHistoryEventType : std::uint8_t {
    InstitutionFounded = 0,
    InstitutionDissolved = 1,
    InstitutionSchism = 2,
    PopulationArrived = 3,
    PopulationDeparted = 4,
    LegalStatusChanged = 5,
    InstitutionActivity = 6,
    CulturalWorkCreated = 7,
    CulturalWorkTransmission = 8
};

enum class CulturalTransmissionType : std::uint8_t {
    Copied = 0,
    Performed = 1,
    Traded = 2,
    Censored = 3,
    Archived = 4
};

enum class CivicSimulationDetail : std::uint8_t {
    ActiveFortress = 0,
    RemoteSite = 1,
    Strategic = 2
};

struct WeightedCultureTag {
    std::string tag;
    float weight{};
};

struct CultureRecord {
    CivicStableId stableId{};
    std::string name;
    std::vector<WeightedCultureTag> values;
    std::vector<std::string> legalNorms;
    std::vector<std::string> architectureTags;
    std::vector<std::string> cuisineTags;
    std::vector<std::string> performanceTraditions;
    std::vector<std::string> artMotifs;
    std::vector<std::string> rituals;
    std::vector<std::string> languageTags;
};

struct SkillRank {
    std::string skill;
    float rank{};
};

struct RelationshipEdge {
    CivicStableId targetStableId{};
    RelationshipType type{RelationshipType::Friend};
    float affinity{};
    float trust{};
    float respect{};
    float fear{};
    float grievance{};
    float familiarity{};
};

struct CitizenRecord {
    CivicStableId stableId{};
    CivicStableId settlementId{};
    CivicStableId originSiteId{};
    CivicStableId householdId{};
    CivicStableId cultureId{};
    std::string name;
    LegalStatus legalStatus{LegalStatus::Visitor};
    bool present{true};
    bool historySignificant{};
    double arrivalHour{};
    double residencyHours{};
    double visitorPermitUntilHour{};
    std::array<float, static_cast<std::size_t>(CivicNeed::Count)> needs{};
    std::vector<SkillRank> skills;
    std::vector<RelationshipEdge> relationships;
    std::vector<CivicStableId> institutionMemberships;
};

struct CivicSchedule {
    // Local settlement clock, [0,24). closeHour < openHour means overnight.
    float openHour{0.0f};
    float closeHour{24.0f};
    float activityPeriodHours{6.0f};
    double nextActivityHour{};
};

struct InstitutionService {
    CivicNeed need{CivicNeed::Social};
    float satisfaction{0.25f};
    std::string requiredSupply;
    int supplyCost{};
};

struct EducationProgram {
    std::string skill;
    float minimumMentorRank{2.0f};
    float learnerGain{0.10f};
};

struct InstitutionRecord {
    CivicStableId stableId{};
    CivicStableId settlementId{};
    CivicStableId cultureId{};
    CivicStableId ownerId{};
    CivicStableId leaderId{};
    InstitutionType type{InstitutionType::Cantina};
    std::string name;
    bool active{true};
    float quality{0.5f};
    int staffAvailable{1};
    CivicSchedule schedule{};
    std::vector<CivicStableId> roomRefs;
    std::vector<CivicStableId> memberIds;
    std::map<std::string, int> supplies;
    std::vector<InstitutionService> services;
    std::optional<EducationProgram> education;
};

struct CulturalWorkRecord {
    CivicStableId stableId{};
    CivicStableId authorStableId{};
    CivicStableId institutionStableId{};
    CivicStableId cultureId{};
    std::string title;
    std::string form;
    std::string subject;
    std::string style;
    float quality{};
    double createdHour{};
    bool archived{};
};

struct CivicHistoryEvent {
    CivicStableId stableId{};
    CivicHistoryEventType type{CivicHistoryEventType::PopulationArrived};
    CivicStableId settlementId{};
    CivicStableId subjectStableId{};
    CivicStableId relatedStableId{};
    double hour{};
    std::string detail;
};

struct CulturalTransmissionRecord {
    CivicStableId stableId{}; // Reuses the corresponding Chronicle/history event StableId.
    CivicStableId workStableId{};
    CulturalTransmissionType type{CulturalTransmissionType::Performed};
    CivicStableId actorStableId{};
    CivicStableId institutionStableId{};
    CivicStableId settlementId{};
    double hour{};
    std::string detail;
};

struct MigrationPolicy {
    bool allowVisitors{true};
    bool allowMigrants{true};
    bool allowRefugees{true};
    bool allowGuestWorkers{true};
    bool allowCitizenship{true};
    double visitorPermitHours{72.0};
    double minimumResidentHoursForCitizenship{24.0 * 30.0};
};

struct SettlementMigrationInputs {
    float safety{0.5f};
    float workDemand{0.5f};
    float reputation{0.5f};
    float culturalAffinity{0.5f};
    float policyOpenness{0.5f};
    float housing{0.5f};
    float foodSecurity{0.5f};
    float institutionCoverage{0.5f};
    float historyPull{0.5f};
    float factionCompatibility{0.5f};
    float activeThreat{};
};

struct MigrationMemberSpec {
    std::string name;
    std::vector<SkillRank> skills;
    std::array<float, static_cast<std::size_t>(CivicNeed::Count)> initialNeeds{};
    bool historySignificant{};
};

struct MigrationRelationshipSpec {
    std::size_t sourceMember{};
    std::size_t targetMember{};
    RelationshipType type{RelationshipType::Household};
    float affinity{0.5f};
    float trust{0.5f};
    float respect{0.5f};
    float fear{};
    float grievance{};
};

struct MigrationGroup {
    CivicStableId groupStableId{};
    CivicStableId originSiteId{};
    CivicStableId cultureId{};
    ArrivalType type{ArrivalType::Migrant};
    std::string motive;
    float urgency{};
    std::vector<MigrationMemberSpec> members;
    std::vector<MigrationRelationshipSpec> relationships;
};

struct DiagnosticFactor {
    std::string label;
    float value{};
    float weight{};
    float contribution{};
};

struct MigrationDecision {
    bool policyAllowed{};
    bool accepted{};
    float score{};
    float threshold{};
    std::string summary;
    std::vector<DiagnosticFactor> factors;
};

struct DepartureInputs {
    float safety{0.5f};
    float workSatisfaction{0.5f};
    float housing{0.5f};
    float foodSecurity{0.5f};
    float institutionAccess{0.5f};
    float relationshipRoots{0.5f};
    float culturalFit{0.5f};
    float factionPressure{};
    float activeThreat{};
    float personalOpportunity{};
};

struct DepartureDecision {
    bool citizenFound{};
    bool wouldLeave{};
    float pressure{};
    float threshold{0.50f};
    std::string summary;
    std::vector<DiagnosticFactor> factors;
};

struct ArrivalResult {
    bool admitted{};
    std::string reason;
    std::vector<CivicStableId> citizenStableIds;
};

struct InstitutionTickTelemetry {
    int institutionsConsidered{};
    int activitiesRun{};
    int citizensServed{};
    int visitorsServed{};
    int needsSatisfied{};
    int educationSessions{};
    int blockedNoStaff{};
    int blockedSupplies{};
};

struct CivicPopulationDiagnostics {
    int present{};
    int visitors{};
    int guestWorkers{};
    int residents{};
    int citizens{};
    int refugees{};
    int restricted{};
    int exiled{};
};

struct CivicServiceDemandEntry {
    CivicNeed need{CivicNeed::Social};
    float totalUnmet{};
    int presentPeople{};
    int activeInstitutions{};
    int blockedInstitutions{};
};

struct InstitutionDiagnostics {
    bool found{};
    bool active{};
    bool openNow{};
    int membersPresent{};
    int visitorsEligible{};
    int staffAvailable{};
    std::vector<std::string> blockers;
    std::vector<std::string> offeredServices;
};

// CivicSimulation is an orchestration facade over component-shaped records.
// Stable IDs are the public/persistence boundary; a production EnTT adapter can
// map these records into ordinary components without serializing entt::entity.
class CivicSimulation {
public:
    explicit CivicSimulation(std::uint64_t deterministicSeed = 1);

    CivicStableId addCulture(CultureRecord culture, CivicStableId preferredStableId = 0);
    CivicStableId createInstitution(InstitutionType type,
                                    CivicStableId settlementId,
                                    CivicStableId cultureId,
                                    std::string name,
                                    CivicSchedule schedule = {},
                                    CivicStableId preferredStableId = 0);
    bool dissolveInstitution(CivicStableId institutionId, double hour, std::string reason);
    bool recordInstitutionSchism(CivicStableId institutionId, CivicStableId childInstitutionId,
                                 double hour, std::string reason);
    bool addInstitutionMember(CivicStableId institutionId, CivicStableId citizenId);
    bool removeInstitutionMember(CivicStableId institutionId, CivicStableId citizenId);
    bool setInstitutionSupply(CivicStableId institutionId, std::string supplyTag, int count);
    bool setInstitutionStaff(CivicStableId institutionId, int count);
    bool setEducationProgram(CivicStableId institutionId, EducationProgram program);

    MigrationDecision evaluateMigration(const MigrationGroup& group,
                                        const SettlementMigrationInputs& inputs,
                                        const MigrationPolicy& policy) const;
    ArrivalResult admitGroup(CivicStableId settlementId,
                             const MigrationGroup& group,
                             const SettlementMigrationInputs& inputs,
                             const MigrationPolicy& policy,
                             double hour);
    bool transitionLegalStatus(CivicStableId citizenId, LegalStatus next,
                               const MigrationPolicy& policy, double hour,
                               std::string* reason = nullptr);
    bool departCitizen(CivicStableId citizenId, double hour, std::string reason);

    bool setCitizenNeed(CivicStableId citizenId, CivicNeed need, float value);
    bool addRelationship(CivicStableId sourceId, RelationshipEdge edge);
    CivicStableId createCulturalWork(CulturalWorkRecord work, CivicStableId preferredStableId = 0);
    CivicStableId recordCulturalTransmission(CivicStableId workStableId,
                                             CulturalTransmissionType type,
                                             CivicStableId actorStableId,
                                             CivicStableId institutionStableId,
                                             CivicStableId settlementId,
                                             double hour,
                                             std::string detail = {});
    std::vector<CulturalTransmissionRecord> transmissionsForWork(CivicStableId workStableId) const;
    DepartureDecision evaluateDeparture(CivicStableId citizenId, const DepartureInputs& inputs) const;

    InstitutionTickTelemetry updateInstitutions(double startHour, double elapsedHours);
    void setSimulationDetail(CivicSimulationDetail detail) { detail_ = detail; }
    CivicSimulationDetail simulationDetail() const { return detail_; }

    const CitizenRecord* citizen(CivicStableId id) const;
    CitizenRecord* citizen(CivicStableId id);
    const InstitutionRecord* institution(CivicStableId id) const;
    InstitutionRecord* institution(CivicStableId id);
    const CultureRecord* culture(CivicStableId id) const;
    const CulturalWorkRecord* culturalWork(CivicStableId id) const;

    std::vector<CivicStableId> citizenIds() const;
    std::vector<CivicStableId> institutionIds() const;
    const std::vector<CivicHistoryEvent>& history() const { return history_; }
    const std::vector<CulturalTransmissionRecord>& culturalTransmissions() const { return transmissions_; }
    CivicPopulationDiagnostics populationDiagnostics(CivicStableId settlementId) const;
    InstitutionDiagnostics institutionDiagnostics(CivicStableId institutionId, double hour) const;
    std::vector<CivicServiceDemandEntry> serviceDemandDiagnostics(CivicStableId settlementId, double hour) const;

    std::string serialize() const;
    bool deserialize(const std::string& text, std::string* error = nullptr);

private:
    CivicStableId allocateStableId(CivicStableId preferred = 0);
    CivicStableId deriveMemberStableId(CivicStableId settlementId, CivicStableId groupId,
                                       std::size_t memberIndex) const;
    CivicStableId ensureUniqueDerived(CivicStableId candidate) const;
    CivicStableId appendHistory(CivicHistoryEventType type, CivicStableId settlementId,
                                CivicStableId subjectId, CivicStableId relatedId,
                                double hour, std::string detail);
    LegalStatus initialStatusFor(ArrivalType type) const;
    bool policyAllows(ArrivalType type, const MigrationPolicy& policy) const;
    bool scheduleOpen(const CivicSchedule& schedule, double hour) const;
    void applyDefaultInstitutionServices(InstitutionRecord& record) const;
    float* skillRank(CitizenRecord& citizen, const std::string& skill);
    const float* skillRank(const CitizenRecord& citizen, const std::string& skill) const;

    std::uint64_t seed_{1};
    CivicStableId nextStableId_{1};
    CivicSimulationDetail detail_{CivicSimulationDetail::ActiveFortress};
    std::map<CivicStableId, CultureRecord> cultures_;
    std::map<CivicStableId, CitizenRecord> citizens_;
    std::map<CivicStableId, InstitutionRecord> institutions_;
    std::map<CivicStableId, CulturalWorkRecord> works_;
    std::vector<CulturalTransmissionRecord> transmissions_;
    std::vector<CivicHistoryEvent> history_;
};

const char* institutionTypeName(InstitutionType type);
const char* legalStatusName(LegalStatus status);
const char* civicNeedName(CivicNeed need);
const char* arrivalTypeName(ArrivalType type);
const char* culturalTransmissionTypeName(CulturalTransmissionType type);

} // namespace elysium
