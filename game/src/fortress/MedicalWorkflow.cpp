#include "fortress/MedicalWorkflow.hpp"

#include <algorithm>

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kMedicalJobLabel = 0x4D45444943414C31ULL;

const HospitalCapacity* bestHospital(std::span<const HospitalCapacity> hospitals) {
    const HospitalCapacity* best = nullptr;
    float bestScore = -1.0f;
    for (const auto& hospital : hospitals) {
        if (hospital.freeBeds == 0) continue;
        const float score = 0.45f * hospital.cleanliness + 0.35f * hospital.atmosphereSafety +
                            0.20f * hospital.supplyFraction;
        if (best == nullptr || score > bestScore ||
            (score == bestScore && hospital.hospital.value < best->hospital.value)) {
            best = &hospital;
            bestScore = score;
        }
    }
    return best;
}

ContentId laborForTreatment(TreatmentKind treatment) {
    switch (treatment) {
        case TreatmentKind::Diagnose: return ContentId{"elysium:labor/diagnosis"};
        case TreatmentKind::SetBone: return ContentId{"elysium:labor/bone_setting"};
        case TreatmentKind::Surgery: return ContentId{"elysium:labor/surgery"};
        case TreatmentKind::Medication: return ContentId{"elysium:labor/pharmacology"};
        case TreatmentKind::Rehabilitation: return ContentId{"elysium:labor/rehabilitation"};
        case TreatmentKind::ProstheticFit: return ContentId{"elysium:labor/prosthetics"};
        default: return ContentId{"elysium:labor/trauma_care"};
    }
}
}

WorkflowPlan planMedicalResponse(StableId patient,
                                 SiteId site,
                                 const BodyState& body,
                                 std::span<const HospitalCapacity> hospitals,
                                 std::uint64_t seed,
                                 std::uint64_t tick,
                                 std::uint32_t producer) {
    WorkflowPlan plan{};
    const auto treatment = buildTreatmentPlan(patient, body);
    const auto* hospital = bestHospital(hospitals);
    std::uint32_t sequence = 0;

    if (hospital == nullptr && !treatment.orders.empty()) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "no_hospital_bed", "Patient needs treatment but no safe hospital bed is available",
            PriorityBand::Emergency, patient});
    }

    JobId previous{};
    for (std::size_t i = 0; i < treatment.orders.size(); ++i) {
        const auto& order = treatment.orders[i];
        const auto id = makeDerivedId<JobId>(seed, patient.value, kMedicalJobLabel,
                                             tick + static_cast<std::uint64_t>(i));
        auto job = workflowJob(id, "elysium:job/medical_treatment", laborForTreatment(order.treatment),
                               order.priority, patient, SpatialAnchor{}, std::max(0.25f, order.work));
        if (previous) job.dependencies.push_back(previous);
        if (hospital != nullptr) {
            ReservationClaim bed{};
            bed.kind = ReservationKind::Bed;
            bed.resource = StableId{hospital->hospital.value};
            bed.owner = patient;
            bed.job = id;
            bed.expiresTick = static_cast<std::int64_t>(tick + 1800);
            job.reservations.push_back(bed);
            plan.commands.push(workflowHeader(tick, producer, sequence++), ReserveCommand{bed});
        } else {
            job.state = JobState::Blocked;
            job.failureReason = "no safe treatment bed";
        }
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateTreatmentCommand{order});
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{job});
        previous = id;
    }

    if (!treatment.orders.empty()) {
        plan.events.push_back(workflowEvent(FortressEventKind::WoundApplied,
                                            TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            patient, site, "Medical response staged",
                                            static_cast<float>(treatment.orders.size())));
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
