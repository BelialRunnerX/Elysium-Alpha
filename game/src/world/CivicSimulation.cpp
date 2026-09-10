// Intended function: imported world implementation for CivicSimulation; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/CivicSimulation.hpp"
#include "core/Determinism.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <utility>

namespace elysium {
namespace {

constexpr std::uint64_t kCivicMemberLabel = 0x43495649434D454DULL; // "CIVICMEM"

float clamp01(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

template <typename T>
bool contains(const std::vector<T>& values, const T& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

template <typename T>
void sortUnique(std::vector<T>& values) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

bool validInstitutionType(int value) {
    return value >= static_cast<int>(InstitutionType::Cantina) &&
           value <= static_cast<int>(InstitutionType::Academy);
}

bool validNeed(int value) {
    return value >= static_cast<int>(CivicNeed::Social) &&
           value < static_cast<int>(CivicNeed::Count);
}

bool validLegalStatus(int value) {
    return value >= static_cast<int>(LegalStatus::Visitor) &&
           value <= static_cast<int>(LegalStatus::Exiled);
}

bool validRelationshipType(int value) {
    return value >= static_cast<int>(RelationshipType::Partner) &&
           value <= static_cast<int>(RelationshipType::Household);
}

bool validHistoryType(int value) {
    return value >= static_cast<int>(CivicHistoryEventType::InstitutionFounded) &&
           value <= static_cast<int>(CivicHistoryEventType::CulturalWorkTransmission);
}

bool validTransmissionType(int value) {
    return value >= static_cast<int>(CulturalTransmissionType::Copied) &&
           value <= static_cast<int>(CulturalTransmissionType::Archived);
}

bool validDetail(int value) {
    return value >= static_cast<int>(CivicSimulationDetail::ActiveFortress) &&
           value <= static_cast<int>(CivicSimulationDetail::Strategic);
}

void appendFactor(MigrationDecision& out, std::string label, float value, float weight) {
    const float clamped = clamp01(value);
    const float contribution = clamped * weight;
    out.factors.push_back({std::move(label), clamped, weight, contribution});
    out.score += contribution;
}

bool readQuotedVector(std::istream& in, std::vector<std::string>& out, std::size_t count,
                      const char* expectedToken, std::string* error) {
    out.clear();
    out.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        std::string token;
        std::string value;
        if (!(in >> token) || token != expectedToken || !(in >> std::quoted(value))) {
            if (error) *error = std::string("invalid civic vector entry: expected ") + expectedToken;
            return false;
        }
        out.push_back(std::move(value));
    }
    return true;
}

} // namespace

const char* institutionTypeName(InstitutionType type) {
    switch (type) {
        case InstitutionType::Cantina: return "cantina";
        case InstitutionType::Shrine: return "shrine";
        case InstitutionType::Guildhall: return "guildhall";
        case InstitutionType::Archive: return "archive";
        case InstitutionType::Hospital: return "hospital";
        case InstitutionType::Memorial: return "memorial";
        case InstitutionType::Forum: return "forum";
        case InstitutionType::Academy: return "academy";
    }
    return "unknown";
}

const char* legalStatusName(LegalStatus status) {
    switch (status) {
        case LegalStatus::Visitor: return "visitor";
        case LegalStatus::GuestWorker: return "guest-worker";
        case LegalStatus::Resident: return "resident";
        case LegalStatus::Citizen: return "citizen";
        case LegalStatus::ProtectedRefugee: return "protected-refugee";
        case LegalStatus::Restricted: return "restricted";
        case LegalStatus::Exiled: return "exiled";
    }
    return "unknown";
}

const char* civicNeedName(CivicNeed need) {
    switch (need) {
        case CivicNeed::Social: return "social";
        case CivicNeed::Reflection: return "reflection";
        case CivicNeed::Learning: return "learning";
        case CivicNeed::Health: return "health";
        case CivicNeed::Grief: return "grief";
        case CivicNeed::Civic: return "civic";
        case CivicNeed::Training: return "training";
        case CivicNeed::Count: break;
    }
    return "unknown";
}

const char* culturalTransmissionTypeName(CulturalTransmissionType type) {
    switch (type) {
        case CulturalTransmissionType::Copied: return "copied";
        case CulturalTransmissionType::Performed: return "performed";
        case CulturalTransmissionType::Traded: return "traded";
        case CulturalTransmissionType::Censored: return "censored";
        case CulturalTransmissionType::Archived: return "archived";
    }
    return "unknown";
}

const char* arrivalTypeName(ArrivalType type) {
    switch (type) {
        case ArrivalType::Founder: return "founder";
        case ArrivalType::Migrant: return "migrant";
        case ArrivalType::Refugee: return "refugee";
        case ArrivalType::Contractor: return "contractor";
        case ArrivalType::PerformerScholar: return "performer-scholar";
        case ArrivalType::MercenaryHunter: return "mercenary-hunter";
        case ArrivalType::TraderCrew: return "trader-crew";
        case ArrivalType::ImperialOfficial: return "imperial-official";
        case ArrivalType::Infiltrator: return "infiltrator";
        case ArrivalType::PilgrimGuild: return "pilgrim-guild";
    }
    return "unknown";
}

CivicSimulation::CivicSimulation(std::uint64_t deterministicSeed)
    : seed_(deterministicSeed == 0 ? 1 : deterministicSeed) {}

CivicStableId CivicSimulation::allocateStableId(CivicStableId preferred) {
    if (preferred != 0) {
        if (cultures_.contains(preferred) || citizens_.contains(preferred) || institutions_.contains(preferred) || works_.contains(preferred)) {
            return 0;
        }
        nextStableId_ = std::max(nextStableId_, preferred + 1);
        return preferred;
    }
    while (nextStableId_ == 0 || cultures_.contains(nextStableId_) || citizens_.contains(nextStableId_) ||
           institutions_.contains(nextStableId_) || works_.contains(nextStableId_)) {
        ++nextStableId_;
    }
    return nextStableId_++;
}

CivicStableId CivicSimulation::ensureUniqueDerived(CivicStableId candidate) const {
    if (candidate == 0) candidate = 1;
    std::uint64_t nonce = 0;
    while (cultures_.contains(candidate) || citizens_.contains(candidate) || institutions_.contains(candidate) || works_.contains(candidate)) {
        candidate = mix64(candidate ^ (++nonce * 0x9e3779b97f4a7c15ULL));
        if (candidate == 0) candidate = nonce + 1;
    }
    return candidate;
}

CivicStableId CivicSimulation::deriveMemberStableId(CivicStableId settlementId, CivicStableId groupId,
                                                     std::size_t memberIndex) const {
    std::uint64_t h = mix64(seed_ ^ kCivicMemberLabel);
    h = mix64(h ^ settlementId);
    h = mix64(h ^ groupId);
    h = mix64(h ^ static_cast<std::uint64_t>(memberIndex + 1));
    return ensureUniqueDerived(h == 0 ? 1 : h);
}

CivicStableId CivicSimulation::appendHistory(CivicHistoryEventType type, CivicStableId settlementId,
                                             CivicStableId subjectId, CivicStableId relatedId,
                                             double hour, std::string detail) {
    const CivicStableId id = allocateStableId();
    if (id == 0) return 0;
    history_.push_back({id, type, settlementId, subjectId, relatedId, hour, std::move(detail)});
    return id;
}

CivicStableId CivicSimulation::addCulture(CultureRecord cultureRecord, CivicStableId preferredStableId) {
    const CivicStableId id = allocateStableId(preferredStableId != 0 ? preferredStableId : cultureRecord.stableId);
    if (id == 0) return 0;
    cultureRecord.stableId = id;
    cultures_.emplace(id, std::move(cultureRecord));
    return id;
}

void CivicSimulation::applyDefaultInstitutionServices(InstitutionRecord& record) const {
    record.services.clear();
    record.education.reset();
    switch (record.type) {
        case InstitutionType::Cantina:
            record.services.push_back({CivicNeed::Social, 0.35f, "food-drink", 1});
            break;
        case InstitutionType::Shrine:
            record.services.push_back({CivicNeed::Reflection, 0.40f, {}, 0});
            break;
        case InstitutionType::Guildhall:
            record.services.push_back({CivicNeed::Learning, 0.25f, {}, 0});
            record.education = EducationProgram{"craft", 2.0f, 0.15f};
            break;
        case InstitutionType::Archive:
            record.services.push_back({CivicNeed::Learning, 0.35f, "data-media", 0});
            record.education = EducationProgram{"research", 2.0f, 0.12f};
            break;
        case InstitutionType::Hospital:
            record.services.push_back({CivicNeed::Health, 0.50f, "medical", 1});
            break;
        case InstitutionType::Memorial:
            record.services.push_back({CivicNeed::Grief, 0.45f, {}, 0});
            break;
        case InstitutionType::Forum:
            record.services.push_back({CivicNeed::Civic, 0.30f, {}, 0});
            break;
        case InstitutionType::Academy:
            record.services.push_back({CivicNeed::Training, 0.35f, {}, 0});
            record.education = EducationProgram{"tactics", 2.0f, 0.12f};
            break;
    }
}

CivicStableId CivicSimulation::createInstitution(InstitutionType type,
                                                 CivicStableId settlementId,
                                                 CivicStableId cultureId,
                                                 std::string name,
                                                 CivicSchedule schedule,
                                                 CivicStableId preferredStableId) {
    if (settlementId == 0 || (cultureId != 0 && !cultures_.contains(cultureId))) return 0;
    const CivicStableId id = allocateStableId(preferredStableId);
    if (id == 0) return 0;
    if (schedule.activityPeriodHours <= 0.0f) schedule.activityPeriodHours = 6.0f;
    InstitutionRecord record;
    record.stableId = id;
    record.settlementId = settlementId;
    record.cultureId = cultureId;
    record.type = type;
    record.name = std::move(name);
    record.schedule = schedule;
    applyDefaultInstitutionServices(record);
    institutions_.emplace(id, std::move(record));
    appendHistory(CivicHistoryEventType::InstitutionFounded, settlementId, id, 0,
                  schedule.nextActivityHour, std::string(institutionTypeName(type)) + " founded");
    return id;
}

bool CivicSimulation::dissolveInstitution(CivicStableId institutionId, double hour, std::string reason) {
    auto* record = institution(institutionId);
    if (!record || !record->active) return false;
    record->active = false;
    for (const CivicStableId citizenId : record->memberIds) {
        if (auto* c = citizen(citizenId)) {
            c->institutionMemberships.erase(std::remove(c->institutionMemberships.begin(), c->institutionMemberships.end(), institutionId),
                                            c->institutionMemberships.end());
        }
    }
    record->memberIds.clear();
    appendHistory(CivicHistoryEventType::InstitutionDissolved, record->settlementId, institutionId, 0, hour, std::move(reason));
    return true;
}

bool CivicSimulation::recordInstitutionSchism(CivicStableId institutionId, CivicStableId childInstitutionId,
                                              double hour, std::string reason) {
    const auto* parent = institution(institutionId);
    const auto* child = institution(childInstitutionId);
    if (!parent || !child || parent->settlementId != child->settlementId) return false;
    appendHistory(CivicHistoryEventType::InstitutionSchism, parent->settlementId, institutionId, childInstitutionId, hour,
                  std::move(reason));
    return true;
}

bool CivicSimulation::addInstitutionMember(CivicStableId institutionId, CivicStableId citizenId) {
    auto* record = institution(institutionId);
    auto* c = citizen(citizenId);
    if (!record || !c || !record->active || !c->present || c->settlementId != record->settlementId || c->legalStatus == LegalStatus::Exiled) return false;
    if (!contains(record->memberIds, citizenId)) record->memberIds.push_back(citizenId);
    if (!contains(c->institutionMemberships, institutionId)) c->institutionMemberships.push_back(institutionId);
    sortUnique(record->memberIds);
    sortUnique(c->institutionMemberships);
    return true;
}

bool CivicSimulation::removeInstitutionMember(CivicStableId institutionId, CivicStableId citizenId) {
    auto* record = institution(institutionId);
    auto* c = citizen(citizenId);
    if (!record || !c) return false;
    const auto oldMemberCount = record->memberIds.size();
    record->memberIds.erase(std::remove(record->memberIds.begin(), record->memberIds.end(), citizenId), record->memberIds.end());
    c->institutionMemberships.erase(std::remove(c->institutionMemberships.begin(), c->institutionMemberships.end(), institutionId),
                                    c->institutionMemberships.end());
    return oldMemberCount != record->memberIds.size();
}

bool CivicSimulation::setInstitutionSupply(CivicStableId institutionId, std::string supplyTag, int count) {
    auto* record = institution(institutionId);
    if (!record || supplyTag.empty() || count < 0) return false;
    if (count == 0) record->supplies.erase(supplyTag);
    else record->supplies[std::move(supplyTag)] = count;
    return true;
}

bool CivicSimulation::setInstitutionStaff(CivicStableId institutionId, int count) {
    auto* record = institution(institutionId);
    if (!record || count < 0) return false;
    record->staffAvailable = count;
    return true;
}

bool CivicSimulation::setEducationProgram(CivicStableId institutionId, EducationProgram program) {
    auto* record = institution(institutionId);
    if (!record || program.skill.empty() || program.minimumMentorRank < 0.0f || program.learnerGain <= 0.0f) return false;
    record->education = std::move(program);
    return true;
}

bool CivicSimulation::policyAllows(ArrivalType type, const MigrationPolicy& policy) const {
    switch (type) {
        case ArrivalType::Founder: return true;
        case ArrivalType::Migrant: return policy.allowMigrants;
        case ArrivalType::Refugee: return policy.allowRefugees;
        case ArrivalType::Contractor:
        case ArrivalType::MercenaryHunter: return policy.allowGuestWorkers;
        case ArrivalType::PerformerScholar:
        case ArrivalType::TraderCrew:
        case ArrivalType::ImperialOfficial:
        case ArrivalType::Infiltrator:
        case ArrivalType::PilgrimGuild: return policy.allowVisitors;
    }
    return false;
}

MigrationDecision CivicSimulation::evaluateMigration(const MigrationGroup& group,
                                                     const SettlementMigrationInputs& inputs,
                                                     const MigrationPolicy& policy) const {
    MigrationDecision out;
    out.policyAllowed = policyAllows(group.type, policy);
    out.threshold = 0.46f;
    if (group.type == ArrivalType::Refugee) out.threshold = 0.30f;
    else if (group.type == ArrivalType::Founder) out.threshold = 0.0f;
    else if (group.type == ArrivalType::ImperialOfficial) out.threshold = 0.20f;
    else if (group.type == ArrivalType::Infiltrator) out.threshold = 0.35f;

    appendFactor(out, "safety", inputs.safety, 0.19f);
    appendFactor(out, "work-demand", inputs.workDemand, 0.13f);
    appendFactor(out, "reputation", inputs.reputation, 0.12f);
    appendFactor(out, "cultural-affinity", inputs.culturalAffinity, 0.10f);
    appendFactor(out, "policy-openness", inputs.policyOpenness, 0.09f);
    appendFactor(out, "housing", inputs.housing, 0.13f);
    appendFactor(out, "food-security", inputs.foodSecurity, 0.10f);
    appendFactor(out, "institution-coverage", inputs.institutionCoverage, 0.07f);
    appendFactor(out, "history-pull", inputs.historyPull, 0.04f);
    appendFactor(out, "faction-compatibility", inputs.factionCompatibility, 0.08f);
    appendFactor(out, "active-threat", inputs.activeThreat, -0.18f);
    appendFactor(out, "group-urgency", group.urgency, group.type == ArrivalType::Refugee ? 0.14f : 0.04f);

    out.score = std::clamp(out.score, -1.0f, 1.0f);
    out.accepted = out.policyAllowed && out.score >= out.threshold && !group.members.empty() && group.groupStableId != 0;
    if (!out.policyAllowed) out.summary = std::string(arrivalTypeName(group.type)) + " blocked by settlement migration policy";
    else if (group.members.empty()) out.summary = "arrival has no members";
    else if (group.groupStableId == 0) out.summary = "arrival lacks stable group identity";
    else if (out.accepted) out.summary = "arrival accepted from policy/site/social inputs";
    else out.summary = "arrival declined because weighted settlement conditions are below threshold";
    return out;
}

LegalStatus CivicSimulation::initialStatusFor(ArrivalType type) const {
    switch (type) {
        case ArrivalType::Founder: return LegalStatus::Citizen;
        case ArrivalType::Migrant: return LegalStatus::Resident;
        case ArrivalType::Refugee: return LegalStatus::ProtectedRefugee;
        case ArrivalType::Contractor:
        case ArrivalType::MercenaryHunter: return LegalStatus::GuestWorker;
        case ArrivalType::PerformerScholar:
        case ArrivalType::TraderCrew:
        case ArrivalType::ImperialOfficial:
        case ArrivalType::Infiltrator:
        case ArrivalType::PilgrimGuild: return LegalStatus::Visitor;
    }
    return LegalStatus::Visitor;
}

ArrivalResult CivicSimulation::admitGroup(CivicStableId settlementId,
                                          const MigrationGroup& group,
                                          const SettlementMigrationInputs& inputs,
                                          const MigrationPolicy& policy,
                                          double hour) {
    ArrivalResult out;
    const MigrationDecision decision = evaluateMigration(group, inputs, policy);
    if (settlementId == 0) {
        out.reason = "invalid settlement stable id";
        return out;
    }
    if (group.cultureId != 0 && !cultures_.contains(group.cultureId)) {
        out.reason = "unknown culture stable id";
        return out;
    }
    if (!decision.accepted) {
        out.reason = decision.summary;
        return out;
    }

    const CivicStableId householdId = ensureUniqueDerived(mix64(seed_ ^ settlementId ^ group.groupStableId ^ 0x484F555345484F4CULL));
    out.citizenStableIds.reserve(group.members.size());
    for (std::size_t i = 0; i < group.members.size(); ++i) {
        const CivicStableId id = deriveMemberStableId(settlementId, group.groupStableId, i);
        CitizenRecord record;
        record.stableId = id;
        record.settlementId = settlementId;
        record.originSiteId = group.originSiteId;
        record.householdId = householdId;
        record.cultureId = group.cultureId;
        record.name = group.members[i].name;
        record.legalStatus = initialStatusFor(group.type);
        record.present = true;
        record.historySignificant = group.members[i].historySignificant || group.type == ArrivalType::Founder;
        record.arrivalHour = hour;
        record.visitorPermitUntilHour = record.legalStatus == LegalStatus::Visitor ? hour + policy.visitorPermitHours : 0.0;
        record.needs = group.members[i].initialNeeds;
        for (float& value : record.needs) value = clamp01(value);
        record.skills = group.members[i].skills;
        citizens_.emplace(id, std::move(record));
        nextStableId_ = std::max(nextStableId_, id + 1);
        out.citizenStableIds.push_back(id);
    }

    for (const auto& relation : group.relationships) {
        if (relation.sourceMember >= out.citizenStableIds.size() || relation.targetMember >= out.citizenStableIds.size() ||
            relation.sourceMember == relation.targetMember) continue;
        auto* source = citizen(out.citizenStableIds[relation.sourceMember]);
        if (!source) continue;
        source->relationships.push_back({out.citizenStableIds[relation.targetMember], relation.type,
                                         relation.affinity, relation.trust, relation.respect, relation.fear,
                                         relation.grievance, 1.0f});
    }

    appendHistory(CivicHistoryEventType::PopulationArrived, settlementId, group.groupStableId,
                  group.originSiteId, hour,
                  std::string(arrivalTypeName(group.type)) + ": " + group.motive + " members=" + std::to_string(out.citizenStableIds.size()));
    out.admitted = true;
    out.reason = decision.summary;
    return out;
}

bool CivicSimulation::transitionLegalStatus(CivicStableId citizenId, LegalStatus next,
                                            const MigrationPolicy& policy, double hour,
                                            std::string* reason) {
    auto* c = citizen(citizenId);
    if (!c) {
        if (reason) *reason = "citizen not found";
        return false;
    }
    if (c->legalStatus == LegalStatus::Exiled && next != LegalStatus::Exiled) {
        if (reason) *reason = "exiled identity requires an explicit legal readmission flow";
        return false;
    }
    if (next == LegalStatus::Citizen) {
        if (!policy.allowCitizenship) {
            if (reason) *reason = "citizenship disabled by policy";
            return false;
        }
        if (c->legalStatus != LegalStatus::Resident && c->legalStatus != LegalStatus::ProtectedRefugee) {
            if (reason) *reason = "citizenship requires resident or protected-refugee status";
            return false;
        }
        c->residencyHours = std::max(c->residencyHours, hour - c->arrivalHour);
        if (c->residencyHours < policy.minimumResidentHoursForCitizenship) {
            if (reason) *reason = "minimum residency time not met";
            return false;
        }
    }
    if (next == LegalStatus::Resident && c->legalStatus == LegalStatus::Visitor && !c->present) {
        if (reason) *reason = "departed visitor cannot become resident";
        return false;
    }
    const LegalStatus previous = c->legalStatus;
    c->legalStatus = next;
    if (next != LegalStatus::Visitor) c->visitorPermitUntilHour = 0.0;
    appendHistory(CivicHistoryEventType::LegalStatusChanged, c->settlementId, citizenId, 0, hour,
                  std::string(legalStatusName(previous)) + " -> " + legalStatusName(next));
    if (reason) reason->clear();
    return true;
}

bool CivicSimulation::departCitizen(CivicStableId citizenId, double hour, std::string reason) {
    auto* c = citizen(citizenId);
    if (!c || !c->present) return false;
    c->residencyHours = std::max(c->residencyHours, hour - c->arrivalHour);
    c->present = false;
    for (const CivicStableId institutionId : c->institutionMemberships) {
        if (auto* record = institution(institutionId)) {
            record->memberIds.erase(std::remove(record->memberIds.begin(), record->memberIds.end(), citizenId), record->memberIds.end());
        }
    }
    c->institutionMemberships.clear();
    appendHistory(CivicHistoryEventType::PopulationDeparted, c->settlementId, citizenId, c->originSiteId, hour, std::move(reason));
    return true;
}

bool CivicSimulation::setCitizenNeed(CivicStableId citizenId, CivicNeed need, float value) {
    auto* c = citizen(citizenId);
    const auto index = static_cast<std::size_t>(need);
    if (!c || index >= c->needs.size()) return false;
    c->needs[index] = clamp01(value);
    return true;
}

bool CivicSimulation::addRelationship(CivicStableId sourceId, RelationshipEdge edge) {
    auto* source = citizen(sourceId);
    if (!source || !citizen(edge.targetStableId) || sourceId == edge.targetStableId) return false;
    source->relationships.push_back(std::move(edge));
    return true;
}

CivicStableId CivicSimulation::createCulturalWork(CulturalWorkRecord work, CivicStableId preferredStableId) {
    if (work.authorStableId != 0 && !citizens_.contains(work.authorStableId)) return 0;
    if (work.institutionStableId != 0 && !institutions_.contains(work.institutionStableId)) return 0;
    const CivicStableId id = allocateStableId(preferredStableId != 0 ? preferredStableId : work.stableId);
    if (id == 0) return 0;
    work.stableId = id;
    work.quality = clamp01(work.quality);
    works_.emplace(id, work);
    CivicStableId settlement = 0;
    if (const auto* author = citizen(work.authorStableId)) settlement = author->settlementId;
    else if (const auto* inst = institution(work.institutionStableId)) settlement = inst->settlementId;
    appendHistory(CivicHistoryEventType::CulturalWorkCreated, settlement, id, work.authorStableId, work.createdHour,
                  work.form + ": " + work.title);
    return id;
}

CivicStableId CivicSimulation::recordCulturalTransmission(CivicStableId workStableId,
                                                          CulturalTransmissionType type,
                                                          CivicStableId actorStableId,
                                                          CivicStableId institutionStableId,
                                                          CivicStableId settlementId,
                                                          double hour,
                                                          std::string detail) {
    auto workIt = works_.find(workStableId);
    if (workIt == works_.end()) return 0;
    if (actorStableId != 0 && !citizens_.contains(actorStableId)) return 0;
    if (institutionStableId != 0 && !institutions_.contains(institutionStableId)) return 0;
    if (settlementId == 0) {
        if (institutionStableId != 0) settlementId = institutions_.at(institutionStableId).settlementId;
        else if (actorStableId != 0) settlementId = citizens_.at(actorStableId).settlementId;
    }
    if (type == CulturalTransmissionType::Archived) workIt->second.archived = true;
    const std::string summary = std::string(culturalTransmissionTypeName(type)) +
        (detail.empty() ? std::string{} : std::string(": ") + detail);
    const CivicStableId eventId = appendHistory(CivicHistoryEventType::CulturalWorkTransmission,
                                                settlementId, workStableId, actorStableId,
                                                hour, summary);
    if (eventId == 0) return 0;
    transmissions_.push_back({eventId, workStableId, type, actorStableId,
                              institutionStableId, settlementId, hour, std::move(detail)});
    return eventId;
}

std::vector<CulturalTransmissionRecord> CivicSimulation::transmissionsForWork(CivicStableId workStableId) const {
    std::vector<CulturalTransmissionRecord> out;
    for (const auto& record : transmissions_) if (record.workStableId == workStableId) out.push_back(record);
    return out;
}

DepartureDecision CivicSimulation::evaluateDeparture(CivicStableId citizenId, const DepartureInputs& inputs) const {
    DepartureDecision out;
    const auto* c = citizen(citizenId);
    if (!c) {
        out.summary = "citizen not found";
        return out;
    }
    out.citizenFound = true;
    auto factor = [&](std::string label, float value, float weight) {
        const float clamped = clamp01(value);
        const float contribution = clamped * weight;
        out.factors.push_back({std::move(label), clamped, weight, contribution});
        out.pressure += contribution;
    };
    // Pressure is deliberately expressed as reasons to leave. Positive site
    // conditions are inverted so the Why view can expose each contribution.
    factor("unsafe", 1.0f - inputs.safety, 0.18f);
    factor("work-dissatisfaction", 1.0f - inputs.workSatisfaction, 0.13f);
    factor("housing-pressure", 1.0f - inputs.housing, 0.10f);
    factor("food-insecurity", 1.0f - inputs.foodSecurity, 0.10f);
    factor("institution-gap", 1.0f - inputs.institutionAccess, 0.07f);
    factor("weak-relationship-roots", 1.0f - inputs.relationshipRoots, 0.13f);
    factor("cultural-mismatch", 1.0f - inputs.culturalFit, 0.09f);
    factor("faction-pressure", inputs.factionPressure, 0.08f);
    factor("active-threat", inputs.activeThreat, 0.14f);
    factor("outside-opportunity", inputs.personalOpportunity, 0.10f);
    out.pressure = std::clamp(out.pressure, 0.0f, 1.0f);
    if (!c->present) {
        out.wouldLeave = false;
        out.summary = "citizen already departed";
    } else if (c->legalStatus == LegalStatus::Exiled) {
        out.wouldLeave = true;
        out.threshold = 0.0f;
        out.summary = "exiled citizen is required to depart while present";
    } else {
        out.wouldLeave = out.pressure >= out.threshold;
        out.summary = out.wouldLeave
            ? "departure pressure exceeds settlement attachment"
            : "settlement attachment currently outweighs departure pressure";
    }
    return out;
}

bool CivicSimulation::scheduleOpen(const CivicSchedule& schedule, double hour) const {
    const double dayHour = std::fmod(std::fmod(hour, 24.0) + 24.0, 24.0);
    const double open = std::clamp<double>(schedule.openHour, 0.0, 24.0);
    const double close = std::clamp<double>(schedule.closeHour, 0.0, 24.0);
    if (std::abs(open - close) < 1e-6 || (open <= 0.0 && close >= 24.0)) return true;
    if (close > open) return dayHour >= open && dayHour < close;
    return dayHour >= open || dayHour < close;
}

float* CivicSimulation::skillRank(CitizenRecord& c, const std::string& skill) {
    auto it = std::find_if(c.skills.begin(), c.skills.end(), [&](const SkillRank& s) { return s.skill == skill; });
    if (it == c.skills.end()) {
        c.skills.push_back({skill, 0.0f});
        return &c.skills.back().rank;
    }
    return &it->rank;
}

const float* CivicSimulation::skillRank(const CitizenRecord& c, const std::string& skill) const {
    const auto it = std::find_if(c.skills.begin(), c.skills.end(), [&](const SkillRank& s) { return s.skill == skill; });
    return it == c.skills.end() ? nullptr : &it->rank;
}

InstitutionTickTelemetry CivicSimulation::updateInstitutions(double startHour, double elapsedHours) {
    InstitutionTickTelemetry telemetry;
    if (elapsedHours <= 0.0) return telemetry;
    const double endHour = startHour + elapsedHours;

    for (auto& [institutionId, record] : institutions_) {
        (void)institutionId;
        ++telemetry.institutionsConsidered;
        if (!record.active) continue;
        if (record.staffAvailable <= 0) {
            ++telemetry.blockedNoStaff;
            continue;
        }
        if (record.schedule.activityPeriodHours <= 0.0f) record.schedule.activityPeriodHours = 6.0f;
        if (record.schedule.nextActivityHour < startHour) {
            const double periods = std::floor((startHour - record.schedule.nextActivityHour) / record.schedule.activityPeriodHours);
            record.schedule.nextActivityHour += std::max(0.0, periods) * record.schedule.activityPeriodHours;
            while (record.schedule.nextActivityHour < startHour) record.schedule.nextActivityHour += record.schedule.activityPeriodHours;
        }

        int catchup = 0;
        while (record.schedule.nextActivityHour <= endHour && catchup < 64) {
            const double activityHour = record.schedule.nextActivityHour;
            record.schedule.nextActivityHour += record.schedule.activityPeriodHours;
            ++catchup;
            if (!scheduleOpen(record.schedule, activityHour)) continue;

            std::vector<CivicStableId> participants;
            participants.reserve(record.memberIds.size());
            for (const CivicStableId citizenId : record.memberIds) {
                const auto* c = citizen(citizenId);
                if (c && c->present && c->settlementId == record.settlementId && c->legalStatus != LegalStatus::Exiled && c->legalStatus != LegalStatus::Restricted) {
                    participants.push_back(citizenId);
                }
            }
            // Visitors can use public services without formal institution membership.
            for (const auto& [citizenId, c] : citizens_) {
                if (!c.present || c.settlementId != record.settlementId || c.legalStatus != LegalStatus::Visitor) continue;
                if (contains(participants, citizenId)) continue;
                participants.push_back(citizenId);
            }
            sortUnique(participants);
            if (participants.empty()) continue;

            bool ranAnyService = false;
            for (const InstitutionService& service : record.services) {
                if (!service.requiredSupply.empty() && service.supplyCost > 0) {
                    const auto supplyIt = record.supplies.find(service.requiredSupply);
                    if (supplyIt == record.supplies.end() || supplyIt->second < service.supplyCost) {
                        ++telemetry.blockedSupplies;
                        continue;
                    }
                }
                int servicedForThisService = 0;
                for (const CivicStableId citizenId : participants) {
                    auto* c = citizen(citizenId);
                    if (!c) continue;
                    const auto index = static_cast<std::size_t>(service.need);
                    if (index >= c->needs.size() || c->needs[index] <= 0.0f) continue;
                    c->needs[index] = std::max(0.0f, c->needs[index] - service.satisfaction * (0.5f + record.quality));
                    ++servicedForThisService;
                    ++telemetry.needsSatisfied;
                }
                if (servicedForThisService > 0 && !service.requiredSupply.empty() && service.supplyCost > 0) {
                    auto& supply = record.supplies[service.requiredSupply];
                    supply = std::max(0, supply - service.supplyCost);
                }
                if (servicedForThisService > 0) ranAnyService = true;
            }

            if (record.education) {
                const EducationProgram& program = *record.education;
                CivicStableId mentorId = 0;
                float mentorRank = -1.0f;
                for (const CivicStableId citizenId : participants) {
                    const auto* c = citizen(citizenId);
                    if (!c) continue;
                    const float* rank = skillRank(*c, program.skill);
                    if (rank && *rank >= program.minimumMentorRank && (*rank > mentorRank || (*rank == mentorRank && citizenId < mentorId))) {
                        mentorRank = *rank;
                        mentorId = citizenId;
                    }
                }
                if (mentorId != 0) {
                    int learners = 0;
                    for (const CivicStableId citizenId : participants) {
                        if (citizenId == mentorId) continue;
                        auto* learner = citizen(citizenId);
                        if (!learner) continue;
                        float* rank = skillRank(*learner, program.skill);
                        if (*rank >= mentorRank) continue;
                        *rank += program.learnerGain * (0.5f + record.quality);
                        ++learners;
                    }
                    if (learners > 0) {
                        ++telemetry.educationSessions;
                        ranAnyService = true;
                    }
                }
            }

            if (ranAnyService) {
                ++telemetry.activitiesRun;
                telemetry.citizensServed += static_cast<int>(participants.size());
                telemetry.visitorsServed += static_cast<int>(std::count_if(participants.begin(), participants.end(), [&](CivicStableId id) {
                    const auto* c = citizen(id);
                    return c && c->legalStatus == LegalStatus::Visitor;
                }));
                appendHistory(CivicHistoryEventType::InstitutionActivity, record.settlementId, record.stableId, 0,
                              activityHour, std::string(institutionTypeName(record.type)) + " activity");
            }
        }
    }

    // Residency accumulation and visitor permit expiration are intentionally
    // independent of detailed actor AI, so behavioral LOD cannot erase legal
    // continuity.
    for (auto& [id, c] : citizens_) {
        (void)id;
        if (!c.present) continue;
        c.residencyHours = std::max(c.residencyHours, endHour - c.arrivalHour);
        if (c.legalStatus == LegalStatus::Visitor && c.visitorPermitUntilHour > 0.0 && endHour > c.visitorPermitUntilHour) {
            // Expiration is explicit state rather than silent deletion. Security/
            // justice agents can later decide how to respond.
            c.legalStatus = LegalStatus::Restricted;
            appendHistory(CivicHistoryEventType::LegalStatusChanged, c.settlementId, c.stableId, 0, endHour,
                          "visitor permit expired -> restricted");
        }
    }
    return telemetry;
}

const CitizenRecord* CivicSimulation::citizen(CivicStableId id) const {
    const auto it = citizens_.find(id);
    return it == citizens_.end() ? nullptr : &it->second;
}

CitizenRecord* CivicSimulation::citizen(CivicStableId id) {
    const auto it = citizens_.find(id);
    return it == citizens_.end() ? nullptr : &it->second;
}

const InstitutionRecord* CivicSimulation::institution(CivicStableId id) const {
    const auto it = institutions_.find(id);
    return it == institutions_.end() ? nullptr : &it->second;
}

InstitutionRecord* CivicSimulation::institution(CivicStableId id) {
    const auto it = institutions_.find(id);
    return it == institutions_.end() ? nullptr : &it->second;
}

const CultureRecord* CivicSimulation::culture(CivicStableId id) const {
    const auto it = cultures_.find(id);
    return it == cultures_.end() ? nullptr : &it->second;
}

const CulturalWorkRecord* CivicSimulation::culturalWork(CivicStableId id) const {
    const auto it = works_.find(id);
    return it == works_.end() ? nullptr : &it->second;
}

std::vector<CivicStableId> CivicSimulation::citizenIds() const {
    std::vector<CivicStableId> out;
    out.reserve(citizens_.size());
    for (const auto& [id, c] : citizens_) {
        (void)c;
        out.push_back(id);
    }
    return out;
}

std::vector<CivicStableId> CivicSimulation::institutionIds() const {
    std::vector<CivicStableId> out;
    out.reserve(institutions_.size());
    for (const auto& [id, record] : institutions_) {
        (void)record;
        out.push_back(id);
    }
    return out;
}

CivicPopulationDiagnostics CivicSimulation::populationDiagnostics(CivicStableId settlementId) const {
    CivicPopulationDiagnostics out;
    for (const auto& [id, c] : citizens_) {
        (void)id;
        if (c.settlementId != settlementId) continue;
        if (c.present) ++out.present;
        switch (c.legalStatus) {
            case LegalStatus::Visitor: ++out.visitors; break;
            case LegalStatus::GuestWorker: ++out.guestWorkers; break;
            case LegalStatus::Resident: ++out.residents; break;
            case LegalStatus::Citizen: ++out.citizens; break;
            case LegalStatus::ProtectedRefugee: ++out.refugees; break;
            case LegalStatus::Restricted: ++out.restricted; break;
            case LegalStatus::Exiled: ++out.exiled; break;
        }
    }
    return out;
}

InstitutionDiagnostics CivicSimulation::institutionDiagnostics(CivicStableId institutionId, double hour) const {
    InstitutionDiagnostics out;
    const auto* record = institution(institutionId);
    if (!record) return out;
    out.found = true;
    out.active = record->active;
    out.openNow = record->active && scheduleOpen(record->schedule, hour);
    out.staffAvailable = record->staffAvailable;
    if (!record->active) out.blockers.push_back("institution dissolved/inactive");
    if (!out.openNow) out.blockers.push_back("outside activity schedule");
    if (record->staffAvailable <= 0) out.blockers.push_back("no staff available");
    for (const InstitutionService& service : record->services) {
        out.offeredServices.push_back(civicNeedName(service.need));
        if (!service.requiredSupply.empty() && service.supplyCost > 0) {
            const auto it = record->supplies.find(service.requiredSupply);
            if (it == record->supplies.end() || it->second < service.supplyCost) {
                out.blockers.push_back("missing supply: " + service.requiredSupply);
            }
        }
    }
    for (const CivicStableId citizenId : record->memberIds) {
        const auto* c = citizen(citizenId);
        if (c && c->present && c->settlementId == record->settlementId) ++out.membersPresent;
    }
    for (const auto& [id, c] : citizens_) {
        (void)id;
        if (c.present && c.settlementId == record->settlementId && c.legalStatus == LegalStatus::Visitor) ++out.visitorsEligible;
    }
    sortUnique(out.blockers);
    return out;
}


std::vector<CivicServiceDemandEntry> CivicSimulation::serviceDemandDiagnostics(CivicStableId settlementId, double hour) const {
    std::vector<CivicServiceDemandEntry> out(static_cast<std::size_t>(CivicNeed::Count));
    for (std::size_t i = 0; i < out.size(); ++i) out[i].need = static_cast<CivicNeed>(i);
    for (const auto& [id, c] : citizens_) {
        (void)id;
        if (!c.present || c.settlementId != settlementId || c.legalStatus == LegalStatus::Exiled) continue;
        for (std::size_t i = 0; i < out.size(); ++i) {
            out[i].totalUnmet += clamp01(c.needs[i]);
            if (c.needs[i] > 0.001f) ++out[i].presentPeople;
        }
    }
    for (const auto& [id, record] : institutions_) {
        (void)id;
        if (!record.active || record.settlementId != settlementId) continue;
        for (const auto& service : record.services) {
            const auto index = static_cast<std::size_t>(service.need);
            if (index >= out.size()) continue;
            bool blocked = record.staffAvailable <= 0 || !scheduleOpen(record.schedule, hour);
            if (!service.requiredSupply.empty() && service.supplyCost > 0) {
                const auto it = record.supplies.find(service.requiredSupply);
                blocked = blocked || it == record.supplies.end() || it->second < service.supplyCost;
            }
            if (blocked) ++out[index].blockedInstitutions;
            else ++out[index].activeInstitutions;
        }
    }
    return out;
}

std::string CivicSimulation::serialize() const {
    std::ostringstream out;
    out << std::setprecision(17);
    out << "ELYSIUM_CIVIC 2\n";
    out << "META " << seed_ << ' ' << nextStableId_ << ' ' << static_cast<int>(detail_) << '\n';
    out << "CULTURES " << cultures_.size() << '\n';
    for (const auto& [id, c] : cultures_) {
        out << "CULT " << id << ' ' << std::quoted(c.name) << ' ' << c.values.size() << ' ' << c.legalNorms.size() << ' '
            << c.architectureTags.size() << ' ' << c.cuisineTags.size() << ' ' << c.performanceTraditions.size() << ' '
            << c.artMotifs.size() << ' ' << c.rituals.size() << ' ' << c.languageTags.size() << '\n';
        for (const auto& value : c.values) out << "VAL " << value.weight << ' ' << std::quoted(value.tag) << '\n';
        for (const auto& value : c.legalNorms) out << "LEGAL " << std::quoted(value) << '\n';
        for (const auto& value : c.architectureTags) out << "ARCH " << std::quoted(value) << '\n';
        for (const auto& value : c.cuisineTags) out << "CUIS " << std::quoted(value) << '\n';
        for (const auto& value : c.performanceTraditions) out << "PERF " << std::quoted(value) << '\n';
        for (const auto& value : c.artMotifs) out << "MOTIF " << std::quoted(value) << '\n';
        for (const auto& value : c.rituals) out << "RITUAL " << std::quoted(value) << '\n';
        for (const auto& value : c.languageTags) out << "LANG " << std::quoted(value) << '\n';
    }

    out << "CITIZENS " << citizens_.size() << '\n';
    for (const auto& [id, c] : citizens_) {
        out << "CIT " << id << ' ' << c.settlementId << ' ' << c.originSiteId << ' ' << c.householdId << ' ' << c.cultureId << ' '
            << static_cast<int>(c.legalStatus) << ' ' << (c.present ? 1 : 0) << ' ' << (c.historySignificant ? 1 : 0) << ' '
            << c.arrivalHour << ' ' << c.residencyHours << ' ' << c.visitorPermitUntilHour << ' ' << std::quoted(c.name) << ' '
            << c.skills.size() << ' ' << c.relationships.size() << ' ' << c.institutionMemberships.size() << '\n';
        out << "NEEDS";
        for (const float need : c.needs) out << ' ' << need;
        out << '\n';
        for (const auto& skill : c.skills) out << "SKILL " << skill.rank << ' ' << std::quoted(skill.skill) << '\n';
        for (const auto& relation : c.relationships) {
            out << "REL " << relation.targetStableId << ' ' << static_cast<int>(relation.type) << ' ' << relation.affinity << ' '
                << relation.trust << ' ' << relation.respect << ' ' << relation.fear << ' ' << relation.grievance << ' '
                << relation.familiarity << '\n';
        }
        for (const CivicStableId institutionId : c.institutionMemberships) out << "MEM " << institutionId << '\n';
    }

    out << "INSTITUTIONS " << institutions_.size() << '\n';
    for (const auto& [id, r] : institutions_) {
        out << "INST " << id << ' ' << r.settlementId << ' ' << r.cultureId << ' ' << r.ownerId << ' ' << r.leaderId << ' '
            << static_cast<int>(r.type) << ' ' << (r.active ? 1 : 0) << ' ' << r.quality << ' ' << r.staffAvailable << ' '
            << r.schedule.openHour << ' ' << r.schedule.closeHour << ' ' << r.schedule.activityPeriodHours << ' '
            << r.schedule.nextActivityHour << ' ' << std::quoted(r.name) << ' ' << r.roomRefs.size() << ' ' << r.memberIds.size() << ' '
            << r.supplies.size() << ' ' << r.services.size() << ' ' << (r.education ? 1 : 0) << '\n';
        for (const CivicStableId room : r.roomRefs) out << "ROOM " << room << '\n';
        for (const CivicStableId member : r.memberIds) out << "IMEM " << member << '\n';
        for (const auto& [tag, count] : r.supplies) out << "SUP " << count << ' ' << std::quoted(tag) << '\n';
        for (const auto& service : r.services) out << "SERV " << static_cast<int>(service.need) << ' ' << service.satisfaction << ' '
                                                      << service.supplyCost << ' ' << std::quoted(service.requiredSupply) << '\n';
        if (r.education) out << "EDU " << r.education->minimumMentorRank << ' ' << r.education->learnerGain << ' '
                             << std::quoted(r.education->skill) << '\n';
    }

    out << "WORKS " << works_.size() << '\n';
    for (const auto& [id, work] : works_) {
        out << "WORK " << id << ' ' << work.authorStableId << ' ' << work.institutionStableId << ' ' << work.cultureId << ' '
            << work.quality << ' ' << work.createdHour << ' ' << (work.archived ? 1 : 0) << ' ' << std::quoted(work.title) << ' '
            << std::quoted(work.form) << ' ' << std::quoted(work.subject) << ' ' << std::quoted(work.style) << '\n';
    }

    out << "TRANSMISSIONS " << transmissions_.size() << '\n';
    for (const auto& tx : transmissions_) {
        out << "TX " << tx.stableId << ' ' << tx.workStableId << ' ' << static_cast<int>(tx.type) << ' '
            << tx.actorStableId << ' ' << tx.institutionStableId << ' ' << tx.settlementId << ' ' << tx.hour << ' '
            << std::quoted(tx.detail) << '\n';
    }

    out << "HISTORY " << history_.size() << '\n';
    for (const auto& event : history_) {
        out << "EV " << event.stableId << ' ' << static_cast<int>(event.type) << ' ' << event.settlementId << ' '
            << event.subjectStableId << ' ' << event.relatedStableId << ' ' << event.hour << ' ' << std::quoted(event.detail) << '\n';
    }
    out << "END\n";
    return out.str();
}

bool CivicSimulation::deserialize(const std::string& text, std::string* error) {
    CivicSimulation candidate(seed_);
    std::istringstream in(text);
    std::string token;
    int version = 0;
    if (!(in >> token >> version) || token != "ELYSIUM_CIVIC" || (version != 1 && version != 2)) {
        if (error) *error = "invalid/unsupported civic save header";
        return false;
    }
    int detail = 0;
    if (!(in >> token >> candidate.seed_ >> candidate.nextStableId_ >> detail) || token != "META" || !validDetail(detail)) {
        if (error) *error = "invalid civic META record";
        return false;
    }
    candidate.detail_ = static_cast<CivicSimulationDetail>(detail);

    std::size_t count = 0;
    if (!(in >> token >> count) || token != "CULTURES") { if (error) *error = "missing CULTURES section"; return false; }
    for (std::size_t i = 0; i < count; ++i) {
        CultureRecord c;
        std::size_t values = 0, legal = 0, arch = 0, cuis = 0, perf = 0, motifs = 0, rituals = 0, langs = 0;
        if (!(in >> token >> c.stableId >> std::quoted(c.name) >> values >> legal >> arch >> cuis >> perf >> motifs >> rituals >> langs) || token != "CULT" || c.stableId == 0) {
            if (error) *error = "invalid CULT record";
            return false;
        }
        c.values.reserve(values);
        for (std::size_t n = 0; n < values; ++n) {
            WeightedCultureTag v;
            if (!(in >> token >> v.weight >> std::quoted(v.tag)) || token != "VAL") { if (error) *error = "invalid VAL record"; return false; }
            c.values.push_back(std::move(v));
        }
        if (!readQuotedVector(in, c.legalNorms, legal, "LEGAL", error) ||
            !readQuotedVector(in, c.architectureTags, arch, "ARCH", error) ||
            !readQuotedVector(in, c.cuisineTags, cuis, "CUIS", error) ||
            !readQuotedVector(in, c.performanceTraditions, perf, "PERF", error) ||
            !readQuotedVector(in, c.artMotifs, motifs, "MOTIF", error) ||
            !readQuotedVector(in, c.rituals, rituals, "RITUAL", error) ||
            !readQuotedVector(in, c.languageTags, langs, "LANG", error)) return false;
        if (candidate.cultures_.contains(c.stableId)) { if (error) *error = "duplicate culture stable id"; return false; }
        candidate.cultures_.emplace(c.stableId, std::move(c));
    }

    if (!(in >> token >> count) || token != "CITIZENS") { if (error) *error = "missing CITIZENS section"; return false; }
    for (std::size_t i = 0; i < count; ++i) {
        CitizenRecord c;
        int status = 0, present = 0, significant = 0;
        std::size_t skills = 0, relations = 0, memberships = 0;
        if (!(in >> token >> c.stableId >> c.settlementId >> c.originSiteId >> c.householdId >> c.cultureId >> status >> present >> significant
                 >> c.arrivalHour >> c.residencyHours >> c.visitorPermitUntilHour >> std::quoted(c.name) >> skills >> relations >> memberships) ||
            token != "CIT" || c.stableId == 0 || !validLegalStatus(status)) {
            if (error) *error = "invalid CIT record";
            return false;
        }
        c.legalStatus = static_cast<LegalStatus>(status);
        c.present = present != 0;
        c.historySignificant = significant != 0;
        if (!(in >> token) || token != "NEEDS") { if (error) *error = "missing NEEDS record"; return false; }
        for (float& need : c.needs) if (!(in >> need)) { if (error) *error = "invalid NEEDS values"; return false; }
        c.skills.reserve(skills);
        for (std::size_t n = 0; n < skills; ++n) {
            SkillRank skill;
            if (!(in >> token >> skill.rank >> std::quoted(skill.skill)) || token != "SKILL") { if (error) *error = "invalid SKILL record"; return false; }
            c.skills.push_back(std::move(skill));
        }
        c.relationships.reserve(relations);
        for (std::size_t n = 0; n < relations; ++n) {
            RelationshipEdge rel;
            int type = 0;
            if (!(in >> token >> rel.targetStableId >> type >> rel.affinity >> rel.trust >> rel.respect >> rel.fear >> rel.grievance >> rel.familiarity) ||
                token != "REL" || !validRelationshipType(type)) { if (error) *error = "invalid REL record"; return false; }
            rel.type = static_cast<RelationshipType>(type);
            c.relationships.push_back(rel);
        }
        c.institutionMemberships.reserve(memberships);
        for (std::size_t n = 0; n < memberships; ++n) {
            CivicStableId institutionId = 0;
            if (!(in >> token >> institutionId) || token != "MEM") { if (error) *error = "invalid MEM record"; return false; }
            c.institutionMemberships.push_back(institutionId);
        }
        if (candidate.citizens_.contains(c.stableId)) { if (error) *error = "duplicate citizen stable id"; return false; }
        candidate.citizens_.emplace(c.stableId, std::move(c));
    }

    if (!(in >> token >> count) || token != "INSTITUTIONS") { if (error) *error = "missing INSTITUTIONS section"; return false; }
    for (std::size_t i = 0; i < count; ++i) {
        InstitutionRecord r;
        int type = 0, active = 0, hasEducation = 0;
        std::size_t rooms = 0, members = 0, supplies = 0, services = 0;
        if (!(in >> token >> r.stableId >> r.settlementId >> r.cultureId >> r.ownerId >> r.leaderId >> type >> active >> r.quality >> r.staffAvailable
                 >> r.schedule.openHour >> r.schedule.closeHour >> r.schedule.activityPeriodHours >> r.schedule.nextActivityHour >> std::quoted(r.name)
                 >> rooms >> members >> supplies >> services >> hasEducation) || token != "INST" || r.stableId == 0 || !validInstitutionType(type)) {
            if (error) *error = "invalid INST record";
            return false;
        }
        r.type = static_cast<InstitutionType>(type);
        r.active = active != 0;
        for (std::size_t n = 0; n < rooms; ++n) { CivicStableId id = 0; if (!(in >> token >> id) || token != "ROOM") { if (error) *error = "invalid ROOM record"; return false; } r.roomRefs.push_back(id); }
        for (std::size_t n = 0; n < members; ++n) { CivicStableId id = 0; if (!(in >> token >> id) || token != "IMEM") { if (error) *error = "invalid IMEM record"; return false; } r.memberIds.push_back(id); }
        for (std::size_t n = 0; n < supplies; ++n) { int amount = 0; std::string tag; if (!(in >> token >> amount >> std::quoted(tag)) || token != "SUP" || amount < 0) { if (error) *error = "invalid SUP record"; return false; } r.supplies[tag] = amount; }
        for (std::size_t n = 0; n < services; ++n) {
            InstitutionService service;
            int need = 0;
            if (!(in >> token >> need >> service.satisfaction >> service.supplyCost >> std::quoted(service.requiredSupply)) || token != "SERV" || !validNeed(need)) {
                if (error) *error = "invalid SERV record";
                return false;
            }
            service.need = static_cast<CivicNeed>(need);
            r.services.push_back(std::move(service));
        }
        if (hasEducation) {
            EducationProgram program;
            if (!(in >> token >> program.minimumMentorRank >> program.learnerGain >> std::quoted(program.skill)) || token != "EDU") {
                if (error) *error = "invalid EDU record";
                return false;
            }
            r.education = std::move(program);
        }
        if (candidate.institutions_.contains(r.stableId)) { if (error) *error = "duplicate institution stable id"; return false; }
        candidate.institutions_.emplace(r.stableId, std::move(r));
    }

    if (!(in >> token >> count) || token != "WORKS") { if (error) *error = "missing WORKS section"; return false; }
    for (std::size_t i = 0; i < count; ++i) {
        CulturalWorkRecord work;
        int archived = 0;
        if (!(in >> token >> work.stableId >> work.authorStableId >> work.institutionStableId >> work.cultureId >> work.quality >> work.createdHour >> archived
                 >> std::quoted(work.title) >> std::quoted(work.form) >> std::quoted(work.subject) >> std::quoted(work.style)) || token != "WORK" || work.stableId == 0) {
            if (error) *error = "invalid WORK record";
            return false;
        }
        work.archived = archived != 0;
        candidate.works_.emplace(work.stableId, std::move(work));
    }

    if (version >= 2) {
        if (!(in >> token >> count) || token != "TRANSMISSIONS") { if (error) *error = "missing TRANSMISSIONS section"; return false; }
        for (std::size_t i = 0; i < count; ++i) {
            CulturalTransmissionRecord tx;
            int type = 0;
            if (!(in >> token >> tx.stableId >> tx.workStableId >> type >> tx.actorStableId >> tx.institutionStableId
                     >> tx.settlementId >> tx.hour >> std::quoted(tx.detail)) || token != "TX" || tx.stableId == 0 || !validTransmissionType(type)) {
                if (error) *error = "invalid TX record";
                return false;
            }
            tx.type = static_cast<CulturalTransmissionType>(type);
            candidate.transmissions_.push_back(std::move(tx));
        }
    }

    if (!(in >> token >> count) || token != "HISTORY") { if (error) *error = "missing HISTORY section"; return false; }
    for (std::size_t i = 0; i < count; ++i) {
        CivicHistoryEvent event;
        int type = 0;
        if (!(in >> token >> event.stableId >> type >> event.settlementId >> event.subjectStableId >> event.relatedStableId >> event.hour >> std::quoted(event.detail)) ||
            token != "EV" || event.stableId == 0 || !validHistoryType(type)) {
            if (error) *error = "invalid EV record";
            return false;
        }
        event.type = static_cast<CivicHistoryEventType>(type);
        candidate.history_.push_back(std::move(event));
    }
    if (!(in >> token) || token != "END") { if (error) *error = "missing END marker"; return false; }

    // Consistency validation required before authority resumes.
    for (const auto& [id, c] : candidate.citizens_) {
        (void)id;
        if (c.cultureId != 0 && !candidate.cultures_.contains(c.cultureId)) { if (error) *error = "citizen references missing culture"; return false; }
        for (const auto& rel : c.relationships) if (!candidate.citizens_.contains(rel.targetStableId)) { if (error) *error = "relationship references missing citizen"; return false; }
        for (const CivicStableId institutionId : c.institutionMemberships) if (!candidate.institutions_.contains(institutionId)) { if (error) *error = "citizen references missing institution"; return false; }
    }
    for (const auto& [id, r] : candidate.institutions_) {
        (void)id;
        if (r.cultureId != 0 && !candidate.cultures_.contains(r.cultureId)) { if (error) *error = "institution references missing culture"; return false; }
        for (const CivicStableId memberId : r.memberIds) {
            const auto* c = candidate.citizen(memberId);
            if (!c || !contains(c->institutionMemberships, r.stableId)) { if (error) *error = "institution/member reverse link mismatch"; return false; }
        }
    }
    for (const auto& [id, work] : candidate.works_) {
        (void)id;
        if (work.authorStableId != 0 && !candidate.citizens_.contains(work.authorStableId)) { if (error) *error = "work references missing author"; return false; }
        if (work.institutionStableId != 0 && !candidate.institutions_.contains(work.institutionStableId)) { if (error) *error = "work references missing institution"; return false; }
    }
    for (const auto& tx : candidate.transmissions_) {
        if (!candidate.works_.contains(tx.workStableId)) { if (error) *error = "transmission references missing work"; return false; }
        if (tx.actorStableId != 0 && !candidate.citizens_.contains(tx.actorStableId)) { if (error) *error = "transmission references missing actor"; return false; }
        if (tx.institutionStableId != 0 && !candidate.institutions_.contains(tx.institutionStableId)) { if (error) *error = "transmission references missing institution"; return false; }
        const auto ev = std::find_if(candidate.history_.begin(), candidate.history_.end(), [&](const CivicHistoryEvent& e) { return e.stableId == tx.stableId; });
        if (ev == candidate.history_.end() || ev->type != CivicHistoryEventType::CulturalWorkTransmission || ev->subjectStableId != tx.workStableId) {
            if (error) *error = "transmission/history linkage mismatch";
            return false;
        }
    }

    *this = std::move(candidate);
    if (error) error->clear();
    return true;
}

} // namespace elysium
