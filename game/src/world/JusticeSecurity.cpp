// Intended function: imported world implementation for JusticeSecurity; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/JusticeSecurity.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <set>
#include <sstream>
#include <tuple>

namespace elysium {
namespace {

constexpr std::uint64_t kIncidentLabel = 0x4352494D45494E43ULL; // CRIMEINC
constexpr std::uint64_t kEvidenceLabel = 0x45564944454E4345ULL; // EVIDENCE
constexpr std::uint64_t kCaseLabel = 0x5345434341534531ULL;     // SECCASE1
constexpr std::uint64_t kHistoryLabel = 0x5345434849535431ULL;  // SECHIST1
constexpr std::uint32_t kSchemaVersion = 1;
constexpr const char* kMagic = "ELYSIUM_JUSTICE_SECURITY";

bool finiteUnit(float value) {
    return std::isfinite(value) && value >= 0.0f && value <= 1.0f;
}

bool finiteSignedUnit(float value) {
    return std::isfinite(value) && value >= -1.0f && value <= 1.0f;
}

float clamp01(float value) {
    if (!std::isfinite(value)) return 0.0f;
    return std::clamp(value, 0.0f, 1.0f);
}

bool containsId(const std::vector<SecurityStableId>& values, SecurityStableId id) {
    return std::find(values.begin(), values.end(), id) != values.end();
}

void appendUnique(std::vector<SecurityStableId>& values, SecurityStableId id) {
    if (id != 0 && !containsId(values, id)) values.push_back(id);
}

bool containsTag(const std::vector<std::uint32_t>& tags, std::uint32_t tag) {
    return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

std::uint32_t legalStatusBit(LegalStatus status) {
    return 1u << static_cast<std::uint32_t>(status);
}

float evidenceWeight(const EvidenceRecord& evidence, const JusticeTuning& tuning) {
    if (evidence.integrity == EvidenceIntegrity::Lost) return 0.0f;
    const float integrity = evidence.integrity == EvidenceIntegrity::Contaminated
        ? tuning.contaminatedEvidenceFactor : 1.0f;
    return clamp01(evidence.confidence) * std::max(0.0f, integrity);
}

bool sameEvidenceFingerprint(const EvidenceRecord& a, const EvidenceRecord& b) {
    return a.incidentStableId == b.incidentStableId &&
           a.kind == b.kind &&
           a.sourceStableId == b.sourceStableId &&
           a.subjectStableId == b.subjectStableId &&
           a.supportsAllegation == b.supportsAllegation;
}

template <typename Enum>
bool enumInRange(Enum value, int maxInclusive) {
    const int v = static_cast<int>(value);
    return v >= 0 && v <= maxInclusive;
}

bool validEvidence(const EvidenceRecord& e) {
    return e.stableId != 0 && e.incidentStableId != 0 && e.sourceStableId != 0 &&
           e.subjectStableId != 0 && enumInRange(e.kind, 8) &&
           enumInRange(e.integrity, 2) && finiteUnit(e.confidence) &&
           finiteUnit(e.perceptionQuality) && finiteUnit(e.memoryFidelity) &&
           finiteSignedUnit(e.bias) && finiteUnit(e.loyalty) && finiteUnit(e.fear);
}

bool validCase(const SecurityCase& c) {
    return c.stableId != 0 && c.incidentStableId != 0 && enumInRange(c.type, 8) &&
           enumInRange(c.status, 5) && enumInRange(c.verdict, 4) &&
           enumInRange(c.currentAction, 7) && enumInRange(c.historyPolicy, 1);
}

void setError(std::string* error, const std::string& value) {
    if (error) *error = value;
}

} // namespace

const char* crimeTypeName(CrimeType type) {
    switch (type) {
        case CrimeType::Theft: return "Theft";
        case CrimeType::Assault: return "Assault";
        case CrimeType::Sabotage: return "Sabotage";
        case CrimeType::Murder: return "Murder";
        case CrimeType::Contraband: return "Contraband";
        case CrimeType::Espionage: return "Espionage";
        case CrimeType::MandateViolation: return "Mandate Violation";
        case CrimeType::Fraud: return "Fraud";
        case CrimeType::ArtifactCrime: return "Artifact Crime";
    }
    return "Unknown";
}

const char* evidenceKindName(EvidenceKind kind) {
    switch (kind) {
        case EvidenceKind::WitnessStatement: return "Witness Statement";
        case EvidenceKind::SensorLog: return "Sensor Log";
        case EvidenceKind::ItemProvenance: return "Item Provenance";
        case EvidenceKind::AccessRecord: return "Access Record";
        case EvidenceKind::WoundRecord: return "Wound Record";
        case EvidenceKind::Contraband: return "Contraband";
        case EvidenceKind::MedicalScan: return "Medical Scan";
        case EvidenceKind::BehavioralObservation: return "Behavioral Observation";
        case EvidenceKind::MaintenanceAudit: return "Maintenance Audit";
    }
    return "Unknown";
}

const char* caseStatusName(CaseStatus status) {
    switch (status) {
        case CaseStatus::Open: return "Open";
        case CaseStatus::Interviewing: return "Interviewing";
        case CaseStatus::EvidenceSufficient: return "Evidence Sufficient";
        case CaseStatus::Adjudication: return "Adjudication";
        case CaseStatus::Closed: return "Closed";
        case CaseStatus::Unresolved: return "Unresolved";
    }
    return "Unknown";
}

const char* verdictKindName(VerdictKind verdict) {
    switch (verdict) {
        case VerdictKind::None: return "None";
        case VerdictKind::Acquitted: return "Acquitted";
        case VerdictKind::Convicted: return "Convicted";
        case VerdictKind::Dismissed: return "Dismissed";
        case VerdictKind::Pardoned: return "Pardoned";
    }
    return "Unknown";
}

const char* justiceActionName(JusticeAction action) {
    switch (action) {
        case JusticeAction::None: return "None";
        case JusticeAction::Warning: return "Warning";
        case JusticeAction::Restitution: return "Restitution";
        case JusticeAction::Restriction: return "Restriction";
        case JusticeAction::Detention: return "Detention";
        case JusticeAction::Exile: return "Exile";
        case JusticeAction::FactionHandoff: return "Faction Handoff";
        case JusticeAction::Pardon: return "Pardon";
    }
    return "Unknown";
}

const char* infiltratorKindName(InfiltratorKind kind) {
    switch (kind) {
        case InfiltratorKind::ImperialInformant: return "Imperial Informant";
        case InfiltratorKind::RaiderScout: return "Raider Scout";
        case InfiltratorKind::EngineeredAgent: return "Engineered Agent";
        case InfiltratorKind::RiftMimic: return "Rift Mimic";
        case InfiltratorKind::CompromisedMachine: return "Compromised Machine";
    }
    return "Unknown";
}

SecurityArchive::SecurityArchive(std::uint64_t worldSeed, JusticeTuning tuning)
    : worldSeed_(worldSeed), tuning_(tuning) {
    tuning_.evidenceSufficientScore = std::max(0.0f, tuning_.evidenceSufficientScore);
    tuning_.independentSourcesForSufficiency = std::max(1, tuning_.independentSourcesForSufficiency);
    tuning_.contaminatedEvidenceFactor = std::clamp(tuning_.contaminatedEvidenceFactor, 0.0f, 1.0f);
}

SecurityStableId SecurityArchive::allocateStable(std::uint64_t label, std::uint64_t& serial) {
    for (;;) {
        const auto id = mix64(worldSeed_ ^ label ^ serial++);
        if (id == 0) continue;
        const bool collision =
            std::any_of(incidents_.begin(), incidents_.end(), [id](const CrimeIncident& x) { return x.stableId == id; }) ||
            std::any_of(evidence_.begin(), evidence_.end(), [id](const EvidenceRecord& x) { return x.stableId == id; }) ||
            std::any_of(cases_.begin(), cases_.end(), [id](const SecurityCase& x) { return x.stableId == id; }) ||
            std::any_of(history_.begin(), history_.end(), [id](const SecurityHistoryEvent& x) { return x.stableId == id; });
        if (!collision) return id;
    }
}

SecurityStableId SecurityArchive::createIncident(CrimeType type,
                                                  SecurityStableId perpetratorStableId,
                                                  std::vector<SecurityStableId> victimStableIds,
                                                  SecurityLocation location,
                                                  SecurityTime timestamp,
                                                  HistoryPolicy historyPolicy) {
    if (perpetratorStableId == 0 || !enumInRange(type, 8) || !enumInRange(historyPolicy, 1)) return 0;
    victimStableIds.erase(std::remove(victimStableIds.begin(), victimStableIds.end(), SecurityStableId{0}), victimStableIds.end());
    std::sort(victimStableIds.begin(), victimStableIds.end());
    victimStableIds.erase(std::unique(victimStableIds.begin(), victimStableIds.end()), victimStableIds.end());

    CrimeIncident incidentRecord;
    incidentRecord.stableId = allocateStable(kIncidentLabel, nextIncidentSerial_);
    incidentRecord.type = type;
    incidentRecord.perpetratorStableId = perpetratorStableId;
    incidentRecord.victimStableIds = std::move(victimStableIds);
    incidentRecord.location = location;
    incidentRecord.timestamp = timestamp;
    incidentRecord.historyPolicy = historyPolicy;
    incidents_.push_back(std::move(incidentRecord));
    return incidents_.back().stableId;
}

const CrimeIncident* SecurityArchive::incident(SecurityStableId stableId) const {
    const auto it = std::find_if(incidents_.begin(), incidents_.end(), [stableId](const CrimeIncident& x) { return x.stableId == stableId; });
    return it == incidents_.end() ? nullptr : &*it;
}

CrimeIncident* SecurityArchive::incident(SecurityStableId stableId) {
    return const_cast<CrimeIncident*>(static_cast<const SecurityArchive&>(*this).incident(stableId));
}

const EvidenceRecord* SecurityArchive::evidence(SecurityStableId stableId) const {
    const auto it = std::find_if(evidence_.begin(), evidence_.end(), [stableId](const EvidenceRecord& x) { return x.stableId == stableId; });
    return it == evidence_.end() ? nullptr : &*it;
}

EvidenceRecord* SecurityArchive::evidence(SecurityStableId stableId) {
    return const_cast<EvidenceRecord*>(static_cast<const SecurityArchive&>(*this).evidence(stableId));
}

const SecurityCase* SecurityArchive::securityCase(SecurityStableId stableId) const {
    const auto it = std::find_if(cases_.begin(), cases_.end(), [stableId](const SecurityCase& x) { return x.stableId == stableId; });
    return it == cases_.end() ? nullptr : &*it;
}

SecurityCase* SecurityArchive::securityCase(SecurityStableId stableId) {
    return const_cast<SecurityCase*>(static_cast<const SecurityArchive&>(*this).securityCase(stableId));
}

const SecurityCase* SecurityArchive::caseForIncident(SecurityStableId incidentStableId) const {
    const auto it = std::find_if(cases_.begin(), cases_.end(), [incidentStableId](const SecurityCase& x) { return x.incidentStableId == incidentStableId; });
    return it == cases_.end() ? nullptr : &*it;
}

SecurityCase* SecurityArchive::caseForIncident(SecurityStableId incidentStableId) {
    return const_cast<SecurityCase*>(static_cast<const SecurityArchive&>(*this).caseForIncident(incidentStableId));
}

const PrisonerState* SecurityArchive::custodyFor(SecurityStableId subjectStableId) const {
    const auto it = std::find_if(custody_.begin(), custody_.end(), [subjectStableId](const PrisonerState& x) { return x.subjectStableId == subjectStableId; });
    return it == custody_.end() ? nullptr : &*it;
}

PrisonerState* SecurityArchive::custodyFor(SecurityStableId subjectStableId) {
    return const_cast<PrisonerState*>(static_cast<const SecurityArchive&>(*this).custodyFor(subjectStableId));
}

const InfiltrationState* SecurityArchive::infiltration(SecurityStableId actorStableId) const {
    const auto it = std::find_if(infiltrators_.begin(), infiltrators_.end(), [actorStableId](const InfiltrationState& x) { return x.actorStableId == actorStableId; });
    return it == infiltrators_.end() ? nullptr : &*it;
}

InfiltrationState* SecurityArchive::infiltration(SecurityStableId actorStableId) {
    return const_cast<InfiltrationState*>(static_cast<const SecurityArchive&>(*this).infiltration(actorStableId));
}

SecurityStableId SecurityArchive::addEvidence(EvidenceRecord record) {
    if (record.incidentStableId == 0 || record.sourceStableId == 0 || record.subjectStableId == 0 ||
        !incident(record.incidentStableId)) return 0;

    record.confidence = clamp01(record.confidence);
    record.perceptionQuality = clamp01(record.perceptionQuality);
    record.memoryFidelity = clamp01(record.memoryFidelity);
    record.bias = std::clamp(std::isfinite(record.bias) ? record.bias : 0.0f, -1.0f, 1.0f);
    record.loyalty = clamp01(record.loyalty);
    record.fear = clamp01(record.fear);

    // A single source repeating the same assertion does not manufacture extra
    // evidentiary weight or duplicate a case. Independent sources still add
    // separate records.
    const auto duplicate = std::find_if(evidence_.begin(), evidence_.end(), [&record](const EvidenceRecord& existing) {
        return sameEvidenceFingerprint(existing, record);
    });
    if (duplicate != evidence_.end()) return duplicate->stableId;

    record.stableId = allocateStable(kEvidenceLabel, nextEvidenceSerial_);
    evidence_.push_back(record);
    auto& caseRecord = ensureCaseForIncident(*incident(record.incidentStableId));
    appendUnique(caseRecord.evidenceStableIds, record.stableId);
    appendUnique(caseRecord.suspectStableIds, record.subjectStableId);
    return record.stableId;
}

SecurityCase& SecurityArchive::ensureCaseForIncident(const CrimeIncident& incidentRecord) {
    if (auto* existing = caseForIncident(incidentRecord.stableId)) return *existing;

    SecurityCase caseRecord;
    caseRecord.stableId = allocateStable(kCaseLabel, nextCaseSerial_);
    caseRecord.incidentStableId = incidentRecord.stableId;
    caseRecord.type = incidentRecord.type;
    caseRecord.victimStableIds = incidentRecord.victimStableIds;
    caseRecord.status = CaseStatus::Open;
    caseRecord.historyPolicy = incidentRecord.historyPolicy;
    cases_.push_back(std::move(caseRecord));
    return cases_.back();
}

void SecurityArchive::appendHistory(SecurityHistoryEventType type,
                                    SecurityTime timestamp,
                                    SecurityStableId caseStableId,
                                    std::vector<SecurityStableId> participants) {
    participants.erase(std::remove(participants.begin(), participants.end(), SecurityStableId{0}), participants.end());
    std::sort(participants.begin(), participants.end());
    participants.erase(std::unique(participants.begin(), participants.end()), participants.end());

    SecurityHistoryEvent event;
    event.stableId = allocateStable(kHistoryLabel, nextHistorySerial_);
    event.type = type;
    event.timestamp = timestamp;
    event.caseStableId = caseStableId;
    event.participants = std::move(participants);
    history_.push_back(std::move(event));
}

SecurityStableId CrimeDetectionSystem::submitWitnessStatement(SecurityStableId incidentStableId,
                                                               const WitnessStatementInput& statement,
                                                               SecurityTime timestamp) {
    if (!archive_.incident(incidentStableId) || statement.witnessStableId == 0 ||
        statement.believedSubjectStableId == 0 || !statement.willingToReport) return 0;

    const float perception = clamp01(statement.perceptionQuality);
    const float memory = clamp01(statement.memoryFidelity);
    const float fear = clamp01(statement.fear);
    const float biasMagnitude = std::min(1.0f, std::abs(std::isfinite(statement.bias) ? statement.bias : 0.0f));
    // Context changes reliability but never invents a statement. Loyalty is
    // retained for diagnostics/social systems; fear and bias directly degrade
    // evidentiary confidence in this portable kernel.
    const float loyalty = clamp01(statement.loyalty);
    const float confidence = perception * memory * (1.0f - 0.35f * biasMagnitude) *
                             (1.0f - 0.25f * loyalty) * (1.0f - 0.50f * fear);
    if (confidence <= 0.0f) return 0;

    EvidenceRecord record;
    record.incidentStableId = incidentStableId;
    record.kind = EvidenceKind::WitnessStatement;
    record.sourceStableId = statement.witnessStableId;
    record.subjectStableId = statement.believedSubjectStableId;
    record.timestamp = timestamp;
    record.supportsAllegation = statement.supportsAllegation;
    record.confidence = confidence;
    record.integrity = EvidenceIntegrity::Intact;
    record.perceptionQuality = perception;
    record.memoryFidelity = memory;
    record.bias = std::clamp(std::isfinite(statement.bias) ? statement.bias : 0.0f, -1.0f, 1.0f);
    record.loyalty = loyalty;
    record.fear = fear;
    return archive_.addEvidence(record);
}

SecurityStableId CrimeDetectionSystem::submitSensorObservation(SecurityStableId incidentStableId,
                                                                const SensorObservationInput& observation,
                                                                SecurityTime timestamp) {
    if (!archive_.incident(incidentStableId) || observation.sensorStableId == 0 ||
        observation.observedSubjectStableId == 0 || !observation.powered || !observation.hadCoverage) return 0;
    const float confidence = clamp01(observation.confidence);
    if (confidence <= 0.0f) return 0;

    EvidenceRecord record;
    record.incidentStableId = incidentStableId;
    record.kind = EvidenceKind::SensorLog;
    record.sourceStableId = observation.sensorStableId;
    record.subjectStableId = observation.observedSubjectStableId;
    record.timestamp = timestamp;
    record.supportsAllegation = observation.supportsAllegation;
    record.confidence = confidence;
    record.integrity = EvidenceIntegrity::Intact;
    return archive_.addEvidence(record);
}

SecurityStableId CrimeDetectionSystem::submitAccessAuditObservation(SecurityStableId incidentStableId,
                                                                     const AccessAuditRecord& audit,
                                                                     SecurityStableId observedSubjectStableId,
                                                                     bool supportsAllegation,
                                                                     float confidence) {
    if (!archive_.incident(incidentStableId) || audit.policyStableId == 0 || audit.targetStableId == 0 ||
        observedSubjectStableId == 0 || !std::isfinite(confidence) || confidence <= 0.0f) return 0;
    EvidenceRecord record;
    record.incidentStableId = incidentStableId;
    record.kind = EvidenceKind::AccessRecord;
    // The policy is the durable provenance source. target/presented identity remain
    // inspectable through the original audit owner and do not become hidden truth.
    record.sourceStableId = audit.policyStableId;
    record.subjectStableId = observedSubjectStableId;
    record.timestamp = audit.timestamp;
    record.supportsAllegation = supportsAllegation;
    record.confidence = clamp01(confidence);
    record.integrity = EvidenceIntegrity::Intact;
    return archive_.addEvidence(record);
}

bool InvestigationSystem::assignInvestigator(SecurityStableId caseStableId, SecurityStableId investigatorStableId) {
    auto* caseRecord = archive_.securityCase(caseStableId);
    if (!caseRecord || investigatorStableId == 0 || caseRecord->status == CaseStatus::Closed) return false;
    caseRecord->investigatorStableId = investigatorStableId;
    caseRecord->status = CaseStatus::Interviewing;
    return true;
}

SecurityStableId InvestigationSystem::addPhysicalEvidence(SecurityStableId incidentStableId,
                                                            EvidenceKind kind,
                                                            SecurityStableId sourceStableId,
                                                            SecurityStableId subjectStableId,
                                                            bool supportsAllegation,
                                                            float confidence,
                                                            SecurityTime timestamp) {
    if (!archive_.incident(incidentStableId) || kind == EvidenceKind::WitnessStatement || kind == EvidenceKind::SensorLog ||
        sourceStableId == 0 || subjectStableId == 0 || !std::isfinite(confidence) || confidence <= 0.0f) return 0;
    EvidenceRecord record;
    record.incidentStableId = incidentStableId;
    record.kind = kind;
    record.sourceStableId = sourceStableId;
    record.subjectStableId = subjectStableId;
    record.timestamp = timestamp;
    record.supportsAllegation = supportsAllegation;
    record.confidence = clamp01(confidence);
    record.integrity = EvidenceIntegrity::Intact;
    return archive_.addEvidence(record);
}

bool InvestigationSystem::contaminateEvidence(SecurityStableId evidenceStableId) {
    auto* record = archive_.evidence(evidenceStableId);
    if (!record || record->integrity == EvidenceIntegrity::Lost) return false;
    record->integrity = EvidenceIntegrity::Contaminated;
    return true;
}

bool InvestigationSystem::loseEvidence(SecurityStableId evidenceStableId) {
    auto* record = archive_.evidence(evidenceStableId);
    if (!record) return false;
    record->integrity = EvidenceIntegrity::Lost;
    return true;
}

CaseEvidenceAssessment InvestigationSystem::assess(SecurityStableId caseStableId, SecurityStableId suspectStableId) const {
    CaseEvidenceAssessment assessment;
    assessment.caseStableId = caseStableId;
    assessment.suspectStableId = suspectStableId;
    const auto* caseRecord = archive_.securityCase(caseStableId);
    if (!caseRecord || suspectStableId == 0) return assessment;

    std::set<SecurityStableId> sources;
    for (const auto evidenceId : caseRecord->evidenceStableIds) {
        const auto* record = archive_.evidence(evidenceId);
        if (!record || record->subjectStableId != suspectStableId) continue;
        const float weight = evidenceWeight(*record, archive_.tuning());
        if (weight <= 0.0f) continue;
        sources.insert(record->sourceStableId);
        if (record->supportsAllegation) {
            assessment.supportingScore += weight;
            ++assessment.supportingEvidence;
        } else {
            assessment.exculpatoryScore += weight;
            ++assessment.conflictingEvidence;
        }
    }
    assessment.netScore = assessment.supportingScore - assessment.exculpatoryScore;
    assessment.independentSources = static_cast<int>(sources.size());
    assessment.sufficient = assessment.netScore >= archive_.tuning().evidenceSufficientScore &&
                            assessment.independentSources >= archive_.tuning().independentSourcesForSufficiency;
    return assessment;
}


bool InvestigationSystem::refreshEvidenceStatus(SecurityStableId caseStableId) {
    auto* caseRecord = archive_.securityCase(caseStableId);
    if (!caseRecord || caseRecord->status == CaseStatus::Closed || caseRecord->status == CaseStatus::Adjudication) return false;
    bool anySufficient = false;
    for (const auto suspectId : caseRecord->suspectStableIds) {
        if (assess(caseStableId, suspectId).sufficient) {
            anySufficient = true;
            break;
        }
    }
    caseRecord->status = anySufficient ? CaseStatus::EvidenceSufficient
                                       : (caseRecord->investigatorStableId != 0 ? CaseStatus::Interviewing : CaseStatus::Open);
    return true;
}

bool InvestigationSystem::accuse(SecurityStableId caseStableId,
                                 SecurityStableId suspectStableId,
                                 SecurityStableId authorityStableId,
                                 SecurityTime timestamp) {
    auto* caseRecord = archive_.securityCase(caseStableId);
    if (!caseRecord || suspectStableId == 0 || authorityStableId == 0 || caseRecord->status == CaseStatus::Closed) return false;
    const auto evidenceAssessment = assess(caseStableId, suspectStableId);
    if (!evidenceAssessment.sufficient) return false;

    appendUnique(caseRecord->suspectStableIds, suspectStableId);
    caseRecord->accusedStableId = suspectStableId;
    caseRecord->status = CaseStatus::Adjudication;
    if (caseRecord->historyPolicy == HistoryPolicy::Major) {
        archive_.appendHistory(SecurityHistoryEventType::Accusation, timestamp, caseStableId,
                               {suspectStableId, authorityStableId});
    }
    return true;
}

std::string InvestigationSystem::explain(SecurityStableId caseStableId, SecurityStableId suspectStableId) const {
    const auto* caseRecord = archive_.securityCase(caseStableId);
    if (!caseRecord) return "case not found";
    const auto a = assess(caseStableId, suspectStableId);
    std::ostringstream out;
    out << "case=" << caseStableId << " status=" << caseStatusName(caseRecord->status)
        << " suspect=" << suspectStableId
        << " supporting=" << a.supportingEvidence << "/" << std::fixed << std::setprecision(3) << a.supportingScore
        << " exculpatory=" << a.conflictingEvidence << "/" << a.exculpatoryScore
        << " independent_sources=" << a.independentSources
        << " net=" << a.netScore
        << " sufficient=" << (a.sufficient ? "yes" : "no");
    return out.str();
}

bool JusticeSystem::adjudicate(SecurityStableId caseStableId,
                               VerdictKind verdict,
                               JusticeAction action,
                               SecurityStableId authorityStableId,
                               SecurityTime timestamp) {
    auto* caseRecord = archive_.securityCase(caseStableId);
    if (!caseRecord || authorityStableId == 0 || caseRecord->accusedStableId == 0 ||
        caseRecord->status != CaseStatus::Adjudication || verdict == VerdictKind::None || verdict == VerdictKind::Pardoned) return false;

    if (verdict == VerdictKind::Convicted) {
        if (action == JusticeAction::None || action == JusticeAction::Pardon) return false;
    } else {
        // Acquittal/dismissal cannot silently impose detention/exile/etc.
        action = JusticeAction::None;
    }

    caseRecord->verdict = verdict;
    caseRecord->currentAction = action;
    caseRecord->status = CaseStatus::Closed;
    caseRecord->outcomeHistory.push_back({timestamp, verdict, action, caseRecord->accusedStableId, authorityStableId});

    if (caseRecord->historyPolicy == HistoryPolicy::Major) {
        archive_.appendHistory(SecurityHistoryEventType::Verdict, timestamp, caseStableId,
                               {caseRecord->accusedStableId, authorityStableId});
    }
    return true;
}

bool JusticeSystem::pardon(SecurityStableId caseStableId,
                           SecurityStableId authorityStableId,
                           SecurityTime timestamp) {
    auto* caseRecord = archive_.securityCase(caseStableId);
    if (!caseRecord || authorityStableId == 0 || caseRecord->accusedStableId == 0 ||
        caseRecord->verdict != VerdictKind::Convicted) return false;
    caseRecord->verdict = VerdictKind::Pardoned;
    caseRecord->currentAction = JusticeAction::Pardon;
    caseRecord->status = CaseStatus::Closed;
    caseRecord->outcomeHistory.push_back({timestamp, VerdictKind::Pardoned, JusticeAction::Pardon,
                                          caseRecord->accusedStableId, authorityStableId});
    if (auto* custody = archive_.custodyFor(caseRecord->accusedStableId)) custody->active = false;
    if (caseRecord->historyPolicy == HistoryPolicy::Major) {
        archive_.appendHistory(SecurityHistoryEventType::Pardon, timestamp, caseStableId,
                               {caseRecord->accusedStableId, authorityStableId});
    }
    return true;
}

bool CustodySystem::applyCaseAction(SecurityStableId caseStableId,
                                    SecurityStableId facilityStableId,
                                    SecurityTime timestamp,
                                    SecurityTime until) {
    const auto* caseRecord = archive_.securityCase(caseStableId);
    if (!caseRecord || caseRecord->verdict != VerdictKind::Convicted || caseRecord->accusedStableId == 0) return false;
    const auto action = caseRecord->currentAction;
    if (action != JusticeAction::Restriction && action != JusticeAction::Detention &&
        action != JusticeAction::Exile && action != JusticeAction::FactionHandoff) return false;

    PrisonerState state;
    state.subjectStableId = caseRecord->accusedStableId;
    state.caseStableId = caseStableId;
    state.action = action;
    state.facilityStableId = facilityStableId;
    state.since = timestamp;
    state.until = until;
    state.active = true;

    if (auto* existing = archive_.custodyFor(state.subjectStableId)) *existing = state;
    else archive_.custody_.push_back(state);

    if (caseRecord->historyPolicy == HistoryPolicy::Major) {
        if (action == JusticeAction::Detention) {
            archive_.appendHistory(SecurityHistoryEventType::Detention, timestamp, caseStableId,
                                   {state.subjectStableId, facilityStableId});
        } else if (action == JusticeAction::Exile) {
            archive_.appendHistory(SecurityHistoryEventType::Exile, timestamp, caseStableId,
                                   {state.subjectStableId});
        }
    }
    return true;
}

bool CustodySystem::release(SecurityStableId subjectStableId) {
    auto* state = archive_.custodyFor(subjectStableId);
    if (!state || !state->active) return false;
    state->active = false;
    return true;
}

std::size_t CustodySystem::releaseExpired(SecurityTime now) {
    std::size_t released = 0;
    for (auto& state : archive_.custody_) {
        if (state.active && state.until != 0 && state.until <= now) {
            state.active = false;
            ++released;
        }
    }
    return released;
}

AccessDecision AccessControlSystem::evaluate(const AccessCredential& credential,
                                             const AccessPolicy& policy,
                                             SecurityTime timestamp) {
    AccessDecision decision;
    decision.targetStableId = policy.targetStableId;
    decision.subjectStableId = credential.subjectStableId;

    const SecurityStableId identity = credential.presentedIdentityStableId != 0
        ? credential.presentedIdentityStableId : credential.subjectStableId;

    if (credential.subjectStableId == 0 || policy.stableId == 0 || policy.targetStableId == 0) {
        decision.reason = "invalid identity or policy";
    } else if (containsId(policy.explicitDenySubjects, identity)) {
        decision.reason = "explicit deny";
    } else if (containsId(policy.explicitAllowSubjects, identity)) {
        decision.allowed = true;
        decision.reason = "explicit allow";
    } else if ((policy.allowedLegalStatusMask & legalStatusBit(credential.legalStatus)) == 0) {
        decision.reason = "legal status not allowed";
    } else if (credential.clearanceLevel < policy.minimumClearance) {
        decision.reason = "clearance too low";
    } else {
        const bool tagsOk = std::all_of(policy.requiredCredentialTags.begin(), policy.requiredCredentialTags.end(),
            [&credential](std::uint32_t tag) { return containsTag(credential.credentialTags, tag); });
        if (!tagsOk) decision.reason = "missing credential tag";
        else {
            decision.allowed = true;
            decision.reason = "policy satisfied";
        }
    }

    decision.audited = decision.allowed ? policy.auditAllowed : policy.auditDenied;
    if (decision.audited) {
        auditLog_.push_back({timestamp, policy.stableId, policy.targetStableId, credential.subjectStableId,
                             identity, decision.allowed});
    }
    return decision;
}

bool InfiltrationSystem::registerInfiltrator(InfiltrationState state) {
    if (state.actorStableId == 0 || !enumInRange(state.kind, 4) || !std::isfinite(state.revealThreshold) || state.revealThreshold <= 0.0f ||
        archive_.infiltration(state.actorStableId)) return false;
    state.suspicionScore = std::max(0.0f, std::isfinite(state.suspicionScore) ? state.suspicionScore : 0.0f);
    state.evidenceStableIds.clear();
    state.detectionState = InfiltrationDetectionState::Hidden;
    archive_.infiltrators_.push_back(std::move(state));
    return true;
}

bool InfiltrationSystem::observeEvidence(SecurityStableId actorStableId,
                                         SecurityStableId evidenceStableId,
                                         SecurityTime timestamp) {
    auto* state = archive_.infiltration(actorStableId);
    const auto* evidence = archive_.evidence(evidenceStableId);
    if (!state || !evidence || evidence->subjectStableId != actorStableId || evidence->integrity == EvidenceIntegrity::Lost) return false;
    if (containsId(state->evidenceStableIds, evidenceStableId)) return false;

    state->evidenceStableIds.push_back(evidenceStableId);
    state->suspicionScore += evidenceWeight(*evidence, archive_.tuning());
    if (state->detectionState == InfiltrationDetectionState::Hidden && state->suspicionScore > 0.0f) {
        state->detectionState = InfiltrationDetectionState::Suspected;
    }
    if (state->detectionState != InfiltrationDetectionState::Revealed && state->suspicionScore >= state->revealThreshold) {
        state->detectionState = InfiltrationDetectionState::Revealed;
        archive_.appendHistory(SecurityHistoryEventType::InfiltratorRevealed, timestamp, 0,
                               {actorStableId, state->coverIdentityStableId});
        return true;
    }
    return false;
}

bool InfiltrationSystem::reevaluateEvidence(SecurityStableId actorStableId) {
    auto* state = archive_.infiltration(actorStableId);
    if (!state) return false;
    if (state->detectionState == InfiltrationDetectionState::Revealed) return true;

    float score = 0.0f;
    for (const auto evidenceId : state->evidenceStableIds) {
        const auto* record = archive_.evidence(evidenceId);
        if (!record || record->subjectStableId != actorStableId || record->integrity == EvidenceIntegrity::Lost) continue;
        score += evidenceWeight(*record, archive_.tuning());
    }
    state->suspicionScore = score;
    state->detectionState = score > 0.0f ? InfiltrationDetectionState::Suspected : InfiltrationDetectionState::Hidden;
    return true;
}

std::string InfiltrationSystem::explain(SecurityStableId actorStableId) const {
    const auto* state = archive_.infiltration(actorStableId);
    if (!state) return "infiltrator not found";
    std::ostringstream out;
    out << "actor=" << actorStableId << " type=" << infiltratorKindName(state->kind)
        << " state=" << static_cast<int>(state->detectionState)
        << " evidence=" << state->evidenceStableIds.size()
        << " suspicion=" << std::fixed << std::setprecision(3) << state->suspicionScore
        << "/" << state->revealThreshold;
    return out.str();
}

std::string SecurityArchive::serializeState() const {
    std::ostringstream out;
    out << std::setprecision(9);
    out << kMagic << ' ' << kSchemaVersion << '\n';
    out << "META " << worldSeed_ << ' ' << nextIncidentSerial_ << ' ' << nextEvidenceSerial_ << ' '
        << nextCaseSerial_ << ' ' << nextHistorySerial_ << ' ' << tuning_.evidenceSufficientScore << ' '
        << tuning_.independentSourcesForSufficiency << ' ' << tuning_.contaminatedEvidenceFactor << '\n';

    out << "INCIDENTS " << incidents_.size() << '\n';
    for (const auto& i : incidents_) {
        out << "I " << i.stableId << ' ' << static_cast<int>(i.type) << ' ' << i.perpetratorStableId << ' '
            << i.location.siteId << ' ' << i.location.areaStableId << ' ' << i.location.x << ' ' << i.location.y << ' '
            << i.location.z << ' ' << i.timestamp << ' ' << static_cast<int>(i.historyPolicy) << ' ' << i.victimStableIds.size();
        for (const auto id : i.victimStableIds) out << ' ' << id;
        out << '\n';
    }

    out << "EVIDENCE " << evidence_.size() << '\n';
    for (const auto& e : evidence_) {
        out << "E " << e.stableId << ' ' << e.incidentStableId << ' ' << static_cast<int>(e.kind) << ' '
            << e.sourceStableId << ' ' << e.subjectStableId << ' ' << e.timestamp << ' ' << (e.supportsAllegation ? 1 : 0) << ' '
            << e.confidence << ' ' << static_cast<int>(e.integrity) << ' ' << e.perceptionQuality << ' '
            << e.memoryFidelity << ' ' << e.bias << ' ' << e.loyalty << ' ' << e.fear << '\n';
    }

    out << "CASES " << cases_.size() << '\n';
    for (const auto& c : cases_) {
        out << "C " << c.stableId << ' ' << c.incidentStableId << ' ' << static_cast<int>(c.type) << ' '
            << c.investigatorStableId << ' ' << static_cast<int>(c.status) << ' ' << c.accusedStableId << ' '
            << static_cast<int>(c.verdict) << ' ' << static_cast<int>(c.currentAction) << ' ' << static_cast<int>(c.historyPolicy) << ' '
            << c.victimStableIds.size();
        for (const auto id : c.victimStableIds) out << ' ' << id;
        out << ' ' << c.suspectStableIds.size();
        for (const auto id : c.suspectStableIds) out << ' ' << id;
        out << ' ' << c.evidenceStableIds.size();
        for (const auto id : c.evidenceStableIds) out << ' ' << id;
        out << ' ' << c.outcomeHistory.size();
        for (const auto& h : c.outcomeHistory) {
            out << ' ' << h.timestamp << ' ' << static_cast<int>(h.verdict) << ' ' << static_cast<int>(h.action)
                << ' ' << h.subjectStableId << ' ' << h.authorityStableId;
        }
        out << '\n';
    }

    out << "CUSTODY " << custody_.size() << '\n';
    for (const auto& c : custody_) {
        out << "P " << c.subjectStableId << ' ' << c.caseStableId << ' ' << static_cast<int>(c.action) << ' '
            << c.facilityStableId << ' ' << c.since << ' ' << c.until << ' ' << (c.active ? 1 : 0) << '\n';
    }

    out << "INFILTRATORS " << infiltrators_.size() << '\n';
    for (const auto& i : infiltrators_) {
        out << "F " << i.actorStableId << ' ' << static_cast<int>(i.kind) << ' ' << i.coverIdentityStableId << ' '
            << i.hiddenGoalStableId << ' ' << static_cast<int>(i.detectionState) << ' ' << i.suspicionScore << ' '
            << i.revealThreshold << ' ' << i.evidenceStableIds.size();
        for (const auto id : i.evidenceStableIds) out << ' ' << id;
        out << '\n';
    }

    out << "HISTORY " << history_.size() << '\n';
    for (const auto& h : history_) {
        out << "H " << h.stableId << ' ' << static_cast<int>(h.type) << ' ' << h.timestamp << ' ' << h.caseStableId << ' '
            << h.participants.size();
        for (const auto id : h.participants) out << ' ' << id;
        out << '\n';
    }
    out << "END\n";
    return out.str();
}

bool SecurityArchive::restoreState(std::string_view text, std::string* error) {
    std::istringstream in{std::string(text)};
    std::string token;
    std::uint32_t schema{};
    if (!(in >> token >> schema) || token != kMagic || schema != kSchemaVersion) {
        setError(error, "unsupported justice/security header");
        return false;
    }

    SecurityArchive restored;
    if (!(in >> token) || token != "META") {
        setError(error, "missing META record");
        return false;
    }
    if (!(in >> restored.worldSeed_ >> restored.nextIncidentSerial_ >> restored.nextEvidenceSerial_
             >> restored.nextCaseSerial_ >> restored.nextHistorySerial_
             >> restored.tuning_.evidenceSufficientScore >> restored.tuning_.independentSourcesForSufficiency
             >> restored.tuning_.contaminatedEvidenceFactor)) {
        setError(error, "malformed META record");
        return false;
    }
    if (!std::isfinite(restored.tuning_.evidenceSufficientScore) || restored.tuning_.evidenceSufficientScore < 0.0f ||
        restored.tuning_.independentSourcesForSufficiency < 1 ||
        !finiteUnit(restored.tuning_.contaminatedEvidenceFactor)) {
        setError(error, "invalid tuning values");
        return false;
    }

    std::size_t count{};
    if (!(in >> token >> count) || token != "INCIDENTS") { setError(error, "missing INCIDENTS section"); return false; }
    restored.incidents_.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        CrimeIncident record;
        int type{}, history{};
        std::size_t victims{};
        if (!(in >> token) || token != "I" ||
            !(in >> record.stableId >> type >> record.perpetratorStableId >> record.location.siteId >> record.location.areaStableId
                 >> record.location.x >> record.location.y >> record.location.z >> record.timestamp >> history >> victims)) {
            setError(error, "malformed incident record"); return false;
        }
        record.type = static_cast<CrimeType>(type);
        record.historyPolicy = static_cast<HistoryPolicy>(history);
        if (record.stableId == 0 || record.perpetratorStableId == 0 || !enumInRange(record.type, 8) || !enumInRange(record.historyPolicy, 1)) {
            setError(error, "invalid incident record"); return false;
        }
        record.victimStableIds.resize(victims);
        for (auto& id : record.victimStableIds) if (!(in >> id)) { setError(error, "truncated incident victims"); return false; }
        restored.incidents_.push_back(std::move(record));
    }

    if (!(in >> token >> count) || token != "EVIDENCE") { setError(error, "missing EVIDENCE section"); return false; }
    restored.evidence_.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        EvidenceRecord record;
        int kind{}, support{}, integrity{};
        if (!(in >> token) || token != "E" ||
            !(in >> record.stableId >> record.incidentStableId >> kind >> record.sourceStableId >> record.subjectStableId
                 >> record.timestamp >> support >> record.confidence >> integrity >> record.perceptionQuality
                 >> record.memoryFidelity >> record.bias >> record.loyalty >> record.fear)) {
            setError(error, "malformed evidence record"); return false;
        }
        record.kind = static_cast<EvidenceKind>(kind);
        record.supportsAllegation = support != 0;
        record.integrity = static_cast<EvidenceIntegrity>(integrity);
        if (!validEvidence(record)) { setError(error, "invalid evidence record"); return false; }
        restored.evidence_.push_back(record);
    }

    if (!(in >> token >> count) || token != "CASES") { setError(error, "missing CASES section"); return false; }
    restored.cases_.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        SecurityCase record;
        int type{}, status{}, verdict{}, action{}, history{};
        std::size_t n{};
        if (!(in >> token) || token != "C" ||
            !(in >> record.stableId >> record.incidentStableId >> type >> record.investigatorStableId >> status
                 >> record.accusedStableId >> verdict >> action >> history >> n)) {
            setError(error, "malformed case record"); return false;
        }
        record.type = static_cast<CrimeType>(type);
        record.status = static_cast<CaseStatus>(status);
        record.verdict = static_cast<VerdictKind>(verdict);
        record.currentAction = static_cast<JusticeAction>(action);
        record.historyPolicy = static_cast<HistoryPolicy>(history);
        record.victimStableIds.resize(n);
        for (auto& id : record.victimStableIds) if (!(in >> id)) { setError(error, "truncated case victims"); return false; }
        if (!(in >> n)) { setError(error, "missing case suspects count"); return false; }
        record.suspectStableIds.resize(n);
        for (auto& id : record.suspectStableIds) if (!(in >> id)) { setError(error, "truncated case suspects"); return false; }
        if (!(in >> n)) { setError(error, "missing case evidence count"); return false; }
        record.evidenceStableIds.resize(n);
        for (auto& id : record.evidenceStableIds) if (!(in >> id)) { setError(error, "truncated case evidence"); return false; }
        if (!(in >> n)) { setError(error, "missing case outcome count"); return false; }
        record.outcomeHistory.resize(n);
        for (auto& h : record.outcomeHistory) {
            int hv{}, ha{};
            if (!(in >> h.timestamp >> hv >> ha >> h.subjectStableId >> h.authorityStableId)) {
                setError(error, "truncated case outcome"); return false;
            }
            h.verdict = static_cast<VerdictKind>(hv);
            h.action = static_cast<JusticeAction>(ha);
            if (!enumInRange(h.verdict, 4) || !enumInRange(h.action, 7)) { setError(error, "invalid case outcome"); return false; }
        }
        if (!validCase(record)) { setError(error, "invalid case record"); return false; }
        restored.cases_.push_back(std::move(record));
    }

    if (!(in >> token >> count) || token != "CUSTODY") { setError(error, "missing CUSTODY section"); return false; }
    restored.custody_.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        PrisonerState record;
        int action{}, active{};
        if (!(in >> token) || token != "P" ||
            !(in >> record.subjectStableId >> record.caseStableId >> action >> record.facilityStableId >> record.since >> record.until >> active)) {
            setError(error, "malformed custody record"); return false;
        }
        record.action = static_cast<JusticeAction>(action);
        record.active = active != 0;
        if (record.subjectStableId == 0 || record.caseStableId == 0 || !enumInRange(record.action, 7)) {
            setError(error, "invalid custody record"); return false;
        }
        restored.custody_.push_back(record);
    }

    if (!(in >> token >> count) || token != "INFILTRATORS") { setError(error, "missing INFILTRATORS section"); return false; }
    restored.infiltrators_.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        InfiltrationState record;
        int kind{}, state{};
        std::size_t n{};
        if (!(in >> token) || token != "F" ||
            !(in >> record.actorStableId >> kind >> record.coverIdentityStableId >> record.hiddenGoalStableId >> state
                 >> record.suspicionScore >> record.revealThreshold >> n)) {
            setError(error, "malformed infiltrator record"); return false;
        }
        record.kind = static_cast<InfiltratorKind>(kind);
        record.detectionState = static_cast<InfiltrationDetectionState>(state);
        record.evidenceStableIds.resize(n);
        for (auto& id : record.evidenceStableIds) if (!(in >> id)) { setError(error, "truncated infiltrator evidence"); return false; }
        if (record.actorStableId == 0 || !enumInRange(record.kind, 4) || !enumInRange(record.detectionState, 2) ||
            !std::isfinite(record.suspicionScore) || record.suspicionScore < 0.0f ||
            !std::isfinite(record.revealThreshold) || record.revealThreshold <= 0.0f) {
            setError(error, "invalid infiltrator record"); return false;
        }
        restored.infiltrators_.push_back(std::move(record));
    }

    if (!(in >> token >> count) || token != "HISTORY") { setError(error, "missing HISTORY section"); return false; }
    restored.history_.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        SecurityHistoryEvent record;
        int type{};
        std::size_t n{};
        if (!(in >> token) || token != "H" || !(in >> record.stableId >> type >> record.timestamp >> record.caseStableId >> n)) {
            setError(error, "malformed history record"); return false;
        }
        record.type = static_cast<SecurityHistoryEventType>(type);
        record.participants.resize(n);
        for (auto& id : record.participants) if (!(in >> id)) { setError(error, "truncated history participants"); return false; }
        if (record.stableId == 0 || !enumInRange(record.type, 5)) { setError(error, "invalid history record"); return false; }
        restored.history_.push_back(std::move(record));
    }

    if (!(in >> token) || token != "END") { setError(error, "missing END record"); return false; }

    // Cross-record validation intentionally requires stable references to be
    // syntactically coherent but does not require referenced people to be live
    // entities. That is how remote/absent historical figures survive cases.
    std::set<SecurityStableId> incidentIds;
    std::set<SecurityStableId> evidenceIds;
    std::set<SecurityStableId> caseIds;
    std::set<SecurityStableId> historyIds;
    for (const auto& record : restored.incidents_) {
        if (!incidentIds.insert(record.stableId).second) { setError(error, "duplicate incident ID"); return false; }
    }
    for (const auto& record : restored.evidence_) {
        if (!evidenceIds.insert(record.stableId).second || !incidentIds.count(record.incidentStableId)) {
            setError(error, "dangling/duplicate evidence record"); return false;
        }
    }
    std::set<SecurityStableId> caseIncidentIds;
    for (const auto& record : restored.cases_) {
        if (!caseIds.insert(record.stableId).second || !incidentIds.count(record.incidentStableId) ||
            !caseIncidentIds.insert(record.incidentStableId).second) {
            setError(error, "dangling/duplicate case record"); return false;
        }
        for (const auto id : record.evidenceStableIds) {
            if (!evidenceIds.count(id)) { setError(error, "case references missing evidence"); return false; }
        }
    }
    for (const auto& record : restored.custody_) {
        if (!caseIds.count(record.caseStableId)) { setError(error, "custody references missing case"); return false; }
    }
    for (const auto& record : restored.infiltrators_) {
        for (const auto id : record.evidenceStableIds) {
            if (!evidenceIds.count(id)) { setError(error, "infiltrator references missing evidence"); return false; }
        }
    }
    for (const auto& record : restored.history_) {
        if (!historyIds.insert(record.stableId).second) { setError(error, "duplicate history ID"); return false; }
        if (record.caseStableId != 0 && !caseIds.count(record.caseStableId)) {
            setError(error, "history references missing case"); return false;
        }
    }

    *this = std::move(restored);
    if (error) error->clear();
    return true;
}

} // namespace elysium
