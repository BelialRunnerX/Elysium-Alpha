// Intended function: Track food nutrition, hydration, spoilage, contamination, preferences, dietary restrictions, and meal quality.
#include "NutritionSystem.hpp"
namespace elysium::food {
std::uint64_t NutritionRecordStore::idOf(const NutritionRecord& v) noexcept { return static_cast<std::uint64_t>(v.foodId); }
bool NutritionRecordStore::put(NutritionRecord v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const NutritionRecord& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool NutritionRecordStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const NutritionRecord& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const NutritionRecord* NutritionRecordStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const NutritionRecord& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::food
