// Intended function: Route immutable simulation events by stable subject/source/category without granting subscribers mutation authority.
#include "EventRouter.hpp"
namespace elysium::simulation {
std::uint64_t SimEventTable::keyOf(const SimEvent& v) noexcept { return static_cast<std::uint64_t>(v.eventId); }
bool SimEventTable::set(SimEvent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SimEvent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SimEventTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SimEvent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const SimEvent* SimEventTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SimEvent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
