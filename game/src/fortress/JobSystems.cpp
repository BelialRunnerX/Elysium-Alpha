#include "fortress/Systems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {
namespace {

float skillRank(const Skills* skills, std::string_view id) {
    if (!skills) return 0.0f;
    for (const auto& skill : skills->entries) {
        if (skill.skill.value == id) return skill.effectiveRank;
    }
    return 0.0f;
}

LaborPolicy laborPolicy(const WorkDetails* details, std::string_view id) {
    if (!details) return LaborPolicy::Unrestricted;
    for (const auto& entry : details->entries) {
        if (entry.labor.value == id) return entry.policy;
    }
    return LaborPolicy::Unrestricted;
}

float priorityWeight(PriorityBand priority) {
    switch (priority) {
        case PriorityBand::Background: return 0.0f;
        case PriorityBand::Low: return 12.0f;
        case PriorityBand::Normal: return 28.0f;
        case PriorityBand::High: return 52.0f;
        case PriorityBand::Urgent: return 78.0f;
        case PriorityBand::Emergency: return 120.0f;
    }
    return 0.0f;
}

std::uint64_t designationLabel(DesignationKind kind) {
    return 0xD3516000ULL + static_cast<std::uint64_t>(kind);
}

ContentId jobTypeFor(DesignationKind kind) {
    switch (kind) {
        case DesignationKind::Mine: return {"elysium:job/mine"};
        case DesignationKind::Channel: return {"elysium:job/channel"};
        case DesignationKind::Ramp: return {"elysium:job/ramp"};
        case DesignationKind::Stair: return {"elysium:job/stair"};
        case DesignationKind::Bore: return {"elysium:job/bore"};
        case DesignationKind::Smooth: return {"elysium:job/smooth"};
        case DesignationKind::Polish: return {"elysium:job/polish"};
        case DesignationKind::Engrave: return {"elysium:job/engrave"};
        case DesignationKind::Coat: return {"elysium:job/coat"};
        case DesignationKind::Seal: return {"elysium:job/seal"};
        case DesignationKind::Decontaminate: return {"elysium:job/decontaminate"};
        case DesignationKind::Build: return {"elysium:job/build"};
        case DesignationKind::Reinforce: return {"elysium:job/reinforce"};
        case DesignationKind::Repair: return {"elysium:job/repair"};
        case DesignationKind::Replace: return {"elysium:job/replace"};
        case DesignationKind::Deconstruct: return {"elysium:job/deconstruct"};
        case DesignationKind::Gather: return {"elysium:job/gather"};
        case DesignationKind::Salvage: return {"elysium:job/salvage"};
        case DesignationKind::Hunt: return {"elysium:job/hunt"};
        case DesignationKind::Dump: return {"elysium:job/dump"};
        case DesignationKind::Forbid: return {"elysium:job/forbid"};
        case DesignationKind::Quarantine: return {"elysium:job/quarantine"};
        case DesignationKind::UtilityRoute: return {"elysium:job/utility_install"};
        case DesignationKind::Traffic: return {"elysium:job/traffic_policy"};
        case DesignationKind::Evacuate: return {"elysium:job/evacuate"};
    }
    return {"elysium:job/unknown"};
}

ContentId laborFor(DesignationKind kind) {
    switch (kind) {
        case DesignationKind::Mine:
        case DesignationKind::Channel:
        case DesignationKind::Ramp:
        case DesignationKind::Stair:
        case DesignationKind::Bore: return {"elysium:labor/mining"};
        case DesignationKind::Smooth:
        case DesignationKind::Polish:
        case DesignationKind::Engrave: return {"elysium:labor/stone_carving"};
        case DesignationKind::Coat:
        case DesignationKind::Seal: return {"elysium:labor/sealing"};
        case DesignationKind::Decontaminate: return {"elysium:labor/hazmat_haul"};
        case DesignationKind::Build:
        case DesignationKind::Reinforce:
        case DesignationKind::Repair:
        case DesignationKind::Replace: return {"elysium:labor/masonry"};
        case DesignationKind::Deconstruct: return {"elysium:labor/deconstruct"};
        case DesignationKind::Gather: return {"elysium:labor/wood_cutting"};
        case DesignationKind::Salvage: return {"elysium:labor/salvage_cutting"};
        case DesignationKind::Hunt: return {"elysium:labor/security"};
        case DesignationKind::Dump: return {"elysium:labor/haul_refuse"};
        case DesignationKind::UtilityRoute: return {"elysium:labor/utility_install"};
        case DesignationKind::Quarantine: return {"elysium:labor/trauma_care"};
        case DesignationKind::Forbid:
        case DesignationKind::Traffic:
        case DesignationKind::Evacuate: return {"elysium:labor/leadership"};
    }
    return {"elysium:labor/haul_general"};
}

} // namespace

JobScoreResult scoreJob(const JobComponent& job, const JobScoreInput& worker) {
    JobScoreResult result{};
    if (job.state != JobState::Pending && job.state != JobState::Blocked && job.state != JobState::Suspended) {
        result.reason = "job is not claimable";
        return result;
    }
    if (worker.biology) {
        if (worker.biology->oxygenation < 0.45f || worker.biology->bloodFraction < 0.45f) {
            result.reason = "worker is medically unfit";
            return result;
        }
        if (job.requirements.requiresMobility && worker.biology->pain > 0.82f) {
            result.reason = "worker mobility limited by pain";
            return result;
        }
    }
    const auto policy = laborPolicy(worker.workDetails, job.requirements.labor.value);
    if (policy == LaborPolicy::Forbidden) {
        result.reason = "labor policy forbids job";
        return result;
    }
    const float skill = skillRank(worker.skills, job.requirements.labor.value);
    if (skill < job.requirements.minimumSkill) {
        result.reason = "skill below minimum";
        return result;
    }
    if (worker.hazard > 0.85f && job.priority != PriorityBand::Emergency) {
        result.reason = "route exceeds hazard policy";
        return result;
    }

    float score = priorityWeight(job.priority);
    score += std::min(skill, 20.0f) * 2.2f;
    score -= std::max(0.0f, worker.estimatedDistance) * 0.35f;
    score -= saturate(worker.hazard) * 42.0f;
    if (worker.stress) score -= worker.stress->load * 24.0f;
    if (worker.biology) score -= worker.biology->sleepDebt * 8.0f + worker.biology->pain * 10.0f;
    if (policy == LaborPolicy::Preferred) score += 16.0f;
    if (policy == LaborPolicy::Required) score += 40.0f;
    if (job.priority == PriorityBand::Emergency) score += 60.0f;

    result.eligible = true;
    result.score = score;
    result.reason = "eligible";
    return result;
}

bool reservationCompatible(const ReservationClaim& existing, const ReservationClaim& incoming) {
    if (existing.resource != incoming.resource) return true;
    if (existing.job == incoming.job && existing.owner == incoming.owner) return true;
    if (existing.kind != incoming.kind) return false;
    switch (existing.kind) {
        case ReservationKind::Item:
        case ReservationKind::VehicleCapacity:
            return existing.quantity + incoming.quantity <= 1.0f;
        case ReservationKind::Tool:
        case ReservationKind::Workshop:
        case ReservationKind::Bed:
        case ReservationKind::TargetCell:
        case ReservationKind::Room:
        case ReservationKind::Door:
        case ReservationKind::MachinePort:
            return false;
    }
    return false;
}

bool workOrderConditionMet(const WorkOrderComponent& order, const FortressStockSnapshot& stock) {
    if (!order.enabled) return false;
    if (stock.powerReserve < order.condition.minimumPowerReserve) return false;
    if (stock.hazard > order.condition.maximumHazard) return false;
    if (stock.suspicion > order.condition.maximumSuspicion) return false;
    if (!order.condition.stockTag.value.empty()) {
        const auto it = stock.quantities.find(order.condition.stockTag.value);
        const float amount = it == stock.quantities.end() ? 0.0f : it->second;
        if (amount < order.condition.minimumStock || amount > order.condition.maximumStock) return false;
    }
    return true;
}

std::vector<CreateJobCommand> evaluateWorkOrder(const WorkOrderComponent& order,
                                                const FortressStockSnapshot& stock,
                                                std::uint64_t seed,
                                                std::uint64_t tick) {
    std::vector<CreateJobCommand> result;
    if (!workOrderConditionMet(order, stock)) return result;

    float current{};
    const auto it = stock.quantities.find(order.recipeOrJob.value);
    if (it != stock.quantities.end()) current = it->second;

    std::uint32_t count{};
    switch (order.mode) {
        case WorkOrderMode::ProduceQuantity:
            count = static_cast<std::uint32_t>(std::clamp(order.targetQuantity, 0.0f, 64.0f));
            break;
        case WorkOrderMode::MaintainAtLeast:
            count = static_cast<std::uint32_t>(std::clamp(order.lowerBound - current, 0.0f, 64.0f));
            break;
        case WorkOrderMode::MaintainBetween:
            if (current < order.lowerBound) count = static_cast<std::uint32_t>(std::clamp(order.upperBound - current, 0.0f, 64.0f));
            break;
        case WorkOrderMode::Repeat:
            count = 1;
            break;
    }

    for (std::uint32_t i = 0; i < count; ++i) {
        JobComponent job{};
        job.id = makeDerivedId<JobId>(seed, order.id.value, 0x574F524BULL + i, tick);
        job.type = order.recipeOrJob;
        job.priority = PriorityBand::Normal;
        job.origin = StableId{order.id.value};
        job.requirements.labor = ContentId{"elysium:labor/fabrication"};
        job.workRequired = 1.0f;
        result.push_back(CreateJobCommand{std::move(job)});
    }
    return result;
}

std::vector<CreateJobCommand> jobsFromDesignation(const DesignationComponent& designation,
                                                  std::uint64_t seed,
                                                  std::uint64_t tick,
                                                  std::uint32_t maxJobs) {
    std::vector<CreateJobCommand> result;
    if (!designation.generatesJobs || designation.suspended || maxJobs == 0) return result;

    const auto& min = designation.extent.min.cell;
    const auto& max = designation.extent.max.cell;
    const int u0 = std::min(min.u, max.u);
    const int u1 = std::max(min.u, max.u);
    const int v0 = std::min(min.v, max.v);
    const int v1 = std::max(min.v, max.v);
    const int r0 = std::min(min.radial, max.radial);
    const int r1 = std::max(min.radial, max.radial);

    std::uint32_t emitted{};
    for (int r = r0; r <= r1 && emitted < maxJobs; ++r) {
        for (int v = v0; v <= v1 && emitted < maxJobs; ++v) {
            for (int u = u0; u <= u1 && emitted < maxJobs; ++u) {
                JobComponent job{};
                const std::uint64_t identity = hashCoords(designation.id.value, u, v, r, designationLabel(designation.kind));
                job.id = makeDerivedId<JobId>(seed, identity, designationLabel(designation.kind), tick);
                job.type = jobTypeFor(designation.kind);
                job.priority = designation.priority;
                job.origin = designation.owner;
                job.target = designation.extent.min;
                job.target.cell.u = u;
                job.target.cell.v = v;
                job.target.cell.radial = r;
                job.requirements.labor = laborFor(designation.kind);
                job.requirements.access = designation.access;
                job.materialFilters.push_back(designation.materialPolicy);
                job.workRequired = 1.0f;
                result.push_back(CreateJobCommand{std::move(job)});
                ++emitted;
            }
        }
    }
    return result;
}

} // namespace elysium::fortress
