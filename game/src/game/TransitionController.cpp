// Intended function: Coordinate loading/streaming transitions between sites, planets, orbit, systems, retire/reclaim, and save reload boundaries.
#include "TransitionController.hpp"
namespace elysium::game {
std::uint64_t TransitionStateCollection::idOf(const TransitionState& v) noexcept { return static_cast<std::uint64_t>(v.transitionId); }
bool TransitionStateCollection::store(TransitionState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TransitionState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool TransitionStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TransitionState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const TransitionState* TransitionStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TransitionState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
