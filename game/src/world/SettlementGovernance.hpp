// Intended function: imported world implementation for SettlementGovernance; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

// Fourth Edition Part 23 portable governance substrate. These records are
// intentionally EnTT-facing POD-style state: the standalone headless stream
// stores them in deterministic component tables, while the authoritative build
// may attach the same records to EnTT entities. Persistent identity is always
// StableId-like uint64_t data and never entt::entity.

enum class GovernanceJurisdictionKind : std::uint8_t {
    Settlement = 0,
    Site = 1,
    District = 2,
    Planet = 3,
    StarSystem = 4,
    Organization = 5
};

struct GovernanceJurisdiction {
    GovernanceJurisdictionKind kind{GovernanceJurisdictionKind::Settlement};
    std::uint64_t stableId{};

    friend bool operator==(const GovernanceJurisdiction&, const GovernanceJurisdiction&) = default;
};

enum class GovernanceOfficeKind : std::uint8_t {
    SettlementGovernor = 0,
    OperationsManager = 1,
    QuartermasterBookkeeper = 2,
    ChiefMedicalOfficer = 3,
    SecurityChief = 4,
    MilitaryCommander = 5,
    ResearchDirector = 6,
    BrokerTradeEnvoy = 7,
    ClaimPrefect = 8
};

enum class OfficeAppointmentRule : std::uint8_t {
    Appointed = 0,
    Elected = 1
};

enum class OfficeSuccessionRule : std::uint8_t {
    LeaveVacant = 0,
    OrderedCandidates = 1
};

enum class OfficeAuthority : std::uint32_t {
    None = 0,
    EmergencyPolicy = 1u << 0u,
    Diplomacy = 1u << 1u,
    WorkOrders = 1u << 2u,
    LaborPolicy = 1u << 3u,
    Accounting = 1u << 4u,
    MedicalPolicy = 1u << 5u,
    Justice = 1u << 6u,
    AccessPolicy = 1u << 7u,
    Military = 1u << 8u,
    Research = 1u << 9u,
    Trade = 1u << 10u,
    ImperialFiling = 1u << 11u,
    Claims = 1u << 12u
};

constexpr std::uint32_t operator|(OfficeAuthority a, OfficeAuthority b) {
    return static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b);
}

struct OfficeComponent {
    std::uint64_t stableId{};
    GovernanceOfficeKind kind{GovernanceOfficeKind::SettlementGovernor};
    GovernanceJurisdiction jurisdiction{};
    std::uint64_t holderStableId{}; // 0 means vacant.
    OfficeAppointmentRule appointmentRule{OfficeAppointmentRule::Appointed};
    OfficeSuccessionRule successionRule{OfficeSuccessionRule::OrderedCandidates};
    std::uint32_t responsibilities{};
    std::uint64_t requiredWorkspaceStableId{}; // optional room/institution StableId.
    std::int32_t requiredStatusLevel{}; // social/administrative status requirement, content-defined.
    std::uint32_t privileges{};
    bool significantAuthority{true};

    friend bool operator==(const OfficeComponent&, const OfficeComponent&) = default;
};

struct OfficeCandidateComponent {
    std::uint64_t officeStableId{};
    std::uint64_t personStableId{};
    std::int32_t priority{};
    bool eligible{true};

    friend bool operator==(const OfficeCandidateComponent&, const OfficeCandidateComponent&) = default;
};

enum class SettlementPolicyKind : std::uint8_t {
    Labor = 0,
    Immigration = 1,
    VisitorAccess = 2,
    RationPriority = 3,
    PowerShedding = 4,
    Quarantine = 5,
    MilitaryReadiness = 6,
    ArtifactProtection = 7,
    TradeRestrictions = 8,
    EmergencyEvacuation = 9,
    JusticeSeverity = 10,
    AutomationPermissions = 11,
    EnvironmentalExposure = 12,
    ImperialFiling = 13
};

// Policy values are intentionally generic here because Part 23 defines the
// policy domains, not their final tuning vocabulary. 0..3 is a small durable
// level used by downstream systems; content/UI may map those levels to named
// choices without changing the save contract.
struct SettlementPolicyComponent {
    SettlementPolicyKind kind{SettlementPolicyKind::Labor};
    std::int32_t value{1};
    std::uint64_t changedByStableId{};
    std::uint64_t revision{};

    friend bool operator==(const SettlementPolicyComponent&, const SettlementPolicyComponent&) = default;
};

enum class MandateConsequenceKind : std::uint8_t {
    None = 0,
    GenerateJob = 1,
    Economy = 2,
    Social = 3
};

enum class MandateState : std::uint8_t {
    Active = 0,
    Fulfilled = 1,
    Cancelled = 2,
    Failed = 3
};

struct MandateComponent {
    std::uint64_t stableId{};
    std::uint64_t issuerStableId{};
    GovernanceJurisdiction scope{};
    std::string rationale;
    std::uint64_t deadlineTick{}; // 0 means no deadline.
    MandateConsequenceKind consequence{MandateConsequenceKind::None};
    std::int32_t consequenceMagnitude{};
    std::string legalBasis;
    std::string cancellationCondition;
    MandateState state{MandateState::Active};

    friend bool operator==(const MandateComponent&, const MandateComponent&) = default;
};

struct LocalGovernmentComponent {
    std::uint64_t organizationStableId{};
    std::uint64_t settlementSiteStableId{};
    GovernanceJurisdiction jurisdiction{};
    std::uint64_t foundingAuthorityOfficeStableId{};

    friend bool operator==(const LocalGovernmentComponent&, const LocalGovernmentComponent&) = default;
};

enum class GovernanceEventKind : std::uint8_t {
    OfficeAppointed = 0,
    OfficeVacated = 1,
    OfficeSucceeded = 2,
    PolicyChanged = 3,
    MandateCreated = 4,
    MandateStateChanged = 5
};

struct GovernanceEvent {
    std::uint64_t eventStableId{};
    std::uint64_t tick{};
    GovernanceEventKind kind{GovernanceEventKind::PolicyChanged};
    std::uint64_t subjectStableId{};
    std::uint64_t actorStableId{};
    SettlementPolicyKind policyKind{SettlementPolicyKind::Labor};
    std::int32_t oldValue{};
    std::int32_t newValue{};
    bool historical{};

    friend bool operator==(const GovernanceEvent&, const GovernanceEvent&) = default;
};

enum class GovernanceActionKind : std::uint8_t {
    JobRequest = 0,
    EconomyConsequence = 1,
    SocialConsequence = 2
};

struct GovernanceActionRequest {
    std::uint64_t requestStableId{};
    std::uint64_t tick{};
    GovernanceActionKind kind{GovernanceActionKind::JobRequest};
    std::uint64_t sourceMandateStableId{};
    GovernanceJurisdiction scope{};
    std::int32_t magnitude{};

    friend bool operator==(const GovernanceActionRequest&, const GovernanceActionRequest&) = default;
};

struct GovernanceSnapshot {
    static constexpr std::uint32_t SchemaVersion = 1;

    std::uint32_t schemaVersion{SchemaVersion};
    std::uint64_t settlementStableId{};
    std::uint64_t nextSequence{1};
    std::optional<LocalGovernmentComponent> localGovernment;
    std::vector<OfficeComponent> offices;
    std::vector<OfficeCandidateComponent> candidates;
    std::vector<SettlementPolicyComponent> policies;
    std::vector<MandateComponent> mandates;
    std::vector<GovernanceEvent> history;
};

class SettlementGovernanceSystem {
public:
    explicit SettlementGovernanceSystem(std::uint64_t settlementStableId = 0);

    std::uint64_t settlementStableId() const { return settlementStableId_; }

    bool setLocalGovernment(LocalGovernmentComponent government, std::string* error = nullptr);
    const std::optional<LocalGovernmentComponent>& localGovernment() const { return localGovernment_; }

    bool addOffice(OfficeComponent office, std::string* error = nullptr);
    const OfficeComponent* office(std::uint64_t officeStableId) const;
    std::vector<OfficeComponent> offices() const;

    bool setOfficeCandidates(std::uint64_t officeStableId,
                             std::vector<OfficeCandidateComponent> candidates,
                             std::string* error = nullptr);
    std::vector<OfficeCandidateComponent> officeCandidates(std::uint64_t officeStableId) const;
    std::vector<std::uint64_t> officeObligationIds(std::uint64_t officeStableId) const;

    bool appointOffice(std::uint64_t officeStableId, std::uint64_t holderStableId,
                       std::uint64_t tick, std::uint64_t actorStableId = 0,
                       std::string* error = nullptr);
    bool vacateOffice(std::uint64_t officeStableId, std::uint64_t tick,
                      std::uint64_t actorStableId = 0,
                      std::string* error = nullptr);

    bool setPolicy(SettlementPolicyKind kind, std::int32_t value,
                   std::uint64_t tick, std::uint64_t actorStableId = 0,
                   std::string* error = nullptr);
    std::optional<SettlementPolicyComponent> policy(SettlementPolicyKind kind) const;
    std::int32_t policyValue(SettlementPolicyKind kind, std::int32_t fallback = 1) const;
    bool policyAllows(SettlementPolicyKind kind, std::int32_t minimumValue) const;
    std::int32_t policyPriorityBias(SettlementPolicyKind kind) const;

    // Part 35 Why-inspector seams. These are derived diagnostics only: they
    // never own authoritative state and are safe to rebuild at any time.
    std::string inspectOffice(std::uint64_t officeStableId) const;
    std::string inspectPolicy(SettlementPolicyKind kind) const;
    std::string inspectMandate(std::uint64_t mandateStableId, std::uint64_t currentTick = 0) const;

    bool addMandate(MandateComponent mandate, std::uint64_t tick,
                    std::string* error = nullptr);
    const MandateComponent* mandate(std::uint64_t mandateStableId) const;
    bool setMandateState(std::uint64_t mandateStableId, MandateState state,
                         std::uint64_t tick, std::uint64_t actorStableId = 0,
                         std::string* error = nullptr);
    void evaluateMandates(std::uint64_t tick);

    const std::vector<GovernanceEvent>& pendingEvents() const { return pendingEvents_; }
    std::vector<GovernanceEvent> takePendingEvents();
    const std::vector<GovernanceActionRequest>& pendingActions() const { return pendingActions_; }
    std::vector<GovernanceActionRequest> takePendingActions();
    const std::vector<GovernanceEvent>& history() const { return history_; }

    GovernanceSnapshot snapshot() const;
    bool restore(const GovernanceSnapshot& snapshot, std::string* error = nullptr);

private:
    std::uint64_t allocateStableEventId();
    void emitEvent(std::uint64_t tick, GovernanceEventKind kind,
                   std::uint64_t subjectStableId, std::uint64_t actorStableId,
                   bool historical,
                   SettlementPolicyKind policyKind = SettlementPolicyKind::Labor,
                   std::int32_t oldValue = 0, std::int32_t newValue = 0);
    void emitMandateConsequence(const MandateComponent& mandate, std::uint64_t tick);

    std::uint64_t settlementStableId_{};
    std::uint64_t nextSequence_{1};
    std::optional<LocalGovernmentComponent> localGovernment_;
    std::vector<OfficeComponent> offices_;
    std::vector<OfficeCandidateComponent> candidates_;
    std::vector<SettlementPolicyComponent> policies_;
    std::vector<MandateComponent> mandates_;
    std::vector<GovernanceEvent> pendingEvents_;
    std::vector<GovernanceActionRequest> pendingActions_;
    std::vector<GovernanceEvent> history_;
};

bool validateGovernanceSnapshot(const GovernanceSnapshot& snapshot, std::string* error = nullptr);
std::string serializeGovernanceSnapshot(const GovernanceSnapshot& snapshot);
std::optional<GovernanceSnapshot> deserializeGovernanceSnapshot(std::string_view text, std::string* error = nullptr);

const char* governanceOfficeKindName(GovernanceOfficeKind kind);
const char* settlementPolicyKindName(SettlementPolicyKind kind);

} // namespace elysium
