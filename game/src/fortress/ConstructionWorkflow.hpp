#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct ConstructionMaterialSource {
    StableId item{};
    ContentId material;
    float quantity{};
    bool accessible{true};
};

struct ConstructionStageState {
    BlueprintId blueprint{};
    StableId owner{};
    SiteId site{};
    SpatialAnchor origin{};
    float supportSafety{1.0f};
    float utilityAccess{1.0f};
    bool commissioned{};
};

WorkflowPlan planConstructionWorkflow(const ConstructionBlueprint& blueprint,
                                      const ConstructionStageState& state,
                                      std::span<const ConstructionMaterialSource> materials,
                                      std::uint64_t seed,
                                      std::uint64_t tick,
                                      std::uint32_t producer = 300);

} // namespace elysium::fortress
