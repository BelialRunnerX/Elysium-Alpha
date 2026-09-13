// Intended function: Resolve deterministic mod dependency graphs, versions, optional dependencies, load order, conflicts, and namespaces.
#include "ModDependencyResolver.hpp"
namespace elysium::mod {
std::uint64_t ModDependencyStateCollection::idOf(const ModDependencyState& v) noexcept { return static_cast<std::uint64_t>(v.modId); }
bool ModDependencyStateCollection::store(ModDependencyState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ModDependencyState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ModDependencyStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ModDependencyState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const ModDependencyState* ModDependencyStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ModDependencyState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
