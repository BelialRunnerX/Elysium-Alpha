// Intended function: Coordinate Direct Operative, Fortress Command, Tactical Command, Star-System Strategy, and Chronicle mode transitions.
#include "ModeController.hpp"
namespace elysium::game {
std::uint64_t ModeStateCollection::idOf(const ModeState& v) noexcept { return static_cast<std::uint64_t>(v.sessionId); }
bool ModeStateCollection::store(ModeState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ModeState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ModeStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ModeState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const ModeState* ModeStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ModeState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
