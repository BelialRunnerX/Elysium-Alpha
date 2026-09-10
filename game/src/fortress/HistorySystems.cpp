#include "fortress/Systems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

float historicalSignificance(const HistoricalEvent& event) {
    float base = event.significance;
    switch (event.kind) {
        case HistoricalEventKind::Birth:
        case HistoricalEventKind::Naming:
        case HistoricalEventKind::ComingOfAge: base += 0.1f; break;
        case HistoricalEventKind::Death:
        case HistoricalEventKind::OfficeAssumed:
        case HistoricalEventKind::OfficeRemoved:
        case HistoricalEventKind::Discovery:
        case HistoricalEventKind::Filing: base += 0.35f; break;
        case HistoricalEventKind::RelationshipFormed:
        case HistoricalEventKind::RelationshipDissolved:
        case HistoricalEventKind::Mentorship:
        case HistoricalEventKind::Rivalry:
        case HistoricalEventKind::Oath:
        case HistoricalEventKind::Betrayal: base += 0.2f; break;
        case HistoricalEventKind::SiteFounded:
        case HistoricalEventKind::SiteClaimed:
        case HistoricalEventKind::SiteAbandoned:
        case HistoricalEventKind::SiteDestroyed:
        case HistoricalEventKind::SiteReclaimed:
        case HistoricalEventKind::OrganizationFounded:
        case HistoricalEventKind::OrganizationDissolved:
        case HistoricalEventKind::WarDeclared:
        case HistoricalEventKind::Battle:
        case HistoricalEventKind::Raid:
        case HistoricalEventKind::Siege:
        case HistoricalEventKind::Occupation:
        case HistoricalEventKind::Liberation:
        case HistoricalEventKind::Armistice:
        case HistoricalEventKind::Treaty:
        case HistoricalEventKind::Embargo:
        case HistoricalEventKind::RiftAppearance:
        case HistoricalEventKind::RiftDefeat:
        case HistoricalEventKind::Disaster:
        case HistoricalEventKind::Outbreak:
        case HistoricalEventKind::RegisterAction:
        case HistoricalEventKind::CourtRuling:
        case HistoricalEventKind::Annexation:
        case HistoricalEventKind::FortressRetired: base += 0.7f; break;
        case HistoricalEventKind::ArtifactCreated:
        case HistoricalEventKind::ArtifactStolen:
        case HistoricalEventKind::ArtifactGifted:
        case HistoricalEventKind::ArtifactInherited:
        case HistoricalEventKind::ArtifactLost:
        case HistoricalEventKind::ArtifactRecovered:
        case HistoricalEventKind::ArtifactDestroyed:
        case HistoricalEventKind::Breakthrough:
        case HistoricalEventKind::FirstLanding:
        case HistoricalEventKind::FirstClaim:
        case HistoricalEventKind::OperativeIntervention: base += 0.55f; break;
        case HistoricalEventKind::Disappearance:
        case HistoricalEventKind::SiteExpanded:
        case HistoricalEventKind::SiteRenamed:
        case HistoricalEventKind::SiteContaminated:
        case HistoricalEventKind::Schism:
        case HistoricalEventKind::Crime:
        case HistoricalEventKind::Accusation:
        case HistoricalEventKind::Conviction:
        case HistoricalEventKind::Exile:
        case HistoricalEventKind::Pardon:
        case HistoricalEventKind::Inspection:
        case HistoricalEventKind::Warrant:
        case HistoricalEventKind::Amnesty: base += 0.3f; break;
    }
    if (event.artifact) base += 0.15f;
    if (event.organization) base += 0.10f;
    if (event.civilization) base += 0.12f;
    return std::max(0.0f, base);
}

bool shouldPromoteHistoricalFigure(const HistorySignificance& current,
                                   const HistoricalEvent& event) {
    if (current.historicalFigure || current.forcePersist) return true;
    return current.score + historicalSignificance(event) >= 0.85f;
}

void advanceCivilization(CivilizationState& civilization, float years, float resourceAccess,
                         float conflictPressure, float tradeAccess) {
    const float y = std::max(0.0f, years);
    const float resources = saturate(resourceAccess);
    const float trade = saturate(tradeAccess);
    const float conflict = saturate(conflictPressure);
    const float growth = 0.008f * resources + 0.005f * trade - 0.012f * conflict;
    civilization.population = std::max(0.0f, civilization.population * std::exp(growth * y));
    civilization.technology = std::max(0.0f, civilization.technology + y * (resources * 0.02f + trade * 0.03f - conflict * 0.005f));
    civilization.wealth = std::max(0.0f, civilization.wealth + y * civilization.population * (trade * 0.001f + resources * 0.0006f) - y * conflict * civilization.militaryStrength * 0.02f);
    civilization.militaryStrength = std::max(0.0f, civilization.militaryStrength + y * (civilization.population * conflict * 0.0005f + civilization.wealth * 0.00005f));
}

void advanceSite(SiteState& site, float days, float incomingFood, float outgoingGoods,
                 float threatPressure) {
    const float d = std::max(0.0f, days);
    site.foodReserve = std::max(0.0f, site.foodReserve + incomingFood - site.population * 0.012f * d);
    const float foodSecurity = site.population <= 0.0f ? 1.0f : saturate(site.foodReserve / std::max(1.0f, site.population));
    const float threat = saturate(threatPressure);
    const float growth = (foodSecurity - 0.5f) * 0.0008f * d - threat * 0.0007f * d;
    site.population = std::max(0.0f, site.population * (1.0f + growth));
    site.prosperity = saturate(site.prosperity + outgoingGoods * 0.0002f - threat * d * 0.002f + site.industrialCapacity * d * 0.0001f);
    site.stability = saturate(site.stability + (foodSecurity - 0.5f) * d * 0.001f - threat * d * 0.003f);
    if (site.population < 0.5f && site.stability < 0.15f) site.abandoned = true;
    if (site.abandoned && site.contamination > 0.7f) site.ruined = true;
}

} // namespace elysium::fortress
