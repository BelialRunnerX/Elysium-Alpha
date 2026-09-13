// Intended function: Project available structures/blocks/blueprints, material costs, blockers, power/atmosphere requirements, and placement modes.
#include "BuildMenu.hpp"
namespace elysium::ui {
std::uint64_t BuildMenuStateCollection::idOf(const BuildMenuState& v) noexcept { return static_cast<std::uint64_t>(v.viewId); }
bool BuildMenuStateCollection::store(BuildMenuState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const BuildMenuState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool BuildMenuStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const BuildMenuState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const BuildMenuState* BuildMenuStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const BuildMenuState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
