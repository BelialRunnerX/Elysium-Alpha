// Intended function: Track worker/thread role labels and diagnostic identities without allowing thread identity to influence deterministic simulation.
#include "ThreadNaming.hpp"
namespace elysium::platform {
std::uint64_t ThreadRoleCollection::idOf(const ThreadRole& v) noexcept { return static_cast<std::uint64_t>(v.roleId); }
bool ThreadRoleCollection::store(ThreadRole v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ThreadRole& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ThreadRoleCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ThreadRole& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const ThreadRole* ThreadRoleCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ThreadRole& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
