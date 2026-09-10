#include "fortress/PoiSystems.hpp"

#include <array>

namespace elysium::fortress {
namespace {
constexpr std::array kPois{
    PoiDefinition{"elysium:poi/supply_cache", "surface", "consumables, fuel, rune chance", 1.0f},
    PoiDefinition{"elysium:poi/crashed_ship", "surface", "repairable hull/project", 0.65f},
    PoiDefinition{"elysium:poi/survey_outpost", "surface", "map reveal and ore clue", 0.9f},
    PoiDefinition{"elysium:poi/unsworn_camp", "surface", "trade and directions", 0.7f},
    PoiDefinition{"elysium:poi/mining_rig", "surface", "rich vein plus Suspicion", 0.75f},
    PoiDefinition{"elysium:poi/ruin", "surface", "Aetherium fragments and traps", 0.45f},
    PoiDefinition{"elysium:poi/monolith", "surface", "lore and once-per-planet effect", 0.3f},
    PoiDefinition{"elysium:poi/rift_anomaly", "deep", "dungeon entry", 0.25f},
    PoiDefinition{"elysium:poi/imperial_waystation", "surface", "garrison and pressure reduction target", 0.55f},
    PoiDefinition{"elysium:poi/abandoned_farm", "surface", "seeds, domestic fauna and food", 0.7f},
    PoiDefinition{"elysium:poi/weather_station", "surface", "forecast/radar data", 0.8f},
    PoiDefinition{"elysium:poi/subsurface_vault", "deep", "locked industrial cache", 0.4f},
    PoiDefinition{"elysium:poi/buried_reactor", "deep", "radiation, salvage and power restoration", 0.35f},
    PoiDefinition{"elysium:poi/research_habitat", "surface", "biology logs and samples", 0.6f},
    PoiDefinition{"elysium:poi/planetary_relay", "surface", "communications and map function", 0.65f},
    PoiDefinition{"elysium:poi/smuggler_den", "surface", "black-market trade or ambush", 0.5f},
    PoiDefinition{"elysium:poi/ancient_observatory", "surface", "star charts and anomaly clue", 0.35f},
    PoiDefinition{"elysium:poi/geothermal_plant", "surface", "power puzzle and thermal materials", 0.5f},
    PoiDefinition{"elysium:poi/flooded_complex", "submerged", "pressure traversal", 0.45f},
    PoiDefinition{"elysium:poi/orbital_debris_fall", "surface", "ship salvage field", 0.55f},
    PoiDefinition{"elysium:poi/trading_post", "orbit", "commodity economy", 0.75f},
    PoiDefinition{"elysium:poi/imperial_station", "orbit", "registered services and inspections", 0.55f},
    PoiDefinition{"elysium:poi/unsworn_haven", "orbit", "charts and unregistered trade", 0.4f},
    PoiDefinition{"elysium:poi/derelict_freighter", "space", "modular boarding encounter", 0.45f},
    PoiDefinition{"elysium:poi/data_buoy", "space", "route, lore and scanner upgrade", 0.8f},
    PoiDefinition{"elysium:poi/asteroid_refinery", "space", "bulk mining and industrial site", 0.5f},
    PoiDefinition{"elysium:poi/silent_colony", "surface", "mystery settlement", 0.3f},
    PoiDefinition{"elysium:poi/siege_ruin", "surface", "former fortress and defense loot", 0.25f},
    PoiDefinition{"elysium:poi/anomaly_labyrinth", "anomalous", "non-Euclidean traversal set-piece", 0.15f},
    PoiDefinition{"elysium:poi/court_anchorage", "orbit", "rare envoy staging site", 0.12f}
};
}

std::span<const PoiDefinition> poiCatalog() { return kPois; }

PoiState generatePoi(std::uint64_t seed, SiteId site, const CellAddress& anchor,
                     std::uint64_t ordinal, std::string_view typeId) {
    PoiState poi{};
    const auto token = deterministicToken(seed, site.value, 0x504F490000000000ULL ^ ordinal, hashCoords(seed, anchor.u, anchor.v, anchor.radial, anchor.face));
    poi.id = StableId{token == 0 ? 1 : token};
    poi.site = site;
    poi.type = ContentId{std::string(typeId)};
    poi.location.world = anchor.planet;
    poi.location.cell = anchor;
    poi.damage = static_cast<float>((token >> 8U) & 0xFFU) / 255.0f;
    poi.hazard = static_cast<float>((token >> 24U) & 0xFFU) / 255.0f;
    poi.lootRichness = 0.25f + static_cast<float>((token >> 40U) & 0xFFU) / 255.0f * 1.25f;
    poi.reclaimable = typeId.find("ruin") != std::string_view::npos || typeId.find("colony") != std::string_view::npos || typeId.find("outpost") != std::string_view::npos;
    return poi;
}

} // namespace elysium::fortress
