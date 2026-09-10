// Intended function: Queue deterministic stable-ID commands sorted by kind/key/sequence rather than worker arrival order.
#include "StableCommandQueue.hpp"
namespace elysium::simulation {
std::uint64_t StableCommandTable::keyOf(const StableCommand& v) noexcept { return static_cast<std::uint64_t>(v.commandId); }
bool StableCommandTable::set(StableCommand v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const StableCommand& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool StableCommandTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const StableCommand& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const StableCommand* StableCommandTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const StableCommand& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
