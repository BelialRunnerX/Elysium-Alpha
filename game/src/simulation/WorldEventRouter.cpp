// Intended function: Translate committed edits, combat, trade, deaths, discoveries, migrations, and sieges into cross-system stable events.
#include "WorldEventRouter.hpp"
namespace elysium::simulation {
std::uint64_t WorldEventRecordTable::keyOf(const WorldEventRecord& v) noexcept { return static_cast<std::uint64_t>(v.eventId); }
bool WorldEventRecordTable::set(WorldEventRecord v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WorldEventRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool WorldEventRecordTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WorldEventRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const WorldEventRecord* WorldEventRecordTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WorldEventRecord& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
