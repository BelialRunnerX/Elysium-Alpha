// Intended function: Map durable StableIds to transient active-shard handles while preventing runtime handles from becoming save/network identity.
#include "StableIdRegistry.hpp"
namespace elysium::ecs {
std::uint64_t StableIdBindingCollection::idOf(const StableIdBinding& v) noexcept { return static_cast<std::uint64_t>(v.stableId); }
bool StableIdBindingCollection::store(StableIdBinding v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const StableIdBinding& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool StableIdBindingCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const StableIdBinding& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const StableIdBinding* StableIdBindingCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const StableIdBinding& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
