// Intended function: imported world implementation for JusticeSecurity; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

// Agent 27 portable justice/security kernel.
//
// These POD-style records are the persistent component payloads expected to be
// attached to EnTT entities by the authoritative ECS adapter. The standalone
// Part Two tree keeps the simulation dependency-free/headless-buildable: all
// cross-entity references use durable IDs, never entt::entity or pointers.
using SecurityStableId = std::uint64_t;
using SecuritySiteId = std::uint64_t;
using SecurityTime = std::uint64_t;

enum class CrimeType : std::uint8_t {
    Theft = 0,
    Assault = 1,
    Sabotage = 2,
    Murder = 3,
    Contraband = 4,
    Espionage = 5,
    MandateViolation = 6,
    Fraud = 7,
    ArtifactCrime = 8
};

enum class EvidenceKind : std::uint8_t {
    WitnessStatement = 0,
    SensorLog = 1,
    ItemProvenance = 2,
    AccessRecord = 3,
    WoundRecord = 4,
    Contraband = 5,
    MedicalScan = 6,
    BehavioralObservation = 7,
    MaintenanceAudit = 8
};

enum class EvidenceIntegrity : std::uint8_t {
    Intact = 0,
    Contaminated = 1,
    Lost = 2
};

enum class CaseStatus : std::uint8_t {
    Open = 0,
    Interviewing = 1,
    EvidenceSufficient = 2,
    Adjudication = 3,
    Closed = 4,
    Unresolved = 5
};

enum class VerdictKind : std::uint8_t {
    None = 0,
    Acquitted = 1,
    Convicted = 2,
    Dismissed = 3,
    Pardoned = 4
};

enum class JusticeAction : std::uint8_t {
    None = 0,
    Warning = 1,
    Restitution = 2,
    Restriction = 3,
    Detention = 4,
    Exile = 5,
    FactionHandoff = 6,
    Pardon = 7
};

enum class HistoryPolicy : std::uint8_t {
    Ordinary = 0,
    Major = 1
};

enum class LegalStatus : std::uint8_t {
    Visitor = 0,
    Resident = 1,
    Citizen = 2,
    Restricted = 3,
    Exiled = 4
};

enum class AccessTargetKind : std::uint8_t {
    Door = 0,
    Zone = 1,
    Room = 2,
    Machine = 3,
    Archive = 4,
    Other = 5
};

enum class InfiltratorKind : std::uint8_t {
    ImperialInformant = 0,
    RaiderScout = 1,
    EngineeredAgent = 2,
    RiftMimic = 3,
    CompromisedMachine = 4
};

enum class InfiltrationDetectionState : std::uint8_t {
    Hidden = 0,
    Suspected = 1,
    Revealed = 2
};

enum class SecurityHistoryEventType : std::uint8_t {
    Accusation = 0,
    Verdict = 1,
    Detention = 2,
    Exile = 3,
    Pardon = 4,
    InfiltratorRevealed = 5
};

const char* crimeTypeName(CrimeType type);
const char* evidenceKindName(EvidenceKind kind);
const char* caseStatusName(CaseStatus status);
const char* verdictKindName(VerdictKind verdict);
const char* justiceActionName(JusticeAction action);
const char* infiltratorKindName(InfiltratorKind kind);

struct SecurityLocation {
    SecuritySiteId siteId{};
    SecurityStableId areaStableId{};
    int x{};
    int y{};
    int z{};

    friend bool operator==(const SecurityLocation&, const SecurityLocation&) = default;
};

// Objective incident truth belongs to the simulation, not to player/security
// knowledge. A case is created only after actual evidence/reporting reaches the
// CrimeDetectionSystem.
struct CrimeIncident {
    SecurityStableId stableId{};
    CrimeType type{CrimeType::Theft};
    SecurityStableId perpetratorStableId{};
    std::vector<SecurityStableId> victimStableIds;
    SecurityLocation location{};
    SecurityTime timestamp{};
    HistoryPolicy historyPolicy{HistoryPolicy::Ordinary};
};

struct WitnessStatementInput {
    SecurityStableId witnessStableId{};
    // What the witness believes they saw. This can intentionally differ from
    // the objective perpetrator, allowing mistaken/false testimony.
    SecurityStableId believedSubjectStableId{};
    bool supportsAllegation{true};
    float perceptionQuality{1.0f};
    float memoryFidelity{1.0f};
    float bias{0.0f};          // -1..1; magnitude lowers evidentiary reliability.
    float loyalty{0.0f};       // 0..1; stored for inspection/context.
    float fear{0.0f};          // 0..1; reduces willingness/reliability.
    bool willingToReport{true};
};

struct SensorObservationInput {
    SecurityStableId sensorStableId{};
    SecurityStableId observedSubjectStableId{};
    bool supportsAllegation{true};
    bool powered{};
    bool hadCoverage{};
    float confidence{1.0f};
};

struct EvidenceRecord {
    SecurityStableId stableId{};
    SecurityStableId incidentStableId{};
    EvidenceKind kind{EvidenceKind::WitnessStatement};
    SecurityStableId sourceStableId{};   // witness/sensor/investigator/object
    SecurityStableId subjectStableId{};  // who this evidence tends to implicate/exculpate
    SecurityTime timestamp{};
    bool supportsAllegation{true};
    float confidence{1.0f};
    EvidenceIntegrity integrity{EvidenceIntegrity::Intact};

    // Witness context is persisted even when this is not a witness statement;
    // non-witness records leave these fields zero. This keeps the portable
    // record schema narrow while allowing the inspector to explain testimony.
    float perceptionQuality{};
    float memoryFidelity{};
    float bias{};
    float loyalty{};
    float fear{};
};

struct JusticeOutcome {
    SecurityTime timestamp{};
    VerdictKind verdict{VerdictKind::None};
    JusticeAction action{JusticeAction::None};
    SecurityStableId subjectStableId{};
    SecurityStableId authorityStableId{};
};

// Persistent ECS component payload for a first-class case entity.
struct SecurityCase {
    SecurityStableId stableId{};
    SecurityStableId incidentStableId{};
    CrimeType type{CrimeType::Theft};
    std::vector<SecurityStableId> victimStableIds;
    std::vector<SecurityStableId> suspectStableIds;
    std::vector<SecurityStableId> evidenceStableIds;
    SecurityStableId investigatorStableId{};
    CaseStatus status{CaseStatus::Open};
    SecurityStableId accusedStableId{};
    VerdictKind verdict{VerdictKind::None};
    JusticeAction currentAction{JusticeAction::None};
    HistoryPolicy historyPolicy{HistoryPolicy::Ordinary};
    std::vector<JusticeOutcome> outcomeHistory;
};

// Persistent custody component payload. It references a person by durable ID
// and remains valid even if that person is currently represented only by a
// remote/historical record rather than a live ECS entity.
struct PrisonerState {
    SecurityStableId subjectStableId{};
    SecurityStableId caseStableId{};
    JusticeAction action{JusticeAction::None};
    SecurityStableId facilityStableId{};
    SecurityTime since{};
    SecurityTime until{}; // zero = indefinite/policy driven
    bool active{};
};

struct SecurityHistoryEvent {
    SecurityStableId stableId{};
    SecurityHistoryEventType type{SecurityHistoryEventType::Verdict};
    SecurityTime timestamp{};
    SecurityStableId caseStableId{};
    std::vector<SecurityStableId> participants;
};

struct AccessCredential {
    SecurityStableId subjectStableId{};
    // Presented identity can be a cover identity. Access control intentionally
    // does not know the "true" identity unless counterintelligence has revealed
    // it and higher-level policy replaces the credential.
    SecurityStableId presentedIdentityStableId{};
    LegalStatus legalStatus{LegalStatus::Visitor};
    std::uint32_t clearanceLevel{};
    std::vector<std::uint32_t> credentialTags;
};

struct AccessPolicy {
    SecurityStableId stableId{};
    AccessTargetKind targetKind{AccessTargetKind::Other};
    SecurityStableId targetStableId{}; // door/zone/room/etc stable identity
    std::uint32_t minimumClearance{};
    std::vector<std::uint32_t> requiredCredentialTags;
    std::uint32_t allowedLegalStatusMask{0xFFFFFFFFu};
    std::vector<SecurityStableId> explicitAllowSubjects;
    std::vector<SecurityStableId> explicitDenySubjects;
    bool auditDenied{true};
    bool auditAllowed{};
};

struct AccessDecision {
    bool allowed{};
    bool audited{};
    SecurityStableId targetStableId{};
    SecurityStableId subjectStableId{};
    std::string reason;
};

struct AccessAuditRecord {
    SecurityTime timestamp{};
    SecurityStableId policyStableId{};
    SecurityStableId targetStableId{};
    SecurityStableId subjectStableId{};
    SecurityStableId presentedIdentityStableId{};
    bool allowed{};
};

struct InfiltrationState {
    SecurityStableId actorStableId{};
    InfiltratorKind kind{InfiltratorKind::ImperialInformant};
    SecurityStableId coverIdentityStableId{};
    SecurityStableId hiddenGoalStableId{};
    InfiltrationDetectionState detectionState{InfiltrationDetectionState::Hidden};
    float suspicionScore{};
    float revealThreshold{1.5f};
    std::vector<SecurityStableId> evidenceStableIds;
};

struct CaseEvidenceAssessment {
    SecurityStableId caseStableId{};
    SecurityStableId suspectStableId{};
    float netScore{};
    float supportingScore{};
    float exculpatoryScore{};
    int supportingEvidence{};
    int conflictingEvidence{};
    int independentSources{};
    bool sufficient{};
};

struct JusticeTuning {
    float evidenceSufficientScore{1.0f};
    int independentSourcesForSufficiency{1};
    float contaminatedEvidenceFactor{0.45f};
};

// Shared persistent store. In the authoritative EnTT integration each record
// maps to entity/components or cold side-store state keyed by stable ID. This
// portable owner exists so headless tests can prove the rules without making a
// second hidden object hierarchy authoritative.
class SecurityArchive {
public:
    explicit SecurityArchive(std::uint64_t worldSeed = 0, JusticeTuning tuning = {});

    SecurityStableId createIncident(CrimeType type,
                                    SecurityStableId perpetratorStableId,
                                    std::vector<SecurityStableId> victimStableIds,
                                    SecurityLocation location,
                                    SecurityTime timestamp,
                                    HistoryPolicy historyPolicy = HistoryPolicy::Ordinary);

    const CrimeIncident* incident(SecurityStableId stableId) const;
    CrimeIncident* incident(SecurityStableId stableId);
    const EvidenceRecord* evidence(SecurityStableId stableId) const;
    EvidenceRecord* evidence(SecurityStableId stableId);
    const SecurityCase* securityCase(SecurityStableId stableId) const;
    SecurityCase* securityCase(SecurityStableId stableId);
    const SecurityCase* caseForIncident(SecurityStableId incidentStableId) const;
    SecurityCase* caseForIncident(SecurityStableId incidentStableId);
    const PrisonerState* custodyFor(SecurityStableId subjectStableId) const;
    PrisonerState* custodyFor(SecurityStableId subjectStableId);
    const InfiltrationState* infiltration(SecurityStableId actorStableId) const;
    InfiltrationState* infiltration(SecurityStableId actorStableId);

    const std::vector<CrimeIncident>& incidents() const { return incidents_; }
    const std::vector<EvidenceRecord>& evidenceRecords() const { return evidence_; }
    const std::vector<SecurityCase>& cases() const { return cases_; }
    const std::vector<PrisonerState>& custodyStates() const { return custody_; }
    const std::vector<InfiltrationState>& infiltrators() const { return infiltrators_; }
    const std::vector<SecurityHistoryEvent>& historyEvents() const { return history_; }
    const JusticeTuning& tuning() const { return tuning_; }

    std::string serializeState() const;
    bool restoreState(std::string_view text, std::string* error = nullptr);

private:
    friend class CrimeDetectionSystem;
    friend class InvestigationSystem;
    friend class JusticeSystem;
    friend class CustodySystem;
    friend class InfiltrationSystem;

    std::uint64_t worldSeed_{};
    JusticeTuning tuning_{};
    std::uint64_t nextIncidentSerial_{1};
    std::uint64_t nextEvidenceSerial_{1};
    std::uint64_t nextCaseSerial_{1};
    std::uint64_t nextHistorySerial_{1};
    std::vector<CrimeIncident> incidents_;
    std::vector<EvidenceRecord> evidence_;
    std::vector<SecurityCase> cases_;
    std::vector<PrisonerState> custody_;
    std::vector<InfiltrationState> infiltrators_;
    std::vector<SecurityHistoryEvent> history_;

    SecurityStableId allocateStable(std::uint64_t label, std::uint64_t& serial);
    SecurityStableId addEvidence(EvidenceRecord record);
    SecurityCase& ensureCaseForIncident(const CrimeIncident& incident);
    void appendHistory(SecurityHistoryEventType type,
                       SecurityTime timestamp,
                       SecurityStableId caseStableId,
                       std::vector<SecurityStableId> participants);
};

class CrimeDetectionSystem {
public:
    explicit CrimeDetectionSystem(SecurityArchive& archive) : archive_(archive) {}

    // Returns the stable evidence ID, or zero when no observation reaches
    // authorities (unwilling witness, unpowered sensor, no coverage, etc.).
    SecurityStableId submitWitnessStatement(SecurityStableId incidentStableId,
                                            const WitnessStatementInput& statement,
                                            SecurityTime timestamp);
    SecurityStableId submitSensorObservation(SecurityStableId incidentStableId,
                                             const SensorObservationInput& observation,
                                             SecurityTime timestamp);
    // Converts a real access-control audit into ordinary case evidence. A denied
    // or allowed access event does not create knowledge unless a caller ties it
    // to an actual incident under investigation.
    SecurityStableId submitAccessAuditObservation(SecurityStableId incidentStableId,
                                                  const AccessAuditRecord& audit,
                                                  SecurityStableId observedSubjectStableId,
                                                  bool supportsAllegation,
                                                  float confidence);

private:
    SecurityArchive& archive_;
};

class InvestigationSystem {
public:
    explicit InvestigationSystem(SecurityArchive& archive) : archive_(archive) {}

    bool assignInvestigator(SecurityStableId caseStableId, SecurityStableId investigatorStableId);
    SecurityStableId addPhysicalEvidence(SecurityStableId incidentStableId,
                                         EvidenceKind kind,
                                         SecurityStableId sourceStableId,
                                         SecurityStableId subjectStableId,
                                         bool supportsAllegation,
                                         float confidence,
                                         SecurityTime timestamp);
    bool contaminateEvidence(SecurityStableId evidenceStableId);
    bool loseEvidence(SecurityStableId evidenceStableId);
    CaseEvidenceAssessment assess(SecurityStableId caseStableId, SecurityStableId suspectStableId) const;
    // Recomputes the inspectable case band from current evidence without
    // accusing anyone. A case can therefore become EvidenceSufficient and later
    // fall back to Interviewing if evidence is lost/contaminated.
    bool refreshEvidenceStatus(SecurityStableId caseStableId);
    bool accuse(SecurityStableId caseStableId,
                SecurityStableId suspectStableId,
                SecurityStableId authorityStableId,
                SecurityTime timestamp);
    std::string explain(SecurityStableId caseStableId, SecurityStableId suspectStableId) const;

private:
    SecurityArchive& archive_;
};

class JusticeSystem {
public:
    explicit JusticeSystem(SecurityArchive& archive) : archive_(archive) {}

    // Adjudication is policy/authority input. It never peeks at hidden incident
    // perpetrator truth. The justice layer acts only on the case/evidence state
    // and the explicit verdict supplied by governance/legal policy.
    bool adjudicate(SecurityStableId caseStableId,
                    VerdictKind verdict,
                    JusticeAction action,
                    SecurityStableId authorityStableId,
                    SecurityTime timestamp);
    bool pardon(SecurityStableId caseStableId,
                SecurityStableId authorityStableId,
                SecurityTime timestamp);

private:
    SecurityArchive& archive_;
};

class CustodySystem {
public:
    explicit CustodySystem(SecurityArchive& archive) : archive_(archive) {}

    bool applyCaseAction(SecurityStableId caseStableId,
                         SecurityStableId facilityStableId,
                         SecurityTime timestamp,
                         SecurityTime until = 0);
    bool release(SecurityStableId subjectStableId);
    // Releases finite custody sentences that have reached their explicit end.
    // Indefinite/policy-driven custody uses until == 0 and is untouched.
    std::size_t releaseExpired(SecurityTime now);

private:
    SecurityArchive& archive_;
};

class AccessControlSystem {
public:
    AccessDecision evaluate(const AccessCredential& credential,
                            const AccessPolicy& policy,
                            SecurityTime timestamp = 0);
    const std::vector<AccessAuditRecord>& auditLog() const { return auditLog_; }
    void clearAuditLog() { auditLog_.clear(); }

private:
    std::vector<AccessAuditRecord> auditLog_;
};

class InfiltrationSystem {
public:
    explicit InfiltrationSystem(SecurityArchive& archive) : archive_(archive) {}

    bool registerInfiltrator(InfiltrationState state);
    // Adds a pre-existing evidence record to the actor's counterintelligence
    // picture and re-evaluates exposure. Returns true only on the transition to
    // Revealed. No random/omniscient reveal roll exists.
    bool observeEvidence(SecurityStableId actorStableId,
                         SecurityStableId evidenceStableId,
                         SecurityTime timestamp);
    // Recomputes current suspicion from surviving evidence. Suspected actors can
    // return to Hidden when all evidence is lost; Revealed remains sticky because
    // exposure itself is durable knowledge/history.
    bool reevaluateEvidence(SecurityStableId actorStableId);
    std::string explain(SecurityStableId actorStableId) const;

private:
    SecurityArchive& archive_;
};

} // namespace elysium
