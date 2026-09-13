// Intended function: imported ecs implementation for FortressWorkEcs; preserves the agent-authored subsystem contract for later integration/debugging.
#include "ecs/FortressWorkEcs.hpp"

#include <stdexcept>

namespace elysium::fortress::ecsbridge {
namespace {

template <class Component, class... Args>
void emplaceOrReplace(entt::registry& registry, entt::entity entity, Args&&... args) {
    registry.emplace_or_replace<Component>(entity, std::forward<Args>(args)...);
}

} // namespace

entt::entity FortressWorkEcsAdapter::upsertJob(const JobRecord& job) {
    if (job.id == 0) throw std::invalid_argument("cannot mirror zero JobId into ECS");
    entt::entity entity = runtimeJobEntity(job.id);
    if (entity == entt::null || !registry_->valid(entity)) {
        entity = registry_->create();
        jobs_[job.id] = entity;
    }
    emplaceOrReplace<JobStableIdComponent>(*registry_, entity, job.id);
    emplaceOrReplace<JobTypeRefComponent>(*registry_, entity, job.jobTypeId);
    emplaceOrReplace<JobPriorityComponent>(*registry_, entity, job.explicitPriority);
    emplaceOrReplace<JobOriginComponent>(*registry_, entity, job.origin);
    emplaceOrReplace<JobTargetComponent>(*registry_, entity, job.target);
    emplaceOrReplace<JobRequirementsComponent>(*registry_, entity, job.requirements);
    emplaceOrReplace<JobMaterialFiltersComponent>(*registry_, entity, job.materialFilters);
    emplaceOrReplace<JobReservationsComponent>(*registry_, entity, job.inputReservations, job.toolReservation);
    if (job.workerClaim) emplaceOrReplace<JobWorkerClaimComponent>(*registry_, entity, *job.workerClaim);
    else registry_->remove<JobWorkerClaimComponent>(entity);
    emplaceOrReplace<JobDependenciesComponent>(*registry_, entity, job.dependencies);
    emplaceOrReplace<JobProgressComponent>(*registry_, entity, job.progress);
    emplaceOrReplace<JobInterruptPolicyComponent>(*registry_, entity, job.interruptPolicy);
    emplaceOrReplace<JobFailureComponent>(*registry_, entity, job.failureReason, job.diagnosticDetail);
    emplaceOrReplace<JobHistoryPolicyComponent>(*registry_, entity, job.historyPolicy);
    emplaceOrReplace<JobStateComponent>(*registry_, entity, job.state);
    emplaceOrReplace<JobSourceOrderComponent>(*registry_, entity, job.sourceOrderId, job.quantity, job.sourceStableId, job.sourceOrdinal);
    return entity;
}

entt::entity FortressWorkEcsAdapter::upsertWorkOrder(const WorkOrder& order) {
    if (order.id == 0) throw std::invalid_argument("cannot mirror zero WorkOrderId into ECS");
    entt::entity entity = runtimeWorkOrderEntity(order.id);
    if (entity == entt::null || !registry_->valid(entity)) {
        entity = registry_->create();
        workOrders_[order.id] = entity;
    }
    emplaceOrReplace<WorkOrderStableIdComponent>(*registry_, entity, order.id);
    emplaceOrReplace<WorkOrderDefinitionComponent>(*registry_, entity, order);
    return entity;
}

void FortressWorkEcsAdapter::eraseJob(JobId id) {
    const auto it = jobs_.find(id);
    if (it == jobs_.end()) return;
    if (registry_->valid(it->second)) registry_->destroy(it->second);
    jobs_.erase(it);
}

void FortressWorkEcsAdapter::eraseWorkOrder(WorkOrderId id) {
    const auto it = workOrders_.find(id);
    if (it == workOrders_.end()) return;
    if (registry_->valid(it->second)) registry_->destroy(it->second);
    workOrders_.erase(it);
}

entt::entity FortressWorkEcsAdapter::runtimeJobEntity(JobId id) const {
    const auto it = jobs_.find(id);
    return it == jobs_.end() ? entt::null : it->second;
}

entt::entity FortressWorkEcsAdapter::runtimeWorkOrderEntity(WorkOrderId id) const {
    const auto it = workOrders_.find(id);
    return it == workOrders_.end() ? entt::null : it->second;
}

JobRecord FortressWorkEcsAdapter::snapshotJob(JobId id) const {
    const auto entity = runtimeJobEntity(id);
    if (entity == entt::null || !registry_->valid(entity)) throw std::out_of_range("unknown ECS JobId");
    JobRecord job{};
    job.id = registry_->get<JobStableIdComponent>(entity).value;
    job.jobTypeId = registry_->get<JobTypeRefComponent>(entity).value;
    job.explicitPriority = registry_->get<JobPriorityComponent>(entity).value;
    job.origin = registry_->get<JobOriginComponent>(entity).value;
    job.target = registry_->get<JobTargetComponent>(entity).value;
    job.requirements = registry_->get<JobRequirementsComponent>(entity).value;
    job.materialFilters = registry_->get<JobMaterialFiltersComponent>(entity).value;
    const auto& reservations = registry_->get<JobReservationsComponent>(entity);
    job.inputReservations = reservations.inputs;
    job.toolReservation = reservations.tool;
    if (const auto* claim = registry_->try_get<JobWorkerClaimComponent>(entity)) job.workerClaim = claim->value;
    job.dependencies = registry_->get<JobDependenciesComponent>(entity).value;
    job.progress = registry_->get<JobProgressComponent>(entity).value;
    job.interruptPolicy = registry_->get<JobInterruptPolicyComponent>(entity).value;
    const auto& failure = registry_->get<JobFailureComponent>(entity);
    job.failureReason = failure.reason;
    job.diagnosticDetail = failure.detail;
    job.historyPolicy = registry_->get<JobHistoryPolicyComponent>(entity).value;
    job.state = registry_->get<JobStateComponent>(entity).value;
    const auto& source = registry_->get<JobSourceOrderComponent>(entity);
    job.sourceOrderId = source.value;
    job.quantity = source.quantity;
    job.sourceStableId = source.generatedSourceStableId;
    job.sourceOrdinal = source.generatedSourceOrdinal;
    return job;
}

WorkOrder FortressWorkEcsAdapter::snapshotWorkOrder(WorkOrderId id) const {
    const auto entity = runtimeWorkOrderEntity(id);
    if (entity == entt::null || !registry_->valid(entity)) throw std::out_of_range("unknown ECS WorkOrderId");
    return registry_->get<WorkOrderDefinitionComponent>(entity).value;
}

} // namespace elysium::fortress::ecsbridge
