#include "fortress/DiseaseWorkflow.hpp"

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kDiseaseLabel = 0x4449534541534531ULL;
}

WorkflowPlan planDiseaseResponse(const SyndromeState& syndrome,
                                 SiteId site,
                                 std::span<const DiseaseContact> contacts,
                                 std::uint64_t seed,
                                 std::uint64_t tick,
                                 std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    if (syndrome.severity < 0.2f && syndrome.contagiousness < 0.2f) return plan;

    const auto quarantineId = makeDerivedId<JobId>(seed, syndrome.host.value, kDiseaseLabel, tick);
    auto quarantine = workflowJob(quarantineId, "elysium:job/quarantine_patient",
                                  ContentId{"elysium:labor/quarantine"}, PriorityBand::Urgent,
                                  syndrome.host, SpatialAnchor{}, 1.0f);
    quarantine.interruptible = false;
    plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{quarantine});

    for (std::size_t i = 0; i < contacts.size(); ++i) {
        const auto& contact = contacts[i];
        const float risk = transmissionRisk(syndrome, contact.contactIntensity,
                                            contact.filtration, contact.protection);
        if (risk < 0.35f) continue;
        const auto id = makeDerivedId<JobId>(seed, contact.host.value, kDiseaseLabel,
                                             tick + 1 + static_cast<std::uint64_t>(i));
        auto screening = workflowJob(id, "elysium:job/medical_screening",
                                     ContentId{"elysium:labor/diagnosis"}, PriorityBand::High,
                                     contact.host, SpatialAnchor{}, 0.75f);
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{screening});
    }
    plan.events.push_back(workflowEvent(FortressEventKind::DiseaseDetected,
                                        TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                        syndrome.host, site, "Syndrome response and contact screening staged",
                                        syndrome.severity));
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
