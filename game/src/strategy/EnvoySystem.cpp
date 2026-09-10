// Intended function: Represent faction/Court envoys, agendas, demands, gifts, negotiation state, hospitality, and diplomatic consequences.
#include "EnvoySystem.hpp"
namespace elysium::strategy {
std::uint64_t EnvoyStateStore::idOf(const EnvoyState& v) noexcept { return static_cast<std::uint64_t>(v.envoyId); }
bool EnvoyStateStore::put(EnvoyState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const EnvoyState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool EnvoyStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const EnvoyState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const EnvoyState* EnvoyStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const EnvoyState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::strategy
