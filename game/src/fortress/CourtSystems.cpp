#include "fortress/CourtSystems.hpp"

#include <array>
#include <algorithm>

namespace elysium::fortress {
namespace {
constexpr std::array kEnvoys{
    CourtEnvoyDefinition{"elysium:court/elysomnion", "Elysomnion", "Emperor of the Black and Emerald", CourtMetric::Suspicion, 75.0f, 100.0f},
    CourtEnvoyDefinition{"elysium:court/sylphara_voss", "Sylphara Voss", "Chief Imperial Architect", CourtMetric::Suspicion, 25.0f, 100.0f},
    CourtEnvoyDefinition{"elysium:court/sentinel", "Sentinel", "Stealth Envoy", CourtMetric::Always, 0.0f, 100.0f},
    CourtEnvoyDefinition{"elysium:court/lillith", "Lillith", "Fleet Commander", CourtMetric::Favor, 50.0f, 100.0f},
    CourtEnvoyDefinition{"elysium:court/aurelia", "Aurelia", "Queen / former sentient star", CourtMetric::Favor, 75.0f, 100.0f}
};
constexpr std::array<std::string_view, 8> kRequests{
    "restore_archive", "construct_relay", "deliver_exotic_alloy", "suppress_unsworn_cell",
    "survey_anomaly", "host_inspection", "fortify_route", "recover_relic"
};
constexpr std::array<std::string_view, 8> kRewards{
    "imperial_blueprint", "rare_module", "aetherium_cache", "warrant_relief",
    "favor_grant", "court_relic", "orbital_license", "expedition_chart"
};
}

std::span<const CourtEnvoyDefinition> courtEnvoys() { return kEnvoys; }

bool envoyEligible(const CourtEnvoyDefinition& envoy, float favor, float suspicion) {
    if (envoy.metric == CourtMetric::Always) return true;
    const float value = envoy.metric == CourtMetric::Favor ? favor : suspicion;
    return value >= envoy.minimum && value <= envoy.maximum;
}

CourtOffer rollCourtOffer(std::uint64_t seed, std::uint64_t epoch, const CourtEnvoyDefinition& envoy,
                         std::uint64_t system, float favor, float suspicion) {
    CourtOffer offer{};
    const auto token = deterministicToken(seed, system, 0x434F555254ULL ^ std::uint64_t(envoy.id.size()), epoch);
    offer.id = StableId{token == 0 ? 1 : token};
    offer.envoy = ContentId{std::string(envoy.id)};
    offer.request = ContentId{"elysium:court/request/" + std::string(kRequests[token % kRequests.size()])};
    offer.reward = ContentId{"elysium:court/reward/" + std::string(kRewards[(token >> 12U) % kRewards.size()])};
    offer.system = system;
    const float exposure = boundedPercent(suspicion) / 100.0f;
    const float prestige = boundedPercent(favor) / 100.0f;
    offer.tributeValue = 500.0f + (exposure + prestige) * 1500.0f + static_cast<float>((token >> 32U) & 0x3FFU);
    offer.standingDelta = envoy.metric == CourtMetric::Suspicion ? -4.0f - exposure * 8.0f : 2.0f + prestige * 6.0f;
    return offer;
}

} // namespace elysium::fortress
