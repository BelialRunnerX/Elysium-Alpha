#pragma once

#include "fortress/Components.hpp"

#include <span>
#include <string_view>

namespace elysium::fortress {

struct PoiDefinition {
    std::string_view id;
    std::string_view locationDomain;
    std::string_view purpose;
    float rarity{};
};

struct PoiState {
    StableId id{};
    SiteId site{};
    ContentId type;
    SpatialAnchor location{};
    float damage{};
    float hazard{};
    float lootRichness{};
    OrganizationId owner{};
    bool discovered{};
    bool cleared{};
    bool reclaimable{};
};

std::span<const PoiDefinition> poiCatalog();
PoiState generatePoi(std::uint64_t seed, SiteId site, const CellAddress& anchor,
                     std::uint64_t ordinal, std::string_view typeId);

} // namespace elysium::fortress
