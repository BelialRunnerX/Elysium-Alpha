// Intended function: Represent UI scale, contrast, motion reduction, subtitle/caption policy, color-independent indicators, input assists, and text timing.
#include "AccessibilityModel.hpp"
namespace elysium::ui {
std::uint64_t AccessibilityStateCollection::idOf(const AccessibilityState& v) noexcept { return static_cast<std::uint64_t>(v.profileId); }
bool AccessibilityStateCollection::store(AccessibilityState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AccessibilityState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool AccessibilityStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AccessibilityState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const AccessibilityState* AccessibilityStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AccessibilityState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
