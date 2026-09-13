#include "fortress/IncidentWorkflow.hpp"

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kIncidentLabel = 0x494E434944454E54ULL;

std::pair<std::string, std::string> jobForIncident(PlanetaryEventKind kind) {
    switch (kind) {
        case PlanetaryEventKind::StormFront: return {"elysium:job/storm_secure", "elysium:labor/emergency_utility"};
        case PlanetaryEventKind::MeteorFall: return {"elysium:job/survey_meteor", "elysium:labor/geology"};
        case PlanetaryEventKind::SporeBloom: return {"elysium:job/spore_containment", "elysium:labor/decontamination"};
        case PlanetaryEventKind::Migration: return {"elysium:job/ecology_survey", "elysium:labor/xenobiology"};
        case PlanetaryEventKind::CaveCollapse: return {"elysium:job/collapse_rescue", "elysium:labor/rescue"};
        case PlanetaryEventKind::AquiferBreach: return {"elysium:job/aquifer_control", "elysium:labor/pumping"};
        case PlanetaryEventKind::VolcanicSurge: return {"elysium:job/thermal_secure", "elysium:labor/emergency_utility"};
        case PlanetaryEventKind::RadiationPulse: return {"elysium:job/radiation_shelter", "elysium:labor/evacuation_response"};
        case PlanetaryEventKind::UnswornCaravan: return {"elysium:job/receive_caravan", "elysium:labor/brokerage"};
        case PlanetaryEventKind::ImperialSurvey: return {"elysium:job/security_sweep", "elysium:labor/security"};
        case PlanetaryEventKind::DistressCall: return {"elysium:job/distress_response", "elysium:labor/rescue"};
        case PlanetaryEventKind::CelestialAlignment: return {"elysium:job/anomaly_observation", "elysium:labor/rift_science"};
    }
    return {"elysium:job/incident_response", "elysium:labor/security"};
}
}

WorkflowPlan planPlanetaryIncident(const PlanetaryEvent& event,
                                   std::uint64_t seed,
                                   std::uint64_t tick,
                                   std::uint32_t producer) {
    WorkflowPlan plan{};
    if (event.resolved) return plan;
    const auto [type, labor] = jobForIncident(event.kind);
    const auto id = makeDerivedId<JobId>(seed, event.id.value, kIncidentLabel, tick);
    auto job = workflowJob(id, type, ContentId{labor},
                           event.intensity > 0.7f ? PriorityBand::Emergency : PriorityBand::High,
                           event.id, event.location, 1.0f + 2.0f * event.intensity);
    plan.commands.push(workflowHeader(tick, producer, 0), CreateJobCommand{job});
    plan.events.push_back(workflowEvent(FortressEventKind::JobCreated,
                                        TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                        event.id, event.site,
                                        event.signaled ? "Planetary incident response staged" :
                                                         "Planetary incident requires a readable warning signal",
                                        event.intensity));
    if (!event.signaled) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "incident_unsignaled", "Irreversible planetary event must expose a readable signal first",
            PriorityBand::High, event.id});
    }
    return plan;
}

} // namespace elysium::fortress
