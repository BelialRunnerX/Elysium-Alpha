#pragma once

#include "fortress/Commands.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace elysium::fortress {

struct WorkflowDiagnostic {
    std::string code;
    std::string message;
    PriorityBand priority{PriorityBand::Normal};
    StableId subject{};
};

struct WorkflowPlan {
    FortressCommandBuffer commands;
    std::vector<FortressEvent> events;
    std::vector<WorkflowDiagnostic> diagnostics;

    [[nodiscard]] bool empty() const noexcept {
        return commands.empty() && events.empty() && diagnostics.empty();
    }
};

inline CommandHeader workflowHeader(std::uint64_t tick, std::uint32_t producer,
                                    std::uint32_t sequence) noexcept {
    return CommandHeader{tick, producer, sequence};
}

inline JobComponent workflowJob(JobId id, std::string type, ContentId labor,
                                PriorityBand priority, StableId origin,
                                SpatialAnchor target, float work = 1.0f) {
    JobComponent job{};
    job.id = id;
    job.type = ContentId{std::move(type)};
    job.priority = priority;
    job.origin = origin;
    job.target = target;
    job.requirements.labor = std::move(labor);
    job.workRequired = work;
    job.state = JobState::Pending;
    return job;
}

inline FortressEvent workflowEvent(FortressEventKind kind, TimeStamp time, StableId primary,
                                   SiteId site, std::string message, float value = 0.0f) {
    FortressEvent event{};
    event.kind = kind;
    event.time = time;
    event.primary = primary;
    event.site = site;
    event.value = value;
    event.message = std::move(message);
    return event;
}

} // namespace elysium::fortress
