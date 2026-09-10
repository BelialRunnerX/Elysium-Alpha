// Intended function: imported world implementation for ResourceProgression; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/Block.hpp"

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

// Stable design catalogue for the Third/Fourth Edition ore spine. Entries may
// exist before their voxel material is present in this standalone prototype;
// generatedInStandalone makes that implementation boundary explicit.
struct OreProgressionDefinition {
    std::string_view contentId;
    std::string_view name;
    int peakAltitude;
    int spread;
    int toolTier;
    std::string_view element;
    std::string_view role;
    bool generatedInStandalone;
    BlockType standaloneBlock{BlockType::Air};
};

std::span<const OreProgressionDefinition> oreProgressionCatalog();
const OreProgressionDefinition* oreProgression(std::string_view contentId);

struct MiningBreakRule {
    bool mineable{};
    bool toolAllowed{};
    bool playerPlaced{};
    int requiredToolTier{};
    float breakSeconds{};
    BlockType drop{BlockType::Air};
    int dropCount{};
    int xp{};
    float suspicion{};
};

// Pure mining rule shared by the interactive client and headless tests. World
// mutation/inventory publication remains an owner-thread transaction at the
// caller, so this function cannot partially edit a voxel or duplicate a drop.
MiningBreakRule evaluateMiningBreak(BlockType type,
                                    int toolTier,
                                    bool playerPlaced);

struct ObtainabilityRecipe {
    std::string id;
    std::vector<int> inputs;
    std::vector<int> outputs;
};

struct ObtainabilityReport {
    bool valid{};
    std::vector<int> obtainable;
    std::vector<int> unreachableRequired;
    std::vector<std::string> circularRecipeIslands;
    std::vector<std::string> diagnostics;
};

// Computes the fixed-point obtainable set from generated/drop/starter/grant
// sources plus recipes. Required survival items that remain unreachable fail.
// Recipes whose outputs are unreachable and whose unresolved inputs are
// produced only by the same unresolved recipe subgraph are reported as cycles.
ObtainabilityReport validateObtainability(std::span<const int> sources,
                                          std::span<const ObtainabilityRecipe> recipes,
                                          std::span<const int> requiredSurvivalItems);

// Adapter for the current SurfaceIndustry recipe table. Machine availability is
// intentionally not inferred here; callers seed machine-kit/grant prerequisites
// into `sources` when validating a particular campaign start.
enum class ProgressionUnlockId : int {
    BronzeMiningHead = 5000,
    SteelMiningHead = 5001
};

std::vector<ObtainabilityRecipe> survivalCraftingObtainabilityRecipes();
std::vector<ObtainabilityRecipe> surfaceIndustryObtainabilityRecipes();

} // namespace elysium
