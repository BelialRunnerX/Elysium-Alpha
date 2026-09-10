#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>
#include <vector>

namespace elysium::fortress {

struct ProductionRequest {
    StableId origin{};
    SiteId site{};
    SpatialAnchor target{};
    ContentId recipe;
    ContentId labor;
    StableId workshop{};
    std::vector<std::pair<ContentId, float>> inputs;
    std::vector<std::pair<ContentId, float>> outputs;
    PriorityBand priority{PriorityBand::Normal};
    float work{1.0f};
};

struct InventoryCandidate {
    StableId item{};
    ContentId content;
    StableId container{};
    SpatialAnchor location{};
    float availableQuantity{};
    bool forbidden{};
    bool reserved{};
};

WorkflowPlan planProductionChain(const ProductionRequest& request,
                                 std::span<const InventoryCandidate> inventory,
                                 std::span<const StockpileComponent> stockpiles,
                                 std::uint64_t seed,
                                 std::uint64_t tick,
                                 std::uint32_t producer = 100);

} // namespace elysium::fortress
