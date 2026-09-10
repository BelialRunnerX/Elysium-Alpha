// Intended function: imported world implementation for SettlementGovernance; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/SettlementGovernance.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <set>
#include <sstream>
#include <type_traits>
#include <unordered_set>

namespace elysium {
namespace {

constexpr std::uint64_t kGovernanceEventLabel = 0x474F5645524E414EULL; // GOVERNAN

bool setError(std::string* error, std::string message) {
    if (error) *error = std::move(message);
    return false;
}

template<class T>
bool enumInRange(T value, T last) {
    using U = std::underlying_type_t<T>;
    return static_cast<U>(value) >= 0 && static_cast<U>(value) <= static_cast<U>(last);
}

bool validJurisdiction(const GovernanceJurisdiction& j) {
    return enumInRange(j.kind, GovernanceJurisdictionKind::Organization) && j.stableId != 0;
}

bool validPolicyValue(std::int32_t value) {
    return value >= 0 && value <= 3;
}

std::uint32_t defaultResponsibilities(GovernanceOfficeKind kind) {
    switch (kind) {
        case GovernanceOfficeKind::SettlementGovernor:
            return OfficeAuthority::EmergencyPolicy | OfficeAuthority::Diplomacy;
        case GovernanceOfficeKind::OperationsManager:
            return OfficeAuthority::WorkOrders | OfficeAuthority::LaborPolicy;
        case GovernanceOfficeKind::QuartermasterBookkeeper:
            return static_cast<std::uint32_t>(OfficeAuthority::Accounting);
        case GovernanceOfficeKind::ChiefMedicalOfficer:
            return static_cast<std::uint32_t>(OfficeAuthority::MedicalPolicy);
        case GovernanceOfficeKind::SecurityChief:
            return OfficeAuthority::Justice | OfficeAuthority::AccessPolicy;
        case GovernanceOfficeKind::MilitaryCommander:
            return static_cast<std::uint32_t>(OfficeAuthority::Military);
        case GovernanceOfficeKind::ResearchDirector:
            return static_cast<std::uint32_t>(OfficeAuthority::Research);
        case GovernanceOfficeKind::BrokerTradeEnvoy:
            return OfficeAuthority::Trade | OfficeAuthority::Diplomacy;
        case GovernanceOfficeKind::ClaimPrefect:
            return OfficeAuthority::ImperialFiling | OfficeAuthority::Claims;
    }
    return 0;
}

GovernanceActionKind actionKindFor(MandateConsequenceKind kind) {
    switch (kind) {
        case MandateConsequenceKind::GenerateJob: return GovernanceActionKind::JobRequest;
        case MandateConsequenceKind::Economy: return GovernanceActionKind::EconomyConsequence;
        case MandateConsequenceKind::Social: return GovernanceActionKind::SocialConsequence;
        case MandateConsequenceKind::None: break;
    }
    return GovernanceActionKind::JobRequest;
}

template<class T, class KeyFn>
bool uniqueBy(const std::vector<T>& values, KeyFn keyFn) {
    std::unordered_set<std::uint64_t> seen;
    for (const auto& value : values) {
        if (!seen.insert(keyFn(value)).second) return false;
    }
    return true;
}

} // namespace

const char* governanceOfficeKindName(GovernanceOfficeKind kind) {
    switch (kind) {
        case GovernanceOfficeKind::SettlementGovernor: return "Settlement Governor";
        case GovernanceOfficeKind::OperationsManager: return "Operations Manager";
        case GovernanceOfficeKind::QuartermasterBookkeeper: return "Quartermaster / Bookkeeper";
        case GovernanceOfficeKind::ChiefMedicalOfficer: return "Chief Medical Officer";
        case GovernanceOfficeKind::SecurityChief: return "Security Chief";
        case GovernanceOfficeKind::MilitaryCommander: return "Military Commander";
        case GovernanceOfficeKind::ResearchDirector: return "Research Director";
        case GovernanceOfficeKind::BrokerTradeEnvoy: return "Broker / Trade Envoy";
        case GovernanceOfficeKind::ClaimPrefect: return "Claim Prefect";
    }
    return "Office";
}

const char* settlementPolicyKindName(SettlementPolicyKind kind) {
    switch (kind) {
        case SettlementPolicyKind::Labor: return "Labor";
        case SettlementPolicyKind::Immigration: return "Immigration";
        case SettlementPolicyKind::VisitorAccess: return "Visitor Access";
        case SettlementPolicyKind::RationPriority: return "Ration Priority";
        case SettlementPolicyKind::PowerShedding: return "Power Shedding";
        case SettlementPolicyKind::Quarantine: return "Quarantine";
        case SettlementPolicyKind::MilitaryReadiness: return "Military Readiness";
        case SettlementPolicyKind::ArtifactProtection: return "Artifact Protection";
        case SettlementPolicyKind::TradeRestrictions: return "Trade Restrictions";
        case SettlementPolicyKind::EmergencyEvacuation: return "Emergency Evacuation";
        case SettlementPolicyKind::JusticeSeverity: return "Justice Severity";
        case SettlementPolicyKind::AutomationPermissions: return "Automation Permissions";
        case SettlementPolicyKind::EnvironmentalExposure: return "Environmental Exposure";
        case SettlementPolicyKind::ImperialFiling: return "Imperial Filing";
    }
    return "Policy";
}

SettlementGovernanceSystem::SettlementGovernanceSystem(std::uint64_t settlementStableId)
    : settlementStableId_(settlementStableId) {}

bool SettlementGovernanceSystem::setLocalGovernment(LocalGovernmentComponent government, std::string* error) {
    if (government.organizationStableId == 0 || government.settlementSiteStableId == 0)
        return setError(error, "local government requires non-zero organization and site StableIds");
    if (!validJurisdiction(government.jurisdiction))
        return setError(error, "local government has invalid jurisdiction");
    if (settlementStableId_ == 0) settlementStableId_ = government.settlementSiteStableId;
    localGovernment_ = government;
    return true;
}

bool SettlementGovernanceSystem::addOffice(OfficeComponent officeValue, std::string* error) {
    if (officeValue.stableId == 0) return setError(error, "office StableId must be non-zero");
    if (!enumInRange(officeValue.kind, GovernanceOfficeKind::ClaimPrefect))
        return setError(error, "office kind is invalid");
    if (!validJurisdiction(officeValue.jurisdiction))
        return setError(error, "office jurisdiction is invalid");
    if (office(officeValue.stableId)) return setError(error, "duplicate office StableId");
    if (officeValue.responsibilities == 0) officeValue.responsibilities = defaultResponsibilities(officeValue.kind);
    offices_.push_back(officeValue);
    std::sort(offices_.begin(), offices_.end(), [](const auto& a, const auto& b){ return a.stableId < b.stableId; });
    return true;
}

const OfficeComponent* SettlementGovernanceSystem::office(std::uint64_t officeStableId) const {
    const auto it = std::lower_bound(offices_.begin(), offices_.end(), officeStableId,
                                     [](const auto& value, std::uint64_t id){ return value.stableId < id; });
    return it != offices_.end() && it->stableId == officeStableId ? &*it : nullptr;
}

std::vector<OfficeComponent> SettlementGovernanceSystem::offices() const { return offices_; }

bool SettlementGovernanceSystem::setOfficeCandidates(std::uint64_t officeStableId,
                                                       std::vector<OfficeCandidateComponent> newCandidates,
                                                       std::string* error) {
    if (!office(officeStableId)) return setError(error, "candidate list references unknown office");
    std::unordered_set<std::uint64_t> seen;
    for (auto& candidate : newCandidates) {
        candidate.officeStableId = officeStableId;
        if (candidate.personStableId == 0) return setError(error, "office candidate StableId must be non-zero");
        if (!seen.insert(candidate.personStableId).second) return setError(error, "duplicate office candidate StableId");
    }
    candidates_.erase(std::remove_if(candidates_.begin(), candidates_.end(),
                                     [&](const auto& c){ return c.officeStableId == officeStableId; }), candidates_.end());
    candidates_.insert(candidates_.end(), newCandidates.begin(), newCandidates.end());
    std::sort(candidates_.begin(), candidates_.end(), [](const auto& a, const auto& b){
        if (a.officeStableId != b.officeStableId) return a.officeStableId < b.officeStableId;
        if (a.priority != b.priority) return a.priority > b.priority;
        return a.personStableId < b.personStableId;
    });
    return true;
}

std::vector<OfficeCandidateComponent> SettlementGovernanceSystem::officeCandidates(std::uint64_t officeStableId) const {
    std::vector<OfficeCandidateComponent> out;
    for (const auto& candidate : candidates_) if (candidate.officeStableId == officeStableId) out.push_back(candidate);
    return out;
}

std::vector<std::uint64_t> SettlementGovernanceSystem::officeObligationIds(std::uint64_t officeStableId) const {
    std::vector<std::uint64_t> out;
    for (const auto& mandateValue : mandates_)
        if (mandateValue.issuerStableId == officeStableId && mandateValue.state == MandateState::Active) out.push_back(mandateValue.stableId);
    return out;
}

bool SettlementGovernanceSystem::appointOffice(std::uint64_t officeStableId, std::uint64_t holderStableId,
                                                std::uint64_t tick, std::uint64_t actorStableId,
                                                std::string* error) {
    if (holderStableId == 0) return setError(error, "office holder StableId must be non-zero");
    auto it = std::lower_bound(offices_.begin(), offices_.end(), officeStableId,
                               [](const auto& value, std::uint64_t id){ return value.stableId < id; });
    if (it == offices_.end() || it->stableId != officeStableId) return setError(error, "unknown office StableId");
    if (it->holderStableId == holderStableId) return true;
    it->holderStableId = holderStableId;
    emitEvent(tick, GovernanceEventKind::OfficeAppointed, officeStableId,
              actorStableId ? actorStableId : holderStableId, it->significantAuthority);
    return true;
}

bool SettlementGovernanceSystem::vacateOffice(std::uint64_t officeStableId, std::uint64_t tick,
                                               std::uint64_t actorStableId, std::string* error) {
    auto it = std::lower_bound(offices_.begin(), offices_.end(), officeStableId,
                               [](const auto& value, std::uint64_t id){ return value.stableId < id; });
    if (it == offices_.end() || it->stableId != officeStableId) return setError(error, "unknown office StableId");

    const std::uint64_t previousHolder = it->holderStableId;
    it->holderStableId = 0;
    emitEvent(tick, GovernanceEventKind::OfficeVacated, officeStableId,
              actorStableId ? actorStableId : previousHolder, it->significantAuthority);

    if (it->successionRule == OfficeSuccessionRule::OrderedCandidates) {
        const auto list = officeCandidates(officeStableId);
        const auto successor = std::find_if(list.begin(), list.end(), [&](const auto& candidate){
            return candidate.eligible && candidate.personStableId != previousHolder;
        });
        if (successor != list.end()) {
            it->holderStableId = successor->personStableId;
            emitEvent(tick, GovernanceEventKind::OfficeSucceeded, officeStableId,
                      successor->personStableId, it->significantAuthority);
        }
    }
    return true;
}

bool SettlementGovernanceSystem::setPolicy(SettlementPolicyKind kind, std::int32_t value,
                                            std::uint64_t tick, std::uint64_t actorStableId,
                                            std::string* error) {
    if (!enumInRange(kind, SettlementPolicyKind::ImperialFiling)) return setError(error, "policy kind is invalid");
    if (!validPolicyValue(value)) return setError(error, "policy value must be in durable range 0..3");
    auto it = std::find_if(policies_.begin(), policies_.end(), [&](const auto& p){ return p.kind == kind; });
    const std::int32_t oldValue = it == policies_.end() ? 1 : it->value;
    if (it != policies_.end() && it->value == value) return true;
    if (it == policies_.end()) {
        SettlementPolicyComponent p{};
        p.kind = kind;
        p.value = value;
        p.changedByStableId = actorStableId;
        p.revision = 1;
        policies_.push_back(p);
    } else {
        it->value = value;
        it->changedByStableId = actorStableId;
        ++it->revision;
    }
    std::sort(policies_.begin(), policies_.end(), [](const auto& a, const auto& b){
        return static_cast<int>(a.kind) < static_cast<int>(b.kind);
    });
    emitEvent(tick, GovernanceEventKind::PolicyChanged, settlementStableId_, actorStableId,
              true, kind, oldValue, value);
    return true;
}

std::optional<SettlementPolicyComponent> SettlementGovernanceSystem::policy(SettlementPolicyKind kind) const {
    const auto it = std::find_if(policies_.begin(), policies_.end(), [&](const auto& p){ return p.kind == kind; });
    if (it == policies_.end()) return std::nullopt;
    return *it;
}

std::int32_t SettlementGovernanceSystem::policyValue(SettlementPolicyKind kind, std::int32_t fallback) const {
    const auto p = policy(kind);
    return p ? p->value : fallback;
}

bool SettlementGovernanceSystem::policyAllows(SettlementPolicyKind kind, std::int32_t minimumValue) const {
    return policyValue(kind) >= minimumValue;
}

std::int32_t SettlementGovernanceSystem::policyPriorityBias(SettlementPolicyKind kind) const {
    // Durable generic integration seam: level 1 is neutral, 0 deprioritizes,
    // 2/3 increase priority. Domain systems retain the final semantic mapping.
    return policyValue(kind) - 1;
}

std::string SettlementGovernanceSystem::inspectOffice(std::uint64_t officeStableId) const {
    const auto* value = office(officeStableId);
    if (!value) return "office: unknown StableId=" + std::to_string(officeStableId);

    std::ostringstream out;
    out << "office: " << governanceOfficeKindName(value->kind)
        << " StableId=" << value->stableId
        << " jurisdiction=" << static_cast<int>(value->jurisdiction.kind) << ':' << value->jurisdiction.stableId
        << " holder=";
    if (value->holderStableId) out << value->holderStableId; else out << "vacant";
    out << " appointment=" << (value->appointmentRule == OfficeAppointmentRule::Elected ? "elected" : "appointed")
        << " succession=" << (value->successionRule == OfficeSuccessionRule::OrderedCandidates ? "ordered-candidates" : "leave-vacant")
        << " workspace=" << value->requiredWorkspaceStableId
        << " status-required=" << value->requiredStatusLevel;

    const auto candidates = officeCandidates(officeStableId);
    std::size_t eligibleCount = 0;
    for (const auto& candidate : candidates) if (candidate.eligible) ++eligibleCount;
    out << " candidates=" << candidates.size() << " eligible=" << eligibleCount
        << " active-obligations=" << officeObligationIds(officeStableId).size();
    if (value->holderStableId == 0 && value->successionRule == OfficeSuccessionRule::OrderedCandidates && eligibleCount == 0)
        out << " blocker=no eligible successor; remediation=appoint or mark a candidate eligible";
    return out.str();
}

std::string SettlementGovernanceSystem::inspectPolicy(SettlementPolicyKind kind) const {
    if (!enumInRange(kind, SettlementPolicyKind::ImperialFiling)) return "policy: invalid kind";
    const auto value = policy(kind);
    std::ostringstream out;
    out << "policy: " << settlementPolicyKindName(kind);
    if (!value) {
        out << " value=1(default) revision=0 source=implicit-neutral";
    } else {
        out << " value=" << value->value << " revision=" << value->revision
            << " changed-by=" << value->changedByStableId;
    }
    out << " priority-bias=" << policyPriorityBias(kind);
    return out.str();
}

std::string SettlementGovernanceSystem::inspectMandate(std::uint64_t mandateStableId, std::uint64_t currentTick) const {
    const auto* value = mandate(mandateStableId);
    if (!value) return "mandate: unknown StableId=" + std::to_string(mandateStableId);

    std::ostringstream out;
    out << "mandate: StableId=" << value->stableId
        << " issuer=" << value->issuerStableId
        << " scope=" << static_cast<int>(value->scope.kind) << ':' << value->scope.stableId
        << " state=" << static_cast<int>(value->state)
        << " deadline=" << value->deadlineTick
        << " consequence=" << static_cast<int>(value->consequence)
        << " magnitude=" << value->consequenceMagnitude
        << " rationale=" << std::quoted(value->rationale)
        << " legal-basis=" << std::quoted(value->legalBasis)
        << " cancellation=" << std::quoted(value->cancellationCondition);
    if (value->state == MandateState::Active && value->deadlineTick != 0 && currentTick != 0) {
        if (currentTick >= value->deadlineTick) out << " blocker=deadline reached; remediation=fulfill/cancel before evaluation";
        else out << " ticks-remaining=" << (value->deadlineTick - currentTick);
    }
    return out.str();
}

bool SettlementGovernanceSystem::addMandate(MandateComponent mandateValue, std::uint64_t tick,
                                             std::string* error) {
    if (mandateValue.stableId == 0 || mandateValue.issuerStableId == 0)
        return setError(error, "mandate and issuer StableIds must be non-zero");
    if (!validJurisdiction(mandateValue.scope)) return setError(error, "mandate scope is invalid");
    if (!enumInRange(mandateValue.consequence, MandateConsequenceKind::Social))
        return setError(error, "mandate consequence is invalid");
    if (!enumInRange(mandateValue.state, MandateState::Failed)) return setError(error, "mandate state is invalid");
    if (mandate(mandateValue.stableId)) return setError(error, "duplicate mandate StableId");
    const auto mandateId = mandateValue.stableId;
    const auto issuerId = mandateValue.issuerStableId;
    mandates_.push_back(std::move(mandateValue));
    std::sort(mandates_.begin(), mandates_.end(), [](const auto& a, const auto& b){ return a.stableId < b.stableId; });
    emitEvent(tick, GovernanceEventKind::MandateCreated, mandateId, issuerId, true);
    return true;
}

const MandateComponent* SettlementGovernanceSystem::mandate(std::uint64_t mandateStableId) const {
    const auto it = std::lower_bound(mandates_.begin(), mandates_.end(), mandateStableId,
                                     [](const auto& value, std::uint64_t id){ return value.stableId < id; });
    return it != mandates_.end() && it->stableId == mandateStableId ? &*it : nullptr;
}

bool SettlementGovernanceSystem::setMandateState(std::uint64_t mandateStableId, MandateState state,
                                                  std::uint64_t tick, std::uint64_t actorStableId,
                                                  std::string* error) {
    if (!enumInRange(state, MandateState::Failed)) return setError(error, "mandate state is invalid");
    auto it = std::lower_bound(mandates_.begin(), mandates_.end(), mandateStableId,
                               [](const auto& value, std::uint64_t id){ return value.stableId < id; });
    if (it == mandates_.end() || it->stableId != mandateStableId) return setError(error, "unknown mandate StableId");
    if (it->state == state) return true;
    const auto old = it->state;
    it->state = state;
    emitEvent(tick, GovernanceEventKind::MandateStateChanged, mandateStableId, actorStableId,
              true, SettlementPolicyKind::Labor, static_cast<int>(old), static_cast<int>(state));
    if (state == MandateState::Failed) emitMandateConsequence(*it, tick);
    return true;
}

void SettlementGovernanceSystem::evaluateMandates(std::uint64_t tick) {
    for (auto& mandateValue : mandates_) {
        if (mandateValue.state != MandateState::Active || mandateValue.deadlineTick == 0 || tick < mandateValue.deadlineTick)
            continue;
        const auto old = mandateValue.state;
        mandateValue.state = MandateState::Failed;
        emitEvent(tick, GovernanceEventKind::MandateStateChanged, mandateValue.stableId,
                  mandateValue.issuerStableId, true, SettlementPolicyKind::Labor,
                  static_cast<int>(old), static_cast<int>(mandateValue.state));
        emitMandateConsequence(mandateValue, tick);
    }
}

std::vector<GovernanceEvent> SettlementGovernanceSystem::takePendingEvents() {
    auto out = std::move(pendingEvents_);
    pendingEvents_.clear();
    return out;
}

std::vector<GovernanceActionRequest> SettlementGovernanceSystem::takePendingActions() {
    auto out = std::move(pendingActions_);
    pendingActions_.clear();
    return out;
}

std::uint64_t SettlementGovernanceSystem::allocateStableEventId() {
    for (;;) {
        const std::uint64_t sequence = nextSequence_++;
        std::uint64_t id = mix64(settlementStableId_ ^ mix64(kGovernanceEventLabel ^ sequence));
        if (id == 0) id = sequence ? sequence : 1;
        const bool usedHistory = std::any_of(history_.begin(), history_.end(), [&](const auto& event){ return event.eventStableId == id; });
        const bool usedPending = std::any_of(pendingEvents_.begin(), pendingEvents_.end(), [&](const auto& event){ return event.eventStableId == id; });
        const bool usedAction = std::any_of(pendingActions_.begin(), pendingActions_.end(), [&](const auto& action){ return action.requestStableId == id; });
        if (!usedHistory && !usedPending && !usedAction) return id;
    }
}

void SettlementGovernanceSystem::emitEvent(std::uint64_t tick, GovernanceEventKind kind,
                                            std::uint64_t subjectStableId, std::uint64_t actorStableId,
                                            bool historical, SettlementPolicyKind policyKind,
                                            std::int32_t oldValue, std::int32_t newValue) {
    GovernanceEvent event{};
    event.eventStableId = allocateStableEventId();
    event.tick = tick;
    event.kind = kind;
    event.subjectStableId = subjectStableId;
    event.actorStableId = actorStableId;
    event.policyKind = policyKind;
    event.oldValue = oldValue;
    event.newValue = newValue;
    event.historical = historical;
    pendingEvents_.push_back(event);
    if (historical) history_.push_back(event);
}

void SettlementGovernanceSystem::emitMandateConsequence(const MandateComponent& mandateValue, std::uint64_t tick) {
    if (mandateValue.consequence == MandateConsequenceKind::None) return;
    GovernanceActionRequest request{};
    request.requestStableId = allocateStableEventId();
    request.tick = tick;
    request.kind = actionKindFor(mandateValue.consequence);
    request.sourceMandateStableId = mandateValue.stableId;
    request.scope = mandateValue.scope;
    request.magnitude = mandateValue.consequenceMagnitude;
    pendingActions_.push_back(request);
}

GovernanceSnapshot SettlementGovernanceSystem::snapshot() const {
    GovernanceSnapshot out{};
    out.settlementStableId = settlementStableId_;
    out.nextSequence = nextSequence_;
    out.localGovernment = localGovernment_;
    out.offices = offices_;
    out.candidates = candidates_;
    out.policies = policies_;
    out.mandates = mandates_;
    out.history = history_;
    return out;
}

bool SettlementGovernanceSystem::restore(const GovernanceSnapshot& snapshotValue, std::string* error) {
    if (!validateGovernanceSnapshot(snapshotValue, error)) return false;
    settlementStableId_ = snapshotValue.settlementStableId;
    nextSequence_ = snapshotValue.nextSequence;
    localGovernment_ = snapshotValue.localGovernment;
    offices_ = snapshotValue.offices;
    candidates_ = snapshotValue.candidates;
    policies_ = snapshotValue.policies;
    mandates_ = snapshotValue.mandates;
    history_ = snapshotValue.history;
    pendingEvents_.clear();
    pendingActions_.clear();
    return true;
}

bool validateGovernanceSnapshot(const GovernanceSnapshot& snapshot, std::string* error) {
    if (snapshot.schemaVersion != GovernanceSnapshot::SchemaVersion) return setError(error, "unsupported governance snapshot schema");
    if (snapshot.settlementStableId == 0) return setError(error, "governance snapshot settlement StableId is zero");
    if (snapshot.nextSequence == 0) return setError(error, "governance snapshot next sequence is zero");

    if (snapshot.localGovernment) {
        const auto& g = *snapshot.localGovernment;
        if (g.organizationStableId == 0 || g.settlementSiteStableId == 0 || !validJurisdiction(g.jurisdiction))
            return setError(error, "governance snapshot local government is invalid");
    }
    if (!uniqueBy(snapshot.offices, [](const auto& value){ return value.stableId; }))
        return setError(error, "governance snapshot has duplicate office StableId");
    for (const auto& officeValue : snapshot.offices) {
        if (officeValue.stableId == 0 || !validJurisdiction(officeValue.jurisdiction) ||
            !enumInRange(officeValue.kind, GovernanceOfficeKind::ClaimPrefect) ||
            !enumInRange(officeValue.appointmentRule, OfficeAppointmentRule::Elected) ||
            !enumInRange(officeValue.successionRule, OfficeSuccessionRule::OrderedCandidates))
            return setError(error, "governance snapshot contains invalid office record");
    }
    if (snapshot.localGovernment && snapshot.localGovernment->foundingAuthorityOfficeStableId != 0 &&
        std::none_of(snapshot.offices.begin(), snapshot.offices.end(), [&](const auto& officeValue){
            return officeValue.stableId == snapshot.localGovernment->foundingAuthorityOfficeStableId;
        }))
        return setError(error, "governance snapshot founding authority references unknown office");

    for (const auto& candidate : snapshot.candidates) {
        if (candidate.officeStableId == 0 || candidate.personStableId == 0)
            return setError(error, "governance snapshot contains zero candidate StableId");
        if (std::none_of(snapshot.offices.begin(), snapshot.offices.end(), [&](const auto& officeValue){ return officeValue.stableId == candidate.officeStableId; }))
            return setError(error, "governance snapshot candidate references unknown office");
    }
    {
        std::set<std::pair<std::uint64_t, std::uint64_t>> candidateKeys;
        for (const auto& candidate : snapshot.candidates) {
            if (!candidateKeys.emplace(candidate.officeStableId, candidate.personStableId).second)
                return setError(error, "governance snapshot has duplicate office candidate");
        }
    }

    std::unordered_set<int> policyKinds;
    for (const auto& policyValue : snapshot.policies) {
        if (!enumInRange(policyValue.kind, SettlementPolicyKind::ImperialFiling) || !validPolicyValue(policyValue.value) || policyValue.revision == 0)
            return setError(error, "governance snapshot contains invalid policy record");
        if (!policyKinds.insert(static_cast<int>(policyValue.kind)).second)
            return setError(error, "governance snapshot has duplicate policy kind");
    }

    if (!uniqueBy(snapshot.mandates, [](const auto& value){ return value.stableId; }))
        return setError(error, "governance snapshot has duplicate mandate StableId");
    for (const auto& mandateValue : snapshot.mandates) {
        if (mandateValue.stableId == 0 || mandateValue.issuerStableId == 0 || !validJurisdiction(mandateValue.scope) ||
            !enumInRange(mandateValue.consequence, MandateConsequenceKind::Social) || !enumInRange(mandateValue.state, MandateState::Failed))
            return setError(error, "governance snapshot contains invalid mandate record");
    }

    if (!uniqueBy(snapshot.history, [](const auto& value){ return value.eventStableId; }))
        return setError(error, "governance snapshot has duplicate history event StableId");
    for (const auto& event : snapshot.history) {
        if (event.eventStableId == 0 || !event.historical || !enumInRange(event.kind, GovernanceEventKind::MandateStateChanged) ||
            !enumInRange(event.policyKind, SettlementPolicyKind::ImperialFiling))
            return setError(error, "governance snapshot contains invalid history event");
    }
    return true;
}

std::string serializeGovernanceSnapshot(const GovernanceSnapshot& snapshot) {
    std::ostringstream out;
    out << "ELYSIUM_GOVERNANCE " << snapshot.schemaVersion << '\n';
    out << "settlement " << snapshot.settlementStableId << ' ' << snapshot.nextSequence << '\n';
    if (snapshot.localGovernment) {
        const auto& g = *snapshot.localGovernment;
        out << "government 1 " << g.organizationStableId << ' ' << g.settlementSiteStableId << ' '
            << static_cast<int>(g.jurisdiction.kind) << ' ' << g.jurisdiction.stableId << ' '
            << g.foundingAuthorityOfficeStableId << '\n';
    } else {
        out << "government 0\n";
    }
    out << "offices " << snapshot.offices.size() << '\n';
    for (const auto& o : snapshot.offices) {
        out << o.stableId << ' ' << static_cast<int>(o.kind) << ' ' << static_cast<int>(o.jurisdiction.kind) << ' '
            << o.jurisdiction.stableId << ' ' << o.holderStableId << ' ' << static_cast<int>(o.appointmentRule) << ' '
            << static_cast<int>(o.successionRule) << ' ' << o.responsibilities << ' ' << o.requiredWorkspaceStableId << ' '
            << o.requiredStatusLevel << ' ' << o.privileges << ' ' << (o.significantAuthority ? 1 : 0) << '\n';
    }
    out << "candidates " << snapshot.candidates.size() << '\n';
    for (const auto& c : snapshot.candidates)
        out << c.officeStableId << ' ' << c.personStableId << ' ' << c.priority << ' ' << (c.eligible ? 1 : 0) << '\n';
    out << "policies " << snapshot.policies.size() << '\n';
    for (const auto& p : snapshot.policies)
        out << static_cast<int>(p.kind) << ' ' << p.value << ' ' << p.changedByStableId << ' ' << p.revision << '\n';
    out << "mandates " << snapshot.mandates.size() << '\n';
    for (const auto& m : snapshot.mandates) {
        out << m.stableId << ' ' << m.issuerStableId << ' ' << static_cast<int>(m.scope.kind) << ' ' << m.scope.stableId << ' '
            << m.deadlineTick << ' ' << static_cast<int>(m.consequence) << ' ' << m.consequenceMagnitude << ' '
            << static_cast<int>(m.state) << ' ' << std::quoted(m.rationale) << ' ' << std::quoted(m.legalBasis) << ' '
            << std::quoted(m.cancellationCondition) << '\n';
    }
    out << "history " << snapshot.history.size() << '\n';
    for (const auto& e : snapshot.history) {
        out << e.eventStableId << ' ' << e.tick << ' ' << static_cast<int>(e.kind) << ' ' << e.subjectStableId << ' '
            << e.actorStableId << ' ' << static_cast<int>(e.policyKind) << ' ' << e.oldValue << ' ' << e.newValue << ' '
            << (e.historical ? 1 : 0) << '\n';
    }
    return out.str();
}

std::optional<GovernanceSnapshot> deserializeGovernanceSnapshot(std::string_view text, std::string* error) {
    std::istringstream in{std::string(text)};
    std::string token;
    GovernanceSnapshot snapshot{};
    std::uint32_t schema{};
    if (!(in >> token >> schema) || token != "ELYSIUM_GOVERNANCE") {
        setError(error, "governance snapshot header missing");
        return std::nullopt;
    }
    snapshot.schemaVersion = schema;
    if (!(in >> token >> snapshot.settlementStableId >> snapshot.nextSequence) || token != "settlement") {
        setError(error, "governance snapshot settlement line invalid");
        return std::nullopt;
    }
    int hasGovernment{};
    if (!(in >> token >> hasGovernment) || token != "government") {
        setError(error, "governance snapshot government line invalid");
        return std::nullopt;
    }
    if (hasGovernment) {
        LocalGovernmentComponent g{};
        int kind{};
        if (!(in >> g.organizationStableId >> g.settlementSiteStableId >> kind >> g.jurisdiction.stableId >> g.foundingAuthorityOfficeStableId)) {
            setError(error, "governance snapshot government payload invalid");
            return std::nullopt;
        }
        g.jurisdiction.kind = static_cast<GovernanceJurisdictionKind>(kind);
        snapshot.localGovernment = g;
    }

    std::size_t count{};
    if (!(in >> token >> count) || token != "offices") { setError(error, "governance offices section missing"); return std::nullopt; }
    snapshot.offices.reserve(count);
    for (std::size_t i=0; i<count; ++i) {
        OfficeComponent o{};
        int kind{}, jurisdiction{}, appointment{}, succession{}, significant{};
        if (!(in >> o.stableId >> kind >> jurisdiction >> o.jurisdiction.stableId >> o.holderStableId >> appointment >> succession
                 >> o.responsibilities >> o.requiredWorkspaceStableId >> o.requiredStatusLevel >> o.privileges >> significant)) {
            setError(error, "governance office record invalid"); return std::nullopt;
        }
        o.kind = static_cast<GovernanceOfficeKind>(kind);
        o.jurisdiction.kind = static_cast<GovernanceJurisdictionKind>(jurisdiction);
        o.appointmentRule = static_cast<OfficeAppointmentRule>(appointment);
        o.successionRule = static_cast<OfficeSuccessionRule>(succession);
        o.significantAuthority = significant != 0;
        snapshot.offices.push_back(o);
    }

    if (!(in >> token >> count) || token != "candidates") { setError(error, "governance candidates section missing"); return std::nullopt; }
    snapshot.candidates.reserve(count);
    for (std::size_t i=0; i<count; ++i) {
        OfficeCandidateComponent c{}; int eligible{};
        if (!(in >> c.officeStableId >> c.personStableId >> c.priority >> eligible)) { setError(error, "governance candidate record invalid"); return std::nullopt; }
        c.eligible = eligible != 0;
        snapshot.candidates.push_back(c);
    }

    if (!(in >> token >> count) || token != "policies") { setError(error, "governance policies section missing"); return std::nullopt; }
    snapshot.policies.reserve(count);
    for (std::size_t i=0; i<count; ++i) {
        SettlementPolicyComponent p{}; int kind{};
        if (!(in >> kind >> p.value >> p.changedByStableId >> p.revision)) { setError(error, "governance policy record invalid"); return std::nullopt; }
        p.kind = static_cast<SettlementPolicyKind>(kind);
        snapshot.policies.push_back(p);
    }

    if (!(in >> token >> count) || token != "mandates") { setError(error, "governance mandates section missing"); return std::nullopt; }
    snapshot.mandates.reserve(count);
    for (std::size_t i=0; i<count; ++i) {
        MandateComponent m{}; int scope{}, consequence{}, state{};
        if (!(in >> m.stableId >> m.issuerStableId >> scope >> m.scope.stableId >> m.deadlineTick >> consequence >> m.consequenceMagnitude >> state
                 >> std::quoted(m.rationale) >> std::quoted(m.legalBasis) >> std::quoted(m.cancellationCondition))) {
            setError(error, "governance mandate record invalid"); return std::nullopt;
        }
        m.scope.kind = static_cast<GovernanceJurisdictionKind>(scope);
        m.consequence = static_cast<MandateConsequenceKind>(consequence);
        m.state = static_cast<MandateState>(state);
        snapshot.mandates.push_back(std::move(m));
    }

    if (!(in >> token >> count) || token != "history") { setError(error, "governance history section missing"); return std::nullopt; }
    snapshot.history.reserve(count);
    for (std::size_t i=0; i<count; ++i) {
        GovernanceEvent e{}; int kind{}, policy{}, historical{};
        if (!(in >> e.eventStableId >> e.tick >> kind >> e.subjectStableId >> e.actorStableId >> policy >> e.oldValue >> e.newValue >> historical)) {
            setError(error, "governance history record invalid"); return std::nullopt;
        }
        e.kind = static_cast<GovernanceEventKind>(kind);
        e.policyKind = static_cast<SettlementPolicyKind>(policy);
        e.historical = historical != 0;
        snapshot.history.push_back(e);
    }
    if (!validateGovernanceSnapshot(snapshot, error)) return std::nullopt;
    return snapshot;
}

} // namespace elysium
