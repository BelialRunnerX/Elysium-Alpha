// Intended function: Represent radial/local gravity frames, tangent basis identity, gravity strength, and transitions for spherical planetary play.
#include "GravityFrame.hpp"
namespace elysium::physics {
std::uint64_t GravityFrameStateIndex::keyOf(const GravityFrameState& v) noexcept { return static_cast<std::uint64_t>(v.frameId); }
bool GravityFrameStateIndex::upsert(GravityFrameState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const GravityFrameState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool GravityFrameStateIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const GravityFrameState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const GravityFrameState* GravityFrameStateIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const GravityFrameState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
