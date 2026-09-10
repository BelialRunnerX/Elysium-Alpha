// Intended function: imported fortress implementation for AutonomousWork; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "core/JobSystem.hpp"

#include <compare>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace elysium::fortress {

using JobId = std::uint64_t;
using WorkerStableId = std::uint64_t;
using WorkOrderId = std::uint64_t;
using WorkDetailId = std::uint64_t;

struct SurfaceCellRef {
    std::uint64_t worldStableId{};
    std::uint8_t face{};
    std::int32_t u{};
    std::int32_t v{};
    std::int32_t radial{};

    auto operator<=>(const SurfaceCellRef&) const = default;
};

enum class JobOrigin : std::uint8_t {
    Designation,
    Workshop,
    WorkOrder,
    Need,
    Emergency,
    Institution,
    Contract
};

struct JobSourceKey {
    JobOrigin origin{JobOrigin::Designation};
    std::uint64_t stableSourceId{};
    std::uint32_t ordinal{};

    auto operator<=>(const JobSourceKey&) const = default;
};

enum class JobState : std::uint8_t {
    Candidate,
    Pending,
    Claimed,
    Navigating,
    Working,
    Blocked,
    Suspended,
    Completed,
    Cancelled,
    Failed
};

enum class InterruptPolicy : std::uint8_t {
    Never,
    EmergencyOnly,
    HigherPriority,
    FreelyInterruptible
};

enum class HistoryPolicy : std::uint8_t {
    None,
    ImportantOnly,
    Always
};

enum class JobFailureReason : std::uint8_t {
    None,
    DependencyBlocked,
    MissingInput,
    NoEligibleWorker,
    UnsafeRoute,
    UnreachableRoute,
    WorkshopUnavailable,
    PowerFailure,
    EnvironmentFailure,
    ReservationConflict,
    LeaseExpired,
    CancelledByOrigin,
    ExecutionFailed,
    InvalidDefinition
};

const char* failureReasonName(JobFailureReason reason);

enum class ReservationResourceKind : std::uint8_t {
    Item,
    Tool,
    Bed,
    WorkshopSlot,
    TargetVoxel,
    VehicleCapacity,
    Container,
    Service
};

struct ReservationResourceKey {
    ReservationResourceKind kind{ReservationResourceKind::Item};
    std::uint64_t stableIdOrAddressKey{};
    std::uint32_t subslot{};

    auto operator<=>(const ReservationResourceKey&) const = default;
};

struct ReservationClaim {
    JobId jobId{};
    WorkerStableId workerId{};
    double leaseUntil{};
};

struct JobTarget {
    enum class Kind : std::uint8_t { None, StableObject, SurfaceCell, SiteLocation };
    Kind kind{Kind::None};
    std::uint64_t stableObjectId{};
    SurfaceCellRef surfaceCell{};
    std::uint64_t siteId{};
    std::int32_t localX{};
    std::int32_t localY{};
    std::int32_t localZ{};
};

struct JobRequirements {
    std::string laborId;
    std::map<std::string, int> minimumSkill;
    std::set<std::string> requiredCapabilities;
    std::set<std::string> requiredEquipment;
    std::set<std::string> requiredAccess;
    bool requiresCitizen{true};
    bool requiresHealthy{true};
    bool requiresOnSchedule{true};
    std::vector<ReservationResourceKey> reservations;
};

struct WorkerClaim {
    WorkerStableId workerId{};
    double leaseUntil{};
};

struct JobProgress {
    double workDone{};
    double workRequired{1.0};
    std::uint32_t stage{};
};

struct JobRecord {
    JobId id{};
    std::string jobTypeId;
    int explicitPriority{};
    JobOrigin origin{JobOrigin::Designation};
    JobTarget target{};
    JobRequirements requirements{};
    std::set<std::string> materialFilters;
    std::vector<ReservationResourceKey> inputReservations;
    std::optional<ReservationResourceKey> toolReservation;
    std::optional<WorkerClaim> workerClaim;
    std::vector<JobId> dependencies;
    JobProgress progress{};
    InterruptPolicy interruptPolicy{InterruptPolicy::HigherPriority};
    JobFailureReason failureReason{JobFailureReason::None};
    HistoryPolicy historyPolicy{HistoryPolicy::ImportantOnly};
    JobState state{JobState::Candidate};
    WorkOrderId sourceOrderId{};
    std::uint64_t sourceStableId{};
    std::uint32_t sourceOrdinal{};
    std::uint32_t quantity{1};
    std::string diagnosticDetail;
};

enum class LaborPolicy : std::uint8_t {
    Unrestricted,
    Preferred,
    Required,
    Forbidden
};

struct WorkDetailSelector {
    std::set<WorkerStableId> workerIds;
    std::set<std::string> professions;
    std::set<std::uint64_t> squadIds;
    std::set<std::uint64_t> institutionIds;

    bool empty() const;
};

struct WorkDetail {
    WorkDetailId id{};
    std::string name;
    WorkDetailSelector selector;
    std::map<std::string, LaborPolicy> laborPolicies;
};

enum class CitizenshipState : std::uint8_t {
    Visitor,
    Resident,
    Citizen,
    Restricted,
    Detained,
    Exiled
};

struct WorkerSnapshot {
    WorkerStableId stableId{};
    CitizenshipState citizenship{CitizenshipState::Citizen};
    bool healthy{true};
    bool onSchedule{true};
    bool available{true};
    std::string profession;
    std::set<std::uint64_t> squadIds;
    std::set<std::uint64_t> institutionIds;
    std::map<std::string, int> skills;
    std::set<std::string> capabilities;
    std::set<std::string> equipment;
    std::set<std::string> access;
    std::map<std::string, float> workPreferences;
    float throughput{1.0f};
};

struct PathAssessment {
    bool reachable{true};
    bool safe{true};
    float cost{};
};

struct AssignmentContext {
    std::function<bool(const ReservationResourceKey&)> resourceExists;
    std::function<bool(const JobRecord&)> workshopReady;
    std::function<bool(const JobRecord&)> powerReady;
    std::function<bool(const JobRecord&)> environmentReady;
    std::function<PathAssessment(const WorkerSnapshot&, const JobRecord&)> path;
};

struct GeneratedJobSpec {
    std::uint64_t stableSourceId{};
    std::uint32_t ordinal{};
    JobRecord job{};
};

struct JobSyncResult {
    std::vector<JobId> created;
    std::vector<JobId> updated;
    std::vector<JobId> cancelled;
};

struct AssignmentDecision {
    JobId jobId{};
    WorkerStableId workerId{};
    std::int64_t score{};
};

struct JobDiagnostic {
    JobId jobId{};
    JobState state{JobState::Candidate};
    JobFailureReason reason{JobFailureReason::None};
    std::string summary;
};

enum class WorkEventType : std::uint8_t {
    JobCreated,
    JobClaimed,
    JobBlocked,
    JobCompleted,
    JobCancelled,
    ReservationInvalidated,
    WorkOrderTriggered
};

struct WorkEvent {
    WorkEventType type{WorkEventType::JobCreated};
    std::uint64_t sequence{};
    JobId jobId{};
    WorkerStableId workerId{};
    WorkOrderId workOrderId{};
    JobFailureReason reason{JobFailureReason::None};
};

struct DomainWorkCommand {
    std::string commandType;
    std::uint64_t stableTargetId{};
    std::int64_t amount{};
};

struct ExecutionResult {
    bool succeeded{true};
    bool completed{};
    double progressDelta{};
    std::vector<DomainWorkCommand> commands;
    std::string failureDetail;
};

class ReservationLedger {
public:
    bool reserveAtomic(JobId jobId, WorkerStableId workerId, double leaseUntil,
                       const std::vector<ReservationResourceKey>& resources);
    void releaseJob(JobId jobId);
    std::vector<JobId> expire(double now);
    std::optional<ReservationClaim> owner(const ReservationResourceKey& resource) const;
    std::size_t size() const { return claims_.size(); }

private:
    std::map<ReservationResourceKey, ReservationClaim> claims_;
};

enum class WorkOrderQuantityMode : std::uint8_t {
    ProduceN,
    MaintainAtLeast,
    MaintainBetween,
    RepeatIndefinitely
};

enum class WorkOrderFrequency : std::uint8_t {
    OneShot,
    Periodic,
    OnConditionTransition,
    EventTriggered
};

enum class WorkOrderScope : std::uint8_t {
    Global,
    WorkshopGroup,
    NamedWorkshop,
    District,
    Planet,
    RemoteSite
};

enum class WorkOrderConditionKind : std::uint8_t {
    StockAtLeast,
    StockAtMost,
    PowerReserveAtLeast,
    SeasonEquals,
    HazardAtMost,
    SuspicionAtMost,
    InstitutionDemandAtLeast,
    DependencyComplete
};

struct WorkOrderCondition {
    WorkOrderConditionKind kind{WorkOrderConditionKind::StockAtLeast};
    std::string key;
    double value{};
    WorkOrderId dependency{};
};

struct WorkOrder {
    WorkOrderId id{};
    std::string jobTypeId;
    std::string laborId;
    std::string outputStockKey;
    WorkOrderQuantityMode quantityMode{WorkOrderQuantityMode::ProduceN};
    std::uint32_t targetMin{1};
    std::uint32_t targetMax{1};
    std::uint32_t repeatBatch{1};
    WorkOrderFrequency frequency{WorkOrderFrequency::OneShot};
    WorkOrderScope scope{WorkOrderScope::Global};
    std::uint64_t scopeStableId{};
    double periodSeconds{};
    double nextDueTime{};
    std::vector<WorkOrderCondition> conditions;
    std::vector<WorkOrderId> dependencies;
    bool enabled{true};
    bool firedOnce{};
    bool lastConditionValue{};
    bool eventPending{};
    std::uint64_t triggerCount{};
};

struct WorkOrderMetrics {
    std::map<std::string, std::uint32_t> stockCounts;
    std::map<std::string, std::uint32_t> institutionDemand;
    double powerReservePercent{100.0};
    int season{};
    double hazardLevel{};
    double suspicion{};
    std::set<WorkOrderId> completedOrders;
    double now{};
};

struct WorkOrderTrigger {
    WorkOrderId orderId{};
    std::uint32_t quantity{};
};

class AutonomousWorkSystem {
public:
    JobId createJob(JobRecord job);
    const JobRecord* findJob(JobId id) const;
    JobRecord* findJob(JobId id);
    std::vector<JobId> jobIds() const;
    JobSyncResult synchronizeGeneratedJobs(JobOrigin origin, std::vector<GeneratedJobSpec> specs);

    void upsertWorker(WorkerSnapshot worker);
    void removeWorker(WorkerStableId workerId);
    void setWorkDetails(std::vector<WorkDetail> details);

    std::vector<AssignmentDecision> assign(JobSystem& jobs, double now, double leaseSeconds,
                                            const AssignmentContext& context);
    void releaseJob(JobId jobId, JobFailureReason reason = JobFailureReason::None,
                    std::string detail = {});
    void cancelJob(JobId jobId, std::string detail = {});
    std::vector<JobId> expireLeases(double now);
    void invalidateReservation(const ReservationResourceKey& key, std::string detail = {});

    std::vector<DomainWorkCommand> execute(JobId jobId, double dt,
        const std::function<ExecutionResult(const JobRecord&, double)>& executor);

    JobDiagnostic diagnose(JobId id) const;

    WorkOrderId createWorkOrder(WorkOrder order);
    const WorkOrder* findWorkOrder(WorkOrderId id) const;
    bool updateWorkOrderDependencies(WorkOrderId id, std::vector<WorkOrderId> dependencies, std::string* error = nullptr);
    bool validateWorkOrderGraph(std::string* error = nullptr) const;
    bool signalWorkOrderEvent(WorkOrderId id);
    std::vector<WorkOrderTrigger> evaluateWorkOrders(const WorkOrderMetrics& metrics);
    std::vector<JobId> instantiateTriggeredOrders(const std::vector<WorkOrderTrigger>& triggers,
                                                   int priority = 0);

    const ReservationLedger& reservations() const { return reservations_; }
    const std::vector<WorkEvent>& events() const { return events_; }
    void clearEvents() { events_.clear(); }

private:
    struct CandidatePair {
        JobId jobId{};
        WorkerStableId workerId{};
        std::int64_t score{};
        bool workerEligible{};
        bool routeReachable{};
        bool routeSafe{};
    };

    std::map<JobId, JobRecord> jobs_;
    std::map<WorkerStableId, WorkerSnapshot> workers_;
    std::vector<WorkDetail> workDetails_;
    std::map<WorkOrderId, WorkOrder> workOrders_;
    ReservationLedger reservations_;
    std::vector<WorkEvent> events_;
    JobId nextJobId_{1};
    WorkOrderId nextWorkOrderId_{1};
    std::uint64_t nextEventSequence_{1};

    LaborPolicy effectiveLaborPolicy(const WorkerSnapshot& worker, std::string_view laborId,
                                     bool* hasRequiredPolicy) const;
    bool workerEligible(const WorkerSnapshot& worker, const JobRecord& job, LaborPolicy* policyOut = nullptr) const;
    bool dependenciesComplete(const JobRecord& job) const;
    std::int64_t scorePair(const WorkerSnapshot& worker, const JobRecord& job,
                           LaborPolicy policy, float pathCost) const;
    std::vector<ReservationResourceKey> reservationSet(const JobRecord& job) const;
    JobFailureReason staticBlocker(const JobRecord& job, const AssignmentContext& context) const;
    void block(JobRecord& job, JobFailureReason reason, std::string detail = {});
    void emit(WorkEventType type, JobId jobId = 0, WorkerStableId workerId = 0,
              WorkOrderId workOrderId = 0, JobFailureReason reason = JobFailureReason::None);
    bool workOrderConditionsMet(const WorkOrder& order, const WorkOrderMetrics& metrics) const;
    std::uint32_t workOrderQuantity(const WorkOrder& order, const WorkOrderMetrics& metrics) const;
    std::uint32_t outstandingOrderQuantity(WorkOrderId orderId) const;
};

} // namespace elysium::fortress
