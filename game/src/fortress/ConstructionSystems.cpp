#include "fortress/Systems.hpp"

#include <algorithm>

namespace elysium::fortress {

std::vector<CreateDesignationCommand> expandBlueprint(const ConstructionBlueprint& blueprint,
                                                       const SpatialAnchor& origin,
                                                       StableId owner,
                                                       std::uint64_t seed,
                                                       std::uint64_t tick) {
    std::vector<CreateDesignationCommand> result;
    result.reserve(blueprint.steps.size());
    std::uint64_t index{};
    for (const auto& step : blueprint.steps) {
        DesignationComponent designation{};
        designation.id = makeDerivedId<DesignationId>(seed, blueprint.id.value, 0x4250554C44535450ULL ^ index, tick);
        designation.kind = step.kind;
        designation.owner = owner;
        designation.materialPolicy = step.material;
        designation.priority = PriorityBand::Normal;
        designation.extent = step.relativeExtent;
        designation.extent.min.world = origin.world;
        designation.extent.max.world = origin.world;
        designation.extent.min.cell.planet = origin.cell.planet;
        designation.extent.max.cell.planet = origin.cell.planet;
        designation.extent.min.cell.face = origin.cell.face;
        designation.extent.max.cell.face = origin.cell.face;
        designation.extent.min.cell.u += origin.cell.u;
        designation.extent.max.cell.u += origin.cell.u;
        designation.extent.min.cell.v += origin.cell.v;
        designation.extent.max.cell.v += origin.cell.v;
        designation.extent.min.cell.radial += origin.cell.radial;
        designation.extent.max.cell.radial += origin.cell.radial;
        result.push_back(CreateDesignationCommand{std::move(designation)});
        ++index;
    }
    return result;
}

float supportMargin(const SupportIsland& island) {
    if (island.rooted) return std::max(0.0f, island.supportCapacity - island.load) + island.supportCapacity * 0.25f;
    return island.supportCapacity - island.load;
}

bool shouldCollapse(const SupportIsland& island) {
    return island.dirty && !island.rooted && supportMargin(island) < 0.0f;
}

} // namespace elysium::fortress
