#include "fortress/WorldEvents.hpp"

#include <algorithm>

namespace elysium::fortress {

PlanetaryEvent generatePlanetaryEvent(std::uint64_t seed, SiteId site, std::uint64_t epoch,
                                      PlanetaryEventKind kind) {
    PlanetaryEvent event{};
    event.id = StableId{makeDerivedId<StableId>(seed, site.value, 0x504C414E4556454EULL ^ static_cast<std::uint64_t>(kind), epoch).value};
    event.kind = kind;
    event.site = site;
    const auto token = deterministicToken(seed, site.value, static_cast<std::uint64_t>(kind), epoch);
    event.intensity = 0.25f + static_cast<float>((token >> 16U) & 0xFFU) / 255.0f * 0.75f;
    event.durationHours = 1.0f + static_cast<float>((token >> 32U) & 0xFFU) / 255.0f * 48.0f;
    switch (kind) {
        case PlanetaryEventKind::StormFront: event.playerDecisionHint = "shelter, reroute exterior labor, or exploit wind/storm power"; break;
        case PlanetaryEventKind::MeteorFall: event.playerDecisionHint = "investigate salvage before hostile attention arrives"; break;
        case PlanetaryEventKind::SporeBloom: event.playerDecisionHint = "harvest rare organics or isolate contaminated districts"; break;
        case PlanetaryEventKind::Migration: event.playerDecisionHint = "hunt, scan, protect, or domesticate the moving population"; break;
        case PlanetaryEventKind::CaveCollapse: event.playerDecisionHint = "excavate, reroute, reinforce, or salvage rubble"; break;
        case PlanetaryEventKind::AquiferBreach: event.playerDecisionHint = "pump, seal, abandon, or exploit new access"; break;
        case PlanetaryEventKind::VolcanicSurge: event.playerDecisionHint = "secure evacuation routes or exploit thermal resources"; break;
        case PlanetaryEventKind::RadiationPulse: event.playerDecisionHint = "bunker, shield, or rush a protected objective"; break;
        case PlanetaryEventKind::UnswornCaravan: event.playerDecisionHint = "trade, escort, rob, or ignore"; break;
        case PlanetaryEventKind::ImperialSurvey: event.playerDecisionHint = "hide extraction, disable relay, or leave the system"; break;
        case PlanetaryEventKind::DistressCall: event.playerDecisionHint = "rescue, salvage, investigate, or avoid an ambush"; break;
        case PlanetaryEventKind::CelestialAlignment: event.playerDecisionHint = "perform time-bounded research and anomaly exploration"; break;
    }
    return event;
}

void advancePlanetaryEvent(PlanetaryEvent& event, float hours, float mitigation) {
    if (event.resolved) return;
    const float h = std::max(0.0f, hours);
    const float effective = h / std::max(0.1f, event.durationHours) * (1.0f + event.intensity * 0.25f) * (1.0f - saturate(mitigation) * 0.65f);
    event.progress = saturate(event.progress + effective);
    if (event.progress > 0.05f) event.signaled = true;
    event.resolved = event.progress >= 0.999f;
}

} // namespace elysium::fortress
