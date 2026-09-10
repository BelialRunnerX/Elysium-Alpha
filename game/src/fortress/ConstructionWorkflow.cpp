#include "fortress/ConstructionWorkflow.hpp"

#include <algorithm>

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kStageLabel = 0x434F4E5354414745ULL;

ContentId laborForDesignation(DesignationKind kind) {
    switch (kind) {
        case DesignationKind::Mine:
        case DesignationKind::Bore: return ContentId{"elysium:labor/mining"};
        case DesignationKind::Channel: return ContentId{"elysium:labor/channeling"};
        case DesignationKind::Smooth:
        case DesignationKind::Polish:
        case DesignationKind::Engrave: return ContentId{"elysium:labor/micro_detail"};
        case DesignationKind::Seal: return ContentId{"elysium:labor/sealing"};
        case DesignationKind::Reinforce: return ContentId{"elysium:labor/reinforcement"};
        case DesignationKind::Repair:
        case DesignationKind::Replace: return ContentId{"elysium:labor/repair_structure"};
        case DesignationKind::Deconstruct: return ContentId{"elysium:labor/deconstruct"};
        case DesignationKind::UtilityRoute: return ContentId{"elysium:labor/utility_install"};
        default: return ContentId{"elysium:labor/masonry"};
    }
}
}

WorkflowPlan planConstructionWorkflow(const ConstructionBlueprint& blueprint,
                                      const ConstructionStageState& state,
                                      std::span<const ConstructionMaterialSource> materials,
                                      std::uint64_t seed,
                                      std::uint64_t tick,
                                      std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    if (state.supportSafety < 0.35f) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "unsafe_support", "Blueprint support preview is below safe construction threshold",
            PriorityBand::High, state.owner});
    }
    if (state.utilityAccess < 0.25f && !blueprint.utilityPorts.empty()) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "utility_unreachable", "Blueprint requires utility access not present at target",
            PriorityBand::High, state.owner});
    }

    auto designations = expandBlueprint(blueprint, state.origin, state.owner, seed, tick);
    for (std::size_t i = 0; i < designations.size(); ++i) {
        auto designationCommand = designations[i];
        plan.commands.push(workflowHeader(tick, producer, sequence++), designationCommand);
        const auto& designation = designationCommand.designation;
        const auto jobId = makeDerivedId<JobId>(seed, designation.id.value, kStageLabel,
                                                tick + static_cast<std::uint64_t>(i));
        auto build = workflowJob(jobId, "elysium:job/construction_stage",
                                 laborForDesignation(designation.kind), designation.priority,
                                 state.owner, designation.extent.min, 1.0f);
        if (!designation.materialPolicy.value.empty()) {
            build.materialFilters.push_back(designation.materialPolicy);
            const auto it = std::find_if(materials.begin(), materials.end(), [&](const auto& source) {
                return source.material == designation.materialPolicy && source.accessible && source.quantity > 0.0f;
            });
            if (it == materials.end()) {
                build.state = JobState::Blocked;
                build.failureReason = "construction material unavailable";
                plan.diagnostics.push_back(WorkflowDiagnostic{
                    "construction_material_missing", "No accessible material satisfies blueprint step",
                    designation.priority, state.owner});
            } else {
                const auto haulId = makeDerivedId<JobId>(seed, it->item.value, kStageLabel,
                                                         tick + 1000 + static_cast<std::uint64_t>(i));
                auto haul = workflowJob(haulId, "elysium:job/stage_construction_material",
                                        ContentId{"elysium:labor/haul_construction"}, designation.priority,
                                        state.owner, designation.extent.min, 0.5f);
                ReservationClaim claim{};
                claim.kind = ReservationKind::Item;
                claim.resource = it->item;
                claim.owner = state.owner;
                claim.job = haulId;
                claim.quantity = std::min(1.0f, it->quantity);
                claim.expiresTick = static_cast<std::int64_t>(tick + 1800);
                haul.reservations.push_back(claim);
                plan.commands.push(workflowHeader(tick, producer, sequence++), ReserveCommand{claim});
                plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{haul});
                build.dependencies.push_back(haulId);
            }
        }
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{build});
    }

    if (!state.commissioned && state.supportSafety >= 0.35f && state.utilityAccess >= 0.25f) {
        const auto commissionId = makeDerivedId<JobId>(seed, blueprint.id.value, kStageLabel, tick + 5000);
        auto commission = workflowJob(commissionId, "elysium:job/commission_structure",
                                      ContentId{"elysium:labor/machine_install"}, PriorityBand::High,
                                      state.owner, state.origin, 1.5f);
        for (const auto& command : plan.commands.commands()) {
            if (const auto* create = std::get_if<CreateJobCommand>(&command.payload)) {
                if (create->job.type.value == "elysium:job/construction_stage") {
                    commission.dependencies.push_back(create->job.id);
                }
            }
        }
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{commission});
    }
    plan.events.push_back(workflowEvent(FortressEventKind::JobCreated,
                                        TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                        state.owner, state.site, "Blueprint expanded into staged construction workflow",
                                        static_cast<float>(designations.size())));
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
