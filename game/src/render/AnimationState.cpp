// Intended function: Represent animation graph state, locomotion, action montage, hit reaction, equipment pose, facial state, and blend parameters.
#include "AnimationState.hpp"
namespace elysium::render {
std::uint64_t AnimationGraphStateCollection::idOf(const AnimationGraphState& v) noexcept { return static_cast<std::uint64_t>(v.stableId); }
bool AnimationGraphStateCollection::store(AnimationGraphState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AnimationGraphState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool AnimationGraphStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AnimationGraphState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const AnimationGraphState* AnimationGraphStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AnimationGraphState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
