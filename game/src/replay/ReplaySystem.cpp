// Intended function: Record deterministic gameplay commands, world revisions, seeds, checkpoints, and replay cursors for debugging and demos.
#include "ReplaySystem.hpp"
namespace elysium::replay {
std::uint64_t ReplayFrameStore::idOf(const ReplayFrame& v) noexcept { return static_cast<std::uint64_t>(v.frameId); }
bool ReplayFrameStore::put(ReplayFrame v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const ReplayFrame& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool ReplayFrameStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const ReplayFrame& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const ReplayFrame* ReplayFrameStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const ReplayFrame& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::replay
