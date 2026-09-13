// Intended function: imported fortress implementation for AutonomousWork; preserves the agent-authored subsystem contract for later integration/debugging.
#include "fortress/AutonomousWork.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <tuple>

namespace elysium::fortress {
namespace {

int policyRank(LaborPolicy policy) {
    switch (policy) {
        case LaborPolicy::Forbidden: return 4;
        case LaborPolicy::Required: return 3;
        case LaborPolicy::Preferred: return 2;
        case LaborPolicy::Unrestricted: return 1;
    }
    return 0;
}

bool selectorMatches(const WorkDetailSelector& selector, const WorkerSnapshot& worker) {
    if (selector.empty()) return true;
    if (selector.workerIds.contains(worker.stableId)) return true;
    if (!worker.profession.empty() && selector.professions.contains(worker.profession)) return true;
    for (auto squad : worker.squadIds) if (selector.squadIds.contains(squad)) return true;
    for (auto institution : worker.institutionIds) if (selector.institutionIds.contains(institution)) return true;
    return false;
}

std::string detailFor(JobFailureReason reason) {
    switch (reason) {
        case JobFailureReason::None: return "ready";
        case JobFailureReason::DependencyBlocked: return "waiting for dependency";
        case JobFailureReason::MissingInput: return "missing input or reserved resource";
        case JobFailureReason::NoEligibleWorker: return "no eligible worker";
        case JobFailureReason::UnsafeRoute: return "unsafe route";
        case JobFailureReason::UnreachableRoute: return "unreachable route";
        case JobFailureReason::WorkshopUnavailable: return "workshop unavailable";
        case JobFailureReason::PowerFailure: return "power requirement not met";
        case JobFailureReason::EnvironmentFailure: return "environment requirement not met";
        case JobFailureReason::ReservationConflict: return "resource reservation conflict";
        case JobFailureReason::LeaseExpired: return "worker claim lease expired";
        case JobFailureReason::CancelledByOrigin: return "cancelled by origin";
        case JobFailureReason::ExecutionFailed: return "domain executor failed";
        case JobFailureReason::InvalidDefinition: return "invalid job definition";
    }
    return "unknown";
}

} // namespace

const char* failureReasonName(JobFailureReason reason) {
    switch (reason) {
        case JobFailureReason::None: return "none";
        case JobFailureReason::DependencyBlocked: return "dependency_blocked";
        case JobFailureReason::MissingInput: return "missing_input";
        case JobFailureReason::NoEligibleWorker: return "no_eligible_worker";
        case JobFailureReason::UnsafeRoute: return "unsafe_route";
        case JobFailureReason::UnreachableRoute: return "unreachable_route";
        case JobFailureReason::WorkshopUnavailable: return "workshop_unavailable";
        case JobFailureReason::PowerFailure: return "power_failure";
        case JobFailureReason::EnvironmentFailure: return "environment_failure";
        case JobFailureReason::ReservationConflict: return "reservation_conflict";
        case JobFailureReason::LeaseExpired: return "lease_expired";
        case JobFailureReason::CancelledByOrigin: return "cancelled_by_origin";
        case JobFailureReason::ExecutionFailed: return "execution_failed";
        case JobFailureReason::InvalidDefinition: return "invalid_definition";
    }
    return "unknown";
}

bool WorkDetailSelector::empty() const {
    return workerIds.empty() && professions.empty() && squadIds.empty() && institutionIds.empty();
}

bool ReservationLedger::reserveAtomic(JobId jobId, WorkerStableId workerId, double leaseUntil,
                                      const std::vector<ReservationResourceKey>& resources) {
    std::vector<ReservationResourceKey> unique = resources;
    std::sort(unique.begin(), unique.end());
    unique.erase(std::unique(unique.begin(), unique.end()), unique.end());
    for (const auto& resource : unique) {
        const auto it = claims_.find(resource);
        if (it != claims_.end() && it->second.jobId != jobId) return false;
    }
    for (const auto& resource : unique) claims_[resource] = {jobId, workerId, leaseUntil};
    return true;
}

void ReservationLedger::releaseJob(JobId jobId) {
    for (auto it = claims_.begin(); it != claims_.end();) {
        if (it->second.jobId == jobId) it = claims_.erase(it);
        else ++it;
    }
}

std::vector<JobId> ReservationLedger::expire(double now) {
    std::set<JobId> expired;
    for (auto it = claims_.begin(); it != claims_.end();) {
        if (it->second.leaseUntil <= now) {
            expired.insert(it->second.jobId);
            it = claims_.erase(it);
        } else ++it;
    }
    return {expired.begin(), expired.end()};
}

std::optional<ReservationClaim> ReservationLedger::owner(const ReservationResourceKey& resource) const {
    const auto it = claims_.find(resource);
    if (it == claims_.end()) return std::nullopt;
    return it->second;
}

JobId AutonomousWorkSystem::createJob(JobRecord job) {
    if (job.jobTypeId.empty() || job.requirements.laborId.empty()) {
        throw std::invalid_argument("job requires stable jobTypeId and laborId");
    }
    if (job.id == 0) job.id = nextJobId_++;
    else {
        if (jobs_.contains(job.id)) throw std::invalid_argument("duplicate JobId");
        nextJobId_ = std::max(nextJobId_, job.id + 1);
    }
    if (job.progress.workRequired <= 0.0) job.progress.workRequired = 1.0;
    job.state = JobState::Candidate;
    job.failureReason = JobFailureReason::None;
    const JobId id = job.id;
    jobs_.emplace(id, std::move(job));
    emit(WorkEventType::JobCreated, id);
    return id;
}

const JobRecord* AutonomousWorkSystem::findJob(JobId id) const {
    const auto it = jobs_.find(id);
    return it == jobs_.end() ? nullptr : &it->second;
}

JobRecord* AutonomousWorkSystem::findJob(JobId id) {
    const auto it = jobs_.find(id);
    return it == jobs_.end() ? nullptr : &it->second;
}

std::vector<JobId> AutonomousWorkSystem::jobIds() const {
    std::vector<JobId> ids;
    ids.reserve(jobs_.size());
    for (const auto& [id, _] : jobs_) ids.push_back(id);
    return ids;
}

JobSyncResult AutonomousWorkSystem::synchronizeGeneratedJobs(JobOrigin origin, std::vector<GeneratedJobSpec> specs) {
    // Source systems publish a complete stable snapshot for one origin.  Sorting
    // by source identity makes stable JobId allocation independent of producer
    // traversal order, container iteration order, or worker count.
    std::sort(specs.begin(), specs.end(), [](const GeneratedJobSpec& a, const GeneratedJobSpec& b) {
        if (a.stableSourceId != b.stableSourceId) return a.stableSourceId < b.stableSourceId;
        return a.ordinal < b.ordinal;
    });
    for (std::size_t i = 0; i < specs.size(); ++i) {
        if (specs[i].stableSourceId == 0) throw std::invalid_argument("generated job source requires non-zero StableId");
        if (i > 0 && specs[i - 1].stableSourceId == specs[i].stableSourceId && specs[i - 1].ordinal == specs[i].ordinal)
            throw std::invalid_argument("duplicate generated job source key");
        if (specs[i].job.id != 0) throw std::invalid_argument("generated jobs may not prescribe JobId");
        if (specs[i].job.sourceOrderId != 0) throw std::invalid_argument("generated source jobs may not masquerade as work-order jobs");
    }

    std::map<std::pair<std::uint64_t, std::uint32_t>, JobId> existing;
    for (const auto& [id, job] : jobs_) {
        if (job.origin == origin && job.sourceStableId != 0 && job.state != JobState::Cancelled)
            existing[{job.sourceStableId, job.sourceOrdinal}] = id;
    }

    JobSyncResult result;
    std::set<std::pair<std::uint64_t, std::uint32_t>> present;
    for (auto& spec : specs) {
        const auto key = std::make_pair(spec.stableSourceId, spec.ordinal);
        present.insert(key);
        const auto found = existing.find(key);
        if (found == existing.end()) {
            spec.job.origin = origin;
            spec.job.sourceStableId = spec.stableSourceId;
            spec.job.sourceOrdinal = spec.ordinal;
            result.created.push_back(createJob(std::move(spec.job)));
            continue;
        }

        auto& current = jobs_.at(found->second);
        if (current.state == JobState::Completed || current.state == JobState::Cancelled || current.state == JobState::Failed)
            continue;

        // Never rewrite a claimed job's target/reservations underneath its
        // lease.  The source can cancel it by omitting the key; otherwise the
        // next source snapshot is applied after the claim is released.
        if (current.workerClaim) continue;

        const JobId id = current.id;
        const JobState state = current.state;
        const JobProgress progress = current.progress;
        spec.job.id = id;
        spec.job.origin = origin;
        spec.job.sourceStableId = spec.stableSourceId;
        spec.job.sourceOrdinal = spec.ordinal;
        spec.job.state = state;
        spec.job.progress = progress;
        spec.job.workerClaim.reset();
        if (state == JobState::Blocked) {
            // A changed source definition must be re-evaluated rather than
            // preserving a blocker that may no longer apply.
            spec.job.state = JobState::Candidate;
            spec.job.failureReason = JobFailureReason::None;
            spec.job.diagnosticDetail.clear();
        }
        current = std::move(spec.job);
        result.updated.push_back(id);
    }

    for (const auto& [key, id] : existing) {
        if (present.contains(key)) continue;
        auto& job = jobs_.at(id);
        if (job.state == JobState::Completed || job.state == JobState::Cancelled || job.state == JobState::Failed) continue;
        cancelJob(id, "source intent withdrawn");
        result.cancelled.push_back(id);
    }
    return result;
}

void AutonomousWorkSystem::upsertWorker(WorkerSnapshot worker) {
    if (worker.stableId == 0) throw std::invalid_argument("worker requires non-zero StableId");
    workers_[worker.stableId] = std::move(worker);
}

void AutonomousWorkSystem::removeWorker(WorkerStableId workerId) {
    workers_.erase(workerId);
    for (auto& [id, job] : jobs_) {
        if (job.workerClaim && job.workerClaim->workerId == workerId &&
            job.state != JobState::Completed && job.state != JobState::Cancelled && job.state != JobState::Failed) {
            reservations_.releaseJob(id);
            job.workerClaim.reset();
            block(job, JobFailureReason::NoEligibleWorker, "claimed worker left simulation domain");
        }
    }
}

void AutonomousWorkSystem::setWorkDetails(std::vector<WorkDetail> details) {
    std::sort(details.begin(), details.end(), [](const WorkDetail& a, const WorkDetail& b) { return a.id < b.id; });
    workDetails_ = std::move(details);
}

LaborPolicy AutonomousWorkSystem::effectiveLaborPolicy(const WorkerSnapshot& worker, std::string_view laborId,
                                                       bool* hasRequiredPolicy) const {
    LaborPolicy resolved = LaborPolicy::Unrestricted;
    bool required = false;
    for (const auto& detail : workDetails_) {
        if (!selectorMatches(detail.selector, worker)) continue;
        for (const auto& [labor, policy] : detail.laborPolicies) {
            if (policy == LaborPolicy::Required) required = true;
            if (labor == laborId && policyRank(policy) > policyRank(resolved)) resolved = policy;
        }
    }
    if (hasRequiredPolicy) *hasRequiredPolicy = required;
    return resolved;
}

bool AutonomousWorkSystem::workerEligible(const WorkerSnapshot& worker, const JobRecord& job, LaborPolicy* policyOut) const {
    if (!worker.available) return false;
    if (job.requirements.requiresCitizen && worker.citizenship != CitizenshipState::Citizen) return false;
    if (job.requirements.requiresHealthy && !worker.healthy) return false;
    if (job.requirements.requiresOnSchedule && !worker.onSchedule) return false;
    for (const auto& capability : job.requirements.requiredCapabilities)
        if (!worker.capabilities.contains(capability)) return false;
    for (const auto& equipment : job.requirements.requiredEquipment)
        if (!worker.equipment.contains(equipment)) return false;
    for (const auto& access : job.requirements.requiredAccess)
        if (!worker.access.contains(access)) return false;
    for (const auto& [skill, minimum] : job.requirements.minimumSkill) {
        const auto it = worker.skills.find(skill);
        if (it == worker.skills.end() || it->second < minimum) return false;
    }

    bool hasRequired = false;
    const LaborPolicy policy = effectiveLaborPolicy(worker, job.requirements.laborId, &hasRequired);
    if (policyOut) *policyOut = policy;
    if (policy == LaborPolicy::Forbidden) return false;
    if (hasRequired && policy != LaborPolicy::Required) return false;
    return true;
}

bool AutonomousWorkSystem::dependenciesComplete(const JobRecord& job) const {
    for (JobId dependency : job.dependencies) {
        const auto it = jobs_.find(dependency);
        if (it == jobs_.end() || it->second.state != JobState::Completed) return false;
    }
    return true;
}

std::int64_t AutonomousWorkSystem::scorePair(const WorkerSnapshot& worker, const JobRecord& job,
                                             LaborPolicy policy, float pathCost) const {
    // Integer lexicographic bands avoid floating-point tie ambiguity. Stable IDs
    // are applied separately as the final ordering key.
    std::int64_t score = 0;
    if (job.origin == JobOrigin::Emergency) score += 1'000'000'000LL;
    score += static_cast<std::int64_t>(job.explicitPriority) * 1'000'000LL;
    score += static_cast<std::int64_t>(job.dependencies.size()) * 10'000LL;
    if (policy == LaborPolicy::Required) score += 8'000LL;
    else if (policy == LaborPolicy::Preferred) score += 4'000LL;
    const auto pref = worker.workPreferences.find(job.requirements.laborId);
    if (pref != worker.workPreferences.end()) score += static_cast<std::int64_t>(std::llround(pref->second * 100.0f));
    score += static_cast<std::int64_t>(std::llround(std::max(0.0f, worker.throughput) * 100.0f));
    score -= static_cast<std::int64_t>(std::llround(std::max(0.0f, pathCost) * 10.0f));
    return score;
}

std::vector<ReservationResourceKey> AutonomousWorkSystem::reservationSet(const JobRecord& job) const {
    std::vector<ReservationResourceKey> result = job.requirements.reservations;
    result.insert(result.end(), job.inputReservations.begin(), job.inputReservations.end());
    if (job.toolReservation) result.push_back(*job.toolReservation);
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

JobFailureReason AutonomousWorkSystem::staticBlocker(const JobRecord& job, const AssignmentContext& context) const {
    if (!dependenciesComplete(job)) return JobFailureReason::DependencyBlocked;
    if (context.workshopReady && !context.workshopReady(job)) return JobFailureReason::WorkshopUnavailable;
    if (context.powerReady && !context.powerReady(job)) return JobFailureReason::PowerFailure;
    if (context.environmentReady && !context.environmentReady(job)) return JobFailureReason::EnvironmentFailure;
    if (context.resourceExists) {
        for (const auto& resource : reservationSet(job))
            if (!context.resourceExists(resource)) return JobFailureReason::MissingInput;
    }
    return JobFailureReason::None;
}

void AutonomousWorkSystem::block(JobRecord& job, JobFailureReason reason, std::string detail) {
    const bool changed = job.state != JobState::Blocked || job.failureReason != reason || job.diagnosticDetail != detail;
    job.state = JobState::Blocked;
    job.failureReason = reason;
    job.diagnosticDetail = std::move(detail);
    if (changed) emit(WorkEventType::JobBlocked, job.id, job.workerClaim ? job.workerClaim->workerId : 0, 0, reason);
}

std::vector<AssignmentDecision> AutonomousWorkSystem::assign(JobSystem& workerPool, double now, double leaseSeconds,
                                                             const AssignmentContext& context) {
    if (leaseSeconds <= 0.0) throw std::invalid_argument("assignment lease must be positive");
    expireLeases(now);

    std::vector<JobId> candidateJobs;
    for (auto& [id, job] : jobs_) {
        if (job.state == JobState::Completed || job.state == JobState::Cancelled || job.state == JobState::Failed ||
            job.workerClaim) continue;
        const auto blocker = staticBlocker(job, context);
        if (blocker != JobFailureReason::None) {
            block(job, blocker);
            continue;
        }
        job.state = JobState::Pending;
        job.failureReason = JobFailureReason::None;
        job.diagnosticDetail.clear();
        candidateJobs.push_back(id);
    }

    std::vector<WorkerStableId> workerIds;
    workerIds.reserve(workers_.size());
    for (const auto& [id, worker] : workers_) if (worker.available) workerIds.push_back(id);

    struct PairSlot {
        CandidatePair pair;
        LaborPolicy policy{LaborPolicy::Unrestricted};
    };
    std::vector<PairSlot> pairs(candidateJobs.size() * workerIds.size());
    workerPool.parallelFor(pairs.size(), [&](std::size_t index) {
        const std::size_t wi = workerIds.empty() ? 0 : index % workerIds.size();
        const std::size_t ji = workerIds.empty() ? 0 : index / workerIds.size();
        if (workerIds.empty()) return;
        const auto jobId = candidateJobs[ji];
        const auto workerId = workerIds[wi];
        const auto& job = jobs_.at(jobId);
        const auto& worker = workers_.at(workerId);
        LaborPolicy policy = LaborPolicy::Unrestricted;
        const bool eligible = workerEligible(worker, job, &policy);
        PathAssessment path{};
        if (eligible && context.path) path = context.path(worker, job);
        pairs[index].policy = policy;
        pairs[index].pair = {jobId, workerId,
            eligible && path.reachable && path.safe ? scorePair(worker, job, policy, path.cost) : std::numeric_limits<std::int64_t>::min(),
            eligible, path.reachable, path.safe};
    }, 16);

    std::sort(pairs.begin(), pairs.end(), [](const PairSlot& a, const PairSlot& b) {
        if (a.pair.score != b.pair.score) return a.pair.score > b.pair.score;
        if (a.pair.jobId != b.pair.jobId) return a.pair.jobId < b.pair.jobId;
        return a.pair.workerId < b.pair.workerId;
    });

    std::set<JobId> assignedJobs;
    std::set<WorkerStableId> assignedWorkers;
    std::vector<AssignmentDecision> decisions;
    for (const auto& slot : pairs) {
        const auto& pair = slot.pair;
        if (pair.score == std::numeric_limits<std::int64_t>::min()) continue;
        if (assignedJobs.contains(pair.jobId) || assignedWorkers.contains(pair.workerId)) continue;
        auto& job = jobs_.at(pair.jobId);
        if (job.workerClaim) continue;
        const auto resources = reservationSet(job);
        if (!reservations_.reserveAtomic(pair.jobId, pair.workerId, now + leaseSeconds, resources)) continue;
        job.workerClaim = WorkerClaim{pair.workerId, now + leaseSeconds};
        job.state = JobState::Claimed;
        job.failureReason = JobFailureReason::None;
        job.diagnosticDetail.clear();
        assignedJobs.insert(pair.jobId);
        assignedWorkers.insert(pair.workerId);
        decisions.push_back({pair.jobId, pair.workerId, pair.score});
        emit(WorkEventType::JobClaimed, pair.jobId, pair.workerId);
    }

    // Jobs left without a claim receive the most specific diagnosable reason.
    for (JobId id : candidateJobs) {
        auto& job = jobs_.at(id);
        if (job.workerClaim) continue;
        bool sawEligible = false;
        bool sawReachable = false;
        bool sawSafe = false;
        for (const auto& slot : pairs) {
            if (slot.pair.jobId != id) continue;
            sawEligible = sawEligible || slot.pair.workerEligible;
            if (slot.pair.workerEligible) sawReachable = sawReachable || slot.pair.routeReachable;
            if (slot.pair.workerEligible && slot.pair.routeReachable) sawSafe = sawSafe || slot.pair.routeSafe;
        }
        if (!sawEligible) block(job, JobFailureReason::NoEligibleWorker);
        else if (!sawReachable) block(job, JobFailureReason::UnreachableRoute);
        else if (!sawSafe) block(job, JobFailureReason::UnsafeRoute);
        else block(job, JobFailureReason::ReservationConflict);
    }

    std::sort(decisions.begin(), decisions.end(), [](const AssignmentDecision& a, const AssignmentDecision& b) {
        return a.jobId < b.jobId;
    });
    return decisions;
}

void AutonomousWorkSystem::releaseJob(JobId jobId, JobFailureReason reason, std::string detail) {
    auto* job = findJob(jobId);
    if (!job) return;
    reservations_.releaseJob(jobId);
    job->workerClaim.reset();
    if (reason == JobFailureReason::None) {
        job->state = JobState::Pending;
        job->failureReason = JobFailureReason::None;
        job->diagnosticDetail.clear();
    } else {
        block(*job, reason, std::move(detail));
    }
}

void AutonomousWorkSystem::cancelJob(JobId jobId, std::string detail) {
    auto* job = findJob(jobId);
    if (!job) return;
    reservations_.releaseJob(jobId);
    const auto worker = job->workerClaim ? job->workerClaim->workerId : 0;
    job->workerClaim.reset();
    job->state = JobState::Cancelled;
    job->failureReason = JobFailureReason::CancelledByOrigin;
    job->diagnosticDetail = std::move(detail);
    emit(WorkEventType::JobCancelled, jobId, worker, 0, JobFailureReason::CancelledByOrigin);
}

std::vector<JobId> AutonomousWorkSystem::expireLeases(double now) {
    std::set<JobId> expired;
    for (const auto& [id, job] : jobs_) {
        if (job.workerClaim && job.workerClaim->leaseUntil <= now) expired.insert(id);
    }
    for (JobId id : reservations_.expire(now)) expired.insert(id);
    for (JobId id : expired) {
        auto* job = findJob(id);
        if (!job || job->state == JobState::Completed || job->state == JobState::Cancelled) continue;
        job->workerClaim.reset();
        block(*job, JobFailureReason::LeaseExpired);
    }
    return {expired.begin(), expired.end()};
}

void AutonomousWorkSystem::invalidateReservation(const ReservationResourceKey& key, std::string detail) {
    const auto claim = reservations_.owner(key);
    if (!claim) return;
    reservations_.releaseJob(claim->jobId);
    auto* job = findJob(claim->jobId);
    if (job) {
        job->workerClaim.reset();
        block(*job, JobFailureReason::MissingInput, std::move(detail));
    }
    emit(WorkEventType::ReservationInvalidated, claim->jobId, claim->workerId, 0, JobFailureReason::MissingInput);
}

std::vector<DomainWorkCommand> AutonomousWorkSystem::execute(JobId jobId, double dt,
    const std::function<ExecutionResult(const JobRecord&, double)>& executor) {
    if (dt < 0.0) throw std::invalid_argument("job execution dt cannot be negative");
    auto* job = findJob(jobId);
    if (!job || !job->workerClaim || !executor) return {};
    if (job->state != JobState::Claimed && job->state != JobState::Navigating && job->state != JobState::Working) return {};

    job->state = JobState::Working;
    const ExecutionResult result = executor(*job, dt);
    if (!result.succeeded) {
        reservations_.releaseJob(jobId);
        const auto worker = job->workerClaim->workerId;
        job->workerClaim.reset();
        job->state = JobState::Failed;
        job->failureReason = JobFailureReason::ExecutionFailed;
        job->diagnosticDetail = result.failureDetail;
        emit(WorkEventType::JobBlocked, jobId, worker, 0, JobFailureReason::ExecutionFailed);
        return result.commands;
    }

    job->progress.workDone = std::max(0.0, job->progress.workDone + result.progressDelta);
    if (result.completed || job->progress.workDone >= job->progress.workRequired) {
        const auto worker = job->workerClaim->workerId;
        job->progress.workDone = std::max(job->progress.workDone, job->progress.workRequired);
        job->state = JobState::Completed;
        job->failureReason = JobFailureReason::None;
        job->diagnosticDetail.clear();
        job->workerClaim.reset();
        reservations_.releaseJob(jobId);
        emit(WorkEventType::JobCompleted, jobId, worker, job->sourceOrderId);
    }
    return result.commands;
}

JobDiagnostic AutonomousWorkSystem::diagnose(JobId id) const {
    const auto* job = findJob(id);
    if (!job) return {id, JobState::Failed, JobFailureReason::InvalidDefinition, "unknown job"};
    std::ostringstream out;
    out << failureReasonName(job->failureReason) << ": "
        << (job->diagnosticDetail.empty() ? detailFor(job->failureReason) : job->diagnosticDetail);
    if (job->workerClaim) out << " [worker=" << job->workerClaim->workerId << "]";
    return {id, job->state, job->failureReason, out.str()};
}

void AutonomousWorkSystem::emit(WorkEventType type, JobId jobId, WorkerStableId workerId,
                                WorkOrderId workOrderId, JobFailureReason reason) {
    events_.push_back({type, nextEventSequence_++, jobId, workerId, workOrderId, reason});
}

const WorkOrder* AutonomousWorkSystem::findWorkOrder(WorkOrderId id) const {
    const auto it = workOrders_.find(id);
    return it == workOrders_.end() ? nullptr : &it->second;
}

WorkOrderId AutonomousWorkSystem::createWorkOrder(WorkOrder order) {
    if (order.jobTypeId.empty() || order.laborId.empty()) throw std::invalid_argument("work order requires jobTypeId and laborId");
    if (order.id == 0) order.id = nextWorkOrderId_++;
    else {
        if (workOrders_.contains(order.id)) throw std::invalid_argument("duplicate WorkOrderId");
        nextWorkOrderId_ = std::max(nextWorkOrderId_, order.id + 1);
    }
    if (order.quantityMode == WorkOrderQuantityMode::MaintainBetween && order.targetMax < order.targetMin)
        throw std::invalid_argument("work order maintain-between max is below min");
    if (order.repeatBatch == 0) order.repeatBatch = 1;
    const auto id = order.id;
    workOrders_.emplace(id, std::move(order));
    std::string error;
    if (!validateWorkOrderGraph(&error)) {
        workOrders_.erase(id);
        throw std::invalid_argument(error);
    }
    return id;
}

bool AutonomousWorkSystem::updateWorkOrderDependencies(WorkOrderId id, std::vector<WorkOrderId> dependencies, std::string* error) {
    const auto it = workOrders_.find(id);
    if (it == workOrders_.end()) {
        if (error) *error = "unknown work order " + std::to_string(id);
        return false;
    }
    const auto old = it->second.dependencies;
    it->second.dependencies = std::move(dependencies);
    std::string local;
    if (!validateWorkOrderGraph(&local)) {
        it->second.dependencies = old;
        if (error) *error = local;
        return false;
    }
    return true;
}

bool AutonomousWorkSystem::validateWorkOrderGraph(std::string* error) const {
    enum class Mark : std::uint8_t { Unseen, Visiting, Done };
    std::map<WorkOrderId, Mark> marks;
    std::function<bool(WorkOrderId)> visit = [&](WorkOrderId id) {
        const auto it = workOrders_.find(id);
        if (it == workOrders_.end()) {
            if (error) *error = "work order dependency references missing order " + std::to_string(id);
            return false;
        }
        auto& mark = marks[id];
        if (mark == Mark::Visiting) {
            if (error) *error = "work order dependency cycle at order " + std::to_string(id);
            return false;
        }
        if (mark == Mark::Done) return true;
        mark = Mark::Visiting;
        std::vector<WorkOrderId> deps = it->second.dependencies;
        for (const auto& condition : it->second.conditions)
            if (condition.kind == WorkOrderConditionKind::DependencyComplete && condition.dependency != 0)
                deps.push_back(condition.dependency);
        std::sort(deps.begin(), deps.end());
        deps.erase(std::unique(deps.begin(), deps.end()), deps.end());
        for (auto dep : deps) if (!visit(dep)) return false;
        mark = Mark::Done;
        return true;
    };
    for (const auto& [id, _] : workOrders_) if (!visit(id)) return false;
    return true;
}

bool AutonomousWorkSystem::signalWorkOrderEvent(WorkOrderId id) {
    const auto it = workOrders_.find(id);
    if (it == workOrders_.end()) return false;
    it->second.eventPending = true;
    return true;
}

bool AutonomousWorkSystem::workOrderConditionsMet(const WorkOrder& order, const WorkOrderMetrics& metrics) const {
    for (auto dependency : order.dependencies) if (!metrics.completedOrders.contains(dependency)) return false;
    for (const auto& condition : order.conditions) {
        switch (condition.kind) {
            case WorkOrderConditionKind::StockAtLeast: {
                const auto it = metrics.stockCounts.find(condition.key);
                const auto value = it == metrics.stockCounts.end() ? 0U : it->second;
                if (static_cast<double>(value) < condition.value) return false;
                break;
            }
            case WorkOrderConditionKind::StockAtMost: {
                const auto it = metrics.stockCounts.find(condition.key);
                const auto value = it == metrics.stockCounts.end() ? 0U : it->second;
                if (static_cast<double>(value) > condition.value) return false;
                break;
            }
            case WorkOrderConditionKind::PowerReserveAtLeast:
                if (metrics.powerReservePercent < condition.value) return false;
                break;
            case WorkOrderConditionKind::SeasonEquals:
                if (metrics.season != static_cast<int>(condition.value)) return false;
                break;
            case WorkOrderConditionKind::HazardAtMost:
                if (metrics.hazardLevel > condition.value) return false;
                break;
            case WorkOrderConditionKind::SuspicionAtMost:
                if (metrics.suspicion > condition.value) return false;
                break;
            case WorkOrderConditionKind::InstitutionDemandAtLeast: {
                const auto it = metrics.institutionDemand.find(condition.key);
                const auto value = it == metrics.institutionDemand.end() ? 0U : it->second;
                if (static_cast<double>(value) < condition.value) return false;
                break;
            }
            case WorkOrderConditionKind::DependencyComplete:
                if (!metrics.completedOrders.contains(condition.dependency)) return false;
                break;
        }
    }
    return true;
}

std::uint32_t AutonomousWorkSystem::outstandingOrderQuantity(WorkOrderId orderId) const {
    std::uint64_t total = 0;
    for (const auto& [_, job] : jobs_) {
        if (job.sourceOrderId != orderId) continue;
        if (job.state == JobState::Completed || job.state == JobState::Cancelled || job.state == JobState::Failed) continue;
        total += job.quantity;
        if (total >= std::numeric_limits<std::uint32_t>::max()) return std::numeric_limits<std::uint32_t>::max();
    }
    return static_cast<std::uint32_t>(total);
}

std::uint32_t AutonomousWorkSystem::workOrderQuantity(const WorkOrder& order, const WorkOrderMetrics& metrics) const {
    const std::string& stockKey = order.outputStockKey.empty() ? order.jobTypeId : order.outputStockKey;
    const auto it = metrics.stockCounts.find(stockKey);
    const std::uint32_t stock = it == metrics.stockCounts.end() ? 0U : it->second;
    if (order.quantityMode == WorkOrderQuantityMode::RepeatIndefinitely) return order.repeatBatch;

    // In-flight order jobs count toward the requested target.  Inventory is
    // updated by the domain commit later, so ignoring outstanding work would
    // let a periodic maintain-order duplicate the same deficit every cadence.
    const std::uint64_t effective64 = static_cast<std::uint64_t>(stock) + outstandingOrderQuantity(order.id);
    const std::uint32_t current = effective64 >= std::numeric_limits<std::uint32_t>::max()
        ? std::numeric_limits<std::uint32_t>::max() : static_cast<std::uint32_t>(effective64);
    switch (order.quantityMode) {
        case WorkOrderQuantityMode::ProduceN:
            return current >= order.targetMax ? 0U : order.targetMax - current;
        case WorkOrderQuantityMode::MaintainAtLeast:
            return current >= order.targetMin ? 0U : order.targetMin - current;
        case WorkOrderQuantityMode::MaintainBetween:
            return current >= order.targetMin ? 0U : order.targetMax - current;
        case WorkOrderQuantityMode::RepeatIndefinitely:
            return order.repeatBatch;
    }
    return 0;
}

std::vector<WorkOrderTrigger> AutonomousWorkSystem::evaluateWorkOrders(const WorkOrderMetrics& metrics) {
    std::vector<WorkOrderTrigger> triggers;
    for (auto& [id, order] : workOrders_) {
        if (!order.enabled) continue;
        const bool conditions = workOrderConditionsMet(order, metrics);
        bool frequencyAllows = false;
        switch (order.frequency) {
            case WorkOrderFrequency::OneShot:
                frequencyAllows = !order.firedOnce;
                break;
            case WorkOrderFrequency::Periodic:
                frequencyAllows = metrics.now >= order.nextDueTime;
                break;
            case WorkOrderFrequency::OnConditionTransition:
                frequencyAllows = conditions && !order.lastConditionValue;
                break;
            case WorkOrderFrequency::EventTriggered:
                frequencyAllows = order.eventPending;
                break;
        }

        const std::uint32_t quantity = conditions && frequencyAllows ? workOrderQuantity(order, metrics) : 0U;
        if (quantity > 0) {
            triggers.push_back({id, quantity});
            ++order.triggerCount;
            order.firedOnce = true;
            if (order.frequency == WorkOrderFrequency::Periodic) order.nextDueTime = metrics.now + std::max(0.0, order.periodSeconds);
            if (order.frequency == WorkOrderFrequency::EventTriggered) order.eventPending = false;
            emit(WorkEventType::WorkOrderTriggered, 0, 0, id);
        } else if (order.frequency == WorkOrderFrequency::EventTriggered && frequencyAllows) {
            order.eventPending = false;
        }
        order.lastConditionValue = conditions;
    }
    return triggers;
}

std::vector<JobId> AutonomousWorkSystem::instantiateTriggeredOrders(const std::vector<WorkOrderTrigger>& triggers,
                                                                    int priority) {
    std::vector<JobId> result;
    for (const auto& trigger : triggers) {
        const auto it = workOrders_.find(trigger.orderId);
        if (it == workOrders_.end()) continue;
        JobRecord job{};
        job.jobTypeId = it->second.jobTypeId;
        job.explicitPriority = priority;
        job.origin = JobOrigin::WorkOrder;
        job.requirements.laborId = it->second.laborId;
        job.sourceOrderId = trigger.orderId;
        job.quantity = trigger.quantity;
        job.progress.workRequired = static_cast<double>(std::max<std::uint32_t>(1, trigger.quantity));
        result.push_back(createJob(std::move(job)));
    }
    return result;
}

} // namespace elysium::fortress
