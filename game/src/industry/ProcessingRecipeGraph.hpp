#pragma once
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>

namespace elysium::industry {
using ContentId = std::uint64_t;
using CapabilityId = std::uint64_t;
using RecipeId = std::uint64_t;

enum class StatePolicy : std::uint8_t { RecreateOutput, PreservePrimaryItemState, TransformStateExplicitly };
enum class OperationKind : std::uint8_t { Craft, Smelt, Refine, Alloy, Crush, Chemical, Arc, Recycle, Repair, Upgrade };

struct ItemAmount { ContentId item{}; std::uint64_t amount{}; };
struct ProcessingRecipe {
    RecipeId id{};
    OperationKind operation{OperationKind::Craft};
    CapabilityId stationCapability{};
    std::vector<ItemAmount> inputs;
    std::vector<ItemAmount> outputs;
    double cycleSeconds{};
    double powerPerSecond{};
    StatePolicy statePolicy{StatePolicy::RecreateOutput};
    double recycleFraction{}; // [0,1], only meaningful for recycle operations.
    std::uint32_t schemaVersion{1};
};

struct RecipeExecutionContext {
    CapabilityId stationCapability{};
    std::span<const ItemAmount> availableInputs;
    std::uint64_t outputFreeUnits{};
    double availablePowerPerSecond{};
};

struct RecipeExecutionPlan {
    bool accepted{};
    reason::ReasonStack reasons;
    std::vector<ItemAmount> consume;
    std::vector<ItemAmount> produce;
    double cycleSeconds{};
    double powerPerSecond{};
    StatePolicy statePolicy{StatePolicy::RecreateOutput};
};

class ProcessingRecipeGraph {
public:
    bool publish(ProcessingRecipe recipe, reason::ReasonStack* reasons = nullptr);
    [[nodiscard]] const ProcessingRecipe* find(RecipeId id) const;
    [[nodiscard]] std::vector<RecipeId> orderedIds() const;
    [[nodiscard]] RecipeExecutionPlan plan(RecipeId id, const RecipeExecutionContext& context) const;
private:
    std::unordered_map<RecipeId, ProcessingRecipe> recipes_;
};

} // namespace elysium::industry
