// Intended function: Track current save/session, active system/planet/site, player stable identity, simulation clocks, pause, and transition state.
#include "SessionState.hpp"
namespace elysium::game {
std::uint64_t SessionSnapshotStore::idOf(const SessionSnapshot& v) noexcept { return static_cast<std::uint64_t>(v.sessionId); }
bool SessionSnapshotStore::put(SessionSnapshot v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SessionSnapshot& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool SessionSnapshotStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SessionSnapshot& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const SessionSnapshot* SessionSnapshotStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SessionSnapshot& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::game
