#include "fortress/StrategicGeneration.hpp"

#include <algorithm>

namespace elysium::fortress {

StrategicWorldState generateStrategicSeed(const StrategicWorldSeed& seed,
                                          std::span<const std::uint64_t> candidateSystems,
                                          std::uint32_t civilizationCount) {
    StrategicWorldState world{};
    if (candidateSystems.empty()) return world;
    const std::uint32_t count = std::min<std::uint32_t>(civilizationCount, static_cast<std::uint32_t>(candidateSystems.size()));
    for (std::uint32_t i = 0; i < count; ++i) {
        const std::uint64_t token = deterministicToken(seed.galaxySeed, candidateSystems[i], 0x434956494C495A45ULL, seed.generatorVersion + i);
        CivilizationState civilization{};
        civilization.id = makeDerivedId<CivilizationId>(seed.galaxySeed, candidateSystems[i], 0x434956ULL, i);
        civilization.name = "Civilization-" + std::to_string(civilization.id.value & 0xFFFFU);
        civilization.culture = ContentId{"elysium:culture/generated_" + std::to_string((token >> 8U) & 0xFFU)};
        civilization.government = ContentId{(token & 1U) ? "elysium:government/council" : "elysium:government/directorate"};
        civilization.faith = ContentId{(token & 2U) ? "elysium:faith/archive_tradition" : "elysium:faith/secular_technic"};
        civilization.population = 400.0f + static_cast<float>((token >> 16U) & 0xFFFU);
        civilization.technology = 0.2f + static_cast<float>((token >> 28U) & 0xFFU) / 255.0f * 0.7f;
        civilization.wealth = civilization.population * (0.5f + civilization.technology);
        civilization.militaryStrength = civilization.population * 0.05f * (0.5f + civilization.technology);

        SiteState site{};
        site.id = makeDerivedId<SiteId>(seed.galaxySeed, civilization.id.value, 0x53495445ULL, i);
        site.name = "Founding Site " + std::to_string(site.id.value & 0xFFFU);
        site.type = ContentId{"elysium:site/frontier_settlement"};
        site.civilization = civilization.id;
        site.system = candidateSystems[i];
        site.population = std::max(7.0f, civilization.population * 0.25f);
        site.foodReserve = site.population * 1.5f;
        site.industrialCapacity = civilization.technology * 20.0f;
        site.militaryStrength = civilization.militaryStrength * 0.35f;
        site.prosperity = 0.45f + civilization.technology * 0.3f;
        site.stability = 0.65f;

        OrganizationState government{};
        government.id = makeDerivedId<OrganizationId>(seed.galaxySeed, site.id.value, 0x474F5645524EULL, i);
        government.name = site.name + " Government";
        government.purpose = ContentId{"elysium:organization/local_government"};
        government.civilization = civilization.id;
        government.headquarters = site.id;
        government.wealth = civilization.wealth * 0.1f;
        government.influence = 0.5f;
        government.militaryStrength = site.militaryStrength;
        site.government = government.id;
        civilization.sites.push_back(site.id);
        civilization.organizations.push_back(government.id);

        HistoricalEvent founded{};
        founded.id = makeDerivedId<HistoricalEventId>(seed.galaxySeed, site.id.value, 0x464F554E444544ULL, i);
        founded.kind = HistoricalEventKind::SiteFounded;
        founded.site = site.id;
        founded.organization = government.id;
        founded.civilization = civilization.id;
        founded.summary = site.name + " was founded.";
        founded.significance = 0.8f;

        world.civilizations.push_back(std::move(civilization));
        world.organizations.push_back(std::move(government));
        world.sites.push_back(std::move(site));
        world.bootstrapHistory.push_back(std::move(founded));
    }
    return world;
}

} // namespace elysium::fortress
