// Intended function: Generate deterministic item affixes from tier, item family, seed, alignment, rarity, exclusions, and power budget.
#include "AffixSystem.hpp"
namespace elysium::loot {
std::uint64_t AffixRollStore::idOf(const AffixRoll& v) noexcept { return static_cast<std::uint64_t>(v.rollId); }
bool AffixRollStore::put(AffixRoll v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const AffixRoll& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool AffixRollStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const AffixRoll& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const AffixRoll* AffixRollStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const AffixRoll& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::loot
