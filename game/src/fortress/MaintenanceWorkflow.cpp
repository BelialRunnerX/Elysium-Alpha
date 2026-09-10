#include "fortress/MaintenanceWorkflow.hpp"

namespace elysium::fortress {

WorkflowPlan planMaintenance(std::span<const MachineMaintenanceView> machines,
                             SiteId site,
                             std::uint64_t seed,
                             std::uint64_t tick,
                             std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    for (std::size_t i = 0; i < machines.size(); ++i) {
        const auto& view = machines[i];
        if (!maintenanceDue(view.maintenance) && !view.machine.faulted) continue;
        const auto id = makeDerivedId<JobId>(seed, view.machine.id.value,
                                             0x4D41494E54454E41ULL,
                                             tick + static_cast<std::uint64_t>(i));
        const auto priority = view.machine.faulted ? PriorityBand::Urgent : PriorityBand::Normal;
        auto job = workflowJob(id, "elysium:job/machine_maintenance",
                               ContentId{"elysium:labor/mechanics"}, priority,
                               view.machine.id, view.location,
                               1.0f + 2.0f * (1.0f - view.maintenance.condition));
        plan.commands.push(workflowHeader(tick, producer, sequence++),
                           RequestMaintenanceCommand{view.machine.id, priority});
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{job});
        if (view.machine.faulted) {
            plan.events.push_back(workflowEvent(FortressEventKind::MachineFaulted,
                                                TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                                view.machine.id, site, "Faulted machine generated maintenance job"));
        }
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
