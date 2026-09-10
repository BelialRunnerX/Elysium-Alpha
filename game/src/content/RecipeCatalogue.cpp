// Intended function: Provide stable recipe definitions with ingredients, outputs, workstation tags, unlocks, and processing costs.
#include "RecipeCatalogue.hpp"
namespace elysium::content {
std::uint64_t RecipeRecordRegistry::key(const RecipeRecord& r) noexcept { return static_cast<std::uint64_t>(r.recipeId); }
bool RecipeRecordRegistry::publish(RecipeRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const RecipeRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool RecipeRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const RecipeRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const RecipeRecord* RecipeRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const RecipeRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::content
