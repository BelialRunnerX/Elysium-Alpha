#include "fortress/JusticeWorkflow.hpp"

#include <algorithm>

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kCaseLabel = 0x4A55535449434531ULL;
constexpr std::uint64_t kInvestigationLabel = 0x494E564553544947ULL;
}

WorkflowPlan processObservedCrime(const CrimeRecord& crime,
                                  SiteId site,
                                  std::span<const WitnessStatement> witnesses,
                                  float forensicQuality,
                                  float lawSeverity,
                                  float rehabilitationBias,
                                  std::uint64_t seed,
                                  std::uint64_t tick,
                                  std::uint32_t producer) {
    WorkflowPlan plan{};
    float reliabilitySum = 0.0f;
    std::uint32_t observed = 0;
    for (const auto& witness : witnesses) {
        if (witness.observed) {
            reliabilitySum += saturate(witness.reliability);
            ++observed;
        }
    }
    if (observed == 0 && crime.evidence.empty()) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "crime_unobserved", "No witness or physical evidence exposes this crime to authorities",
            PriorityBand::Background, crime.perpetrator});
        return plan;
    }

    const float witnessReliability = observed > 0 ? reliabilitySum / static_cast<float>(observed) : 0.0f;
    JusticeCase justice{};
    justice.id = makeDerivedId<CaseId>(seed, crime.perpetrator.value, kCaseLabel, tick);
    justice.crime = crime;
    justice.accused = crime.perpetrator;
    justice.evidenceStrength = evidenceStrength(crime, forensicQuality, witnessReliability);
    justice.confidence = saturate(justice.evidenceStrength * 0.85f + witnessReliability * 0.15f);
    justice.status = ContentId{"elysium:case/investigating"};
    if (justice.confidence >= 0.75f) {
        justice.sentence = ContentId{recommendSentence(justice, lawSeverity, rehabilitationBias)};
    }

    std::uint32_t sequence = 0;
    plan.commands.push(workflowHeader(tick, producer, sequence++), CreateCaseCommand{justice});
    const auto investigationId = makeDerivedId<JobId>(seed, justice.id.value, kInvestigationLabel, tick);
    auto investigation = workflowJob(investigationId, "elysium:job/investigate_case",
                                     ContentId{"elysium:labor/investigation"}, PriorityBand::High,
                                     crime.perpetrator, crime.location, 1.0f + crime.severity);
    plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{investigation});
    plan.events.push_back(workflowEvent(FortressEventKind::CrimeWitnessed,
                                        TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                        crime.perpetrator, site, "Observed crime created a justice case",
                                        justice.confidence));
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
