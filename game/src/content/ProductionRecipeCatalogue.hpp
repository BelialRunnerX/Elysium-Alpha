#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>
namespace elysium::content {
using ContentId=std::uint64_t;enum class RecipeStatus:std::uint8_t{Locked,Current,Proposed,Tuning};struct ItemAmount{ContentId id{};std::uint64_t amount{};};struct ProductionRecipe{ContentId id{},station{},output{};std::uint64_t outputAmount{};std::uint32_t tier{};ContentId unlock{};RecipeStatus status{RecipeStatus::Current};std::vector<ItemAmount>inputs;std::set<ContentId>useTags;};struct RecipeIssue{ContentId id{};std::string code;};class ProductionRecipeCatalogue{public:void addRawSource(ContentId);void addKnownStation(ContentId);bool add(ProductionRecipe);const ProductionRecipe*find(ContentId)const;std::vector<RecipeIssue>validate()const;bool obtainable(ContentId)const;private:std::set<ContentId>raw_,stations_;std::map<ContentId,ProductionRecipe>recipes_;bool obtainableImpl(ContentId,std::set<ContentId>&)const;};
}
