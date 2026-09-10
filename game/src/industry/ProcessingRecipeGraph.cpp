#include "industry/ProcessingRecipeGraph.hpp"
#include "core/Saturating.hpp"
#include <algorithm>

namespace elysium::industry {
namespace { constexpr std::uint64_t ProviderRecipe = 0x524543495045ULL; }

bool ProcessingRecipeGraph::publish(ProcessingRecipe recipe, reason::ReasonStack* reasons) {
    reason::ReasonStack local;
    if (recipe.id == 0 || recipe.stationCapability == 0 || recipe.inputs.empty() || recipe.outputs.empty())
        local.add({reason::ReasonCode::InvalidRequest, ProviderRecipe, recipe.id, 0});
    for (const auto& v : recipe.inputs) if (v.item == 0 || v.amount == 0) local.add({reason::ReasonCode::UnknownContent, ProviderRecipe, recipe.id, 10});
    for (const auto& v : recipe.outputs) if (v.item == 0 || v.amount == 0) local.add({reason::ReasonCode::UnknownContent, ProviderRecipe, recipe.id, 10});
    recipe.cycleSeconds = safe::nonNegative(recipe.cycleSeconds);
    recipe.powerPerSecond = safe::nonNegative(recipe.powerPerSecond);
    recipe.recycleFraction = safe::finiteClamp(recipe.recycleFraction, 0.0, 1.0);
    if (reasons) reasons->append(local);
    if (local.blocked()) return false;
    return recipes_.emplace(recipe.id, std::move(recipe)).second;
}

const ProcessingRecipe* ProcessingRecipeGraph::find(RecipeId id) const {
    auto it = recipes_.find(id); return it == recipes_.end() ? nullptr : &it->second;
}
std::vector<RecipeId> ProcessingRecipeGraph::orderedIds() const {
    std::vector<RecipeId> ids; ids.reserve(recipes_.size());
    for (const auto& [id, _] : recipes_) ids.push_back(id);
    std::sort(ids.begin(), ids.end()); return ids;
}
RecipeExecutionPlan ProcessingRecipeGraph::plan(RecipeId id, const RecipeExecutionContext& context) const {
    RecipeExecutionPlan out{};
    const auto* recipe = find(id);
    if (!recipe) { out.reasons.add({reason::ReasonCode::UnknownContent, ProviderRecipe, id, 0}); return out; }
    if (context.stationCapability != recipe->stationCapability) {
        out.reasons.add({reason::ReasonCode::MissingCapability, ProviderRecipe, id, 10}); return out;
    }
    for (const auto& need : recipe->inputs) {
        std::uint64_t have = 0;
        for (const auto& item : context.availableInputs) if (item.item == need.item) have = safe::saturatingAdd(have, item.amount);
        if (have < need.amount) { out.reasons.add({reason::ReasonCode::MissingInput, ProviderRecipe, need.item, 20}); }
    }
    std::uint64_t outputUnits = 0;
    for (const auto& item : recipe->outputs) outputUnits = safe::saturatingAdd(outputUnits, item.amount);
    if (outputUnits > context.outputFreeUnits) out.reasons.add({reason::ReasonCode::OutputBlocked, ProviderRecipe, id, 30});
    if (recipe->powerPerSecond > safe::nonNegative(context.availablePowerPerSecond))
        out.reasons.add({reason::ReasonCode::InsufficientPower, ProviderRecipe, id, 40});
    if (out.reasons.blocked()) return out;
    out.accepted = true; out.consume = recipe->inputs; out.produce = recipe->outputs;
    out.cycleSeconds = recipe->cycleSeconds; out.powerPerSecond = recipe->powerPerSecond; out.statePolicy = recipe->statePolicy;
    return out;
}

} // namespace elysium::industry
