// Intended function: Track active/local/remote representation state, promotion requirements, demotion summaries, and equivalence revisions.
#include "BehavioralLod.hpp"
namespace elysium::simulation {
std::uint64_t LodEntityStateTable::keyOf(const LodEntityState& v) noexcept { return static_cast<std::uint64_t>(v.stableId); }
bool LodEntityStateTable::set(LodEntityState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const LodEntityState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool LodEntityStateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const LodEntityState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const LodEntityState* LodEntityStateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const LodEntityState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
