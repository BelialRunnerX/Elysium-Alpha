// Intended function: imported ecs implementation for FortressWorkEcs; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "fortress/AutonomousWork.hpp"

#include <entt/entt.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace elysium::fortress::ecsbridge {

// These are deliberately narrow ECS components. Persistent identity is the
// JobId/WorkOrderId value; entt::entity remains a runtime-only lookup handle.
struct JobStableIdComponent { JobId value{}; };
struct JobTypeRefComponent { std::string value; };
struct JobPriorityComponent { int value{}; };
struct JobOriginComponent { JobOrigin value{JobOrigin::Designation}; };
struct JobTargetComponent { JobTarget value{}; };
struct JobRequirementsComponent { JobRequirements value{}; };
struct JobMaterialFiltersComponent { std::set<std::string> value; };
struct JobReservationsComponent {
    std::vector<ReservationResourceKey> inputs;
    std::optional<ReservationResourceKey> tool;
};
struct JobWorkerClaimComponent { WorkerClaim value{}; };
struct JobDependenciesComponent { std::vector<JobId> value; };
struct JobProgressComponent { JobProgress value{}; };
struct JobInterruptPolicyComponent { InterruptPolicy value{InterruptPolicy::HigherPriority}; };
struct JobFailureComponent { JobFailureReason reason{JobFailureReason::None}; std::string detail; };
struct JobHistoryPolicyComponent { HistoryPolicy value{HistoryPolicy::ImportantOnly}; };
struct JobStateComponent { JobState value{JobState::Candidate}; };
struct JobSourceOrderComponent {
    WorkOrderId value{};
    std::uint32_t quantity{1};
    std::uint64_t generatedSourceStableId{};
    std::uint32_t generatedSourceOrdinal{};
};

struct WorkOrderStableIdComponent { WorkOrderId value{}; };
struct WorkOrderDefinitionComponent { WorkOrder value{}; };

class FortressWorkEcsAdapter {
public:
    explicit FortressWorkEcsAdapter(entt::registry& registry) : registry_(&registry) {}

    entt::entity upsertJob(const JobRecord& job);
    entt::entity upsertWorkOrder(const WorkOrder& order);
    void eraseJob(JobId id);
    void eraseWorkOrder(WorkOrderId id);

    entt::entity runtimeJobEntity(JobId id) const;
    entt::entity runtimeWorkOrderEntity(WorkOrderId id) const;

    JobRecord snapshotJob(JobId id) const;
    WorkOrder snapshotWorkOrder(WorkOrderId id) const;

private:
    entt::registry* registry_{};
    std::map<JobId, entt::entity> jobs_;
    std::map<WorkOrderId, entt::entity> workOrders_;
};

} // namespace elysium::fortress::ecsbridge
