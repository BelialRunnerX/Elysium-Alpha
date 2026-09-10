#pragma once
#include <cstdint>
#include <map>
#include <span>
#include <vector>
namespace elysium::food {
enum class FoodEffectKind:std::uint8_t{HazardResistance,StaminaRecovery,EnergyRecovery,MedicalSupport};
enum class EffectStackPolicy:std::uint8_t{RefreshDuration,ReplaceIfStronger,AddMagnitude};
enum class CookingFailure:std::uint8_t{None,UnknownRecipe,DuplicateTransaction,MissingIngredient,MissingRegionalTag,InvalidInput};
struct FoodEffectDefinition{std::uint64_t effectContentId{};FoodEffectKind kind{FoodEffectKind::StaminaRecovery};std::uint64_t durationTicks{};double magnitude{};EffectStackPolicy stacking{EffectStackPolicy::RefreshDuration};};
struct FoodDefinition{std::uint64_t contentId{};double hunger{},saturation{};std::vector<FoodEffectDefinition>effects;};
struct IngredientPredicate{std::uint64_t contentId{},requiredTagId{},quantity{};};
struct RecipeOutput{std::uint64_t contentId{},quantity{};bool agriculturalByproduct{};};
struct CuisineRecipe{std::uint64_t recipeId{};std::vector<IngredientPredicate>ingredients;std::vector<RecipeOutput>outputs;std::vector<std::uint64_t>requiredRegionalTags;};
struct InventoryLine{std::uint64_t contentId{},quantity{};std::vector<std::uint64_t>tags;};
struct CookingRequest{std::uint64_t transactionId{},recipeId{};std::vector<InventoryLine>inputs;std::vector<std::uint64_t>regionalTags;};
struct CookingReceipt{bool accepted{};CookingFailure failure{CookingFailure::None};std::uint64_t transactionId{},recipeId{};std::vector<InventoryLine>consumed;std::vector<RecipeOutput>produced;std::vector<RecipeOutput>byproducts;};
struct FoodEffectIntent{std::uint64_t actorId{},sourceFoodId{},effectContentId{};FoodEffectKind kind{};std::uint64_t durationTicks{};double magnitude{};EffectStackPolicy stacking{};};
struct FoodConsumptionIntent{std::uint64_t actorId{},foodContentId{};double hunger{},saturation{};std::vector<FoodEffectIntent>effects;};
struct FoodRuntimeSnapshot{std::vector<FoodDefinition>foods;std::vector<CuisineRecipe>recipes;std::vector<std::uint64_t>processedTransactions;};
class FoodRuntime{public:
 bool publish(FoodDefinition);bool publish(CuisineRecipe);
 [[nodiscard]]CookingReceipt cook(const CookingRequest&);
 [[nodiscard]]FoodConsumptionIntent consume(std::uint64_t actorId,std::uint64_t foodContentId)const;
 [[nodiscard]]static FoodEffectIntent combine(const FoodEffectIntent&a,const FoodEffectIntent&b);
 [[nodiscard]]FoodRuntimeSnapshot snapshot()const;bool restore(const FoodRuntimeSnapshot&);
private:std::map<std::uint64_t,FoodDefinition>foods_;std::map<std::uint64_t,CuisineRecipe>recipes_;std::map<std::uint64_t,bool>processed_;
};
} // namespace elysium::food
