// Intended function: Generate compact pre-simulation history seeds for founding, migration, wars, sites, artifacts, rulers, and institutional changes.
#include "HistorySeed.hpp"
namespace elysium::procedural {
std::uint64_t HistorySeedRecordTable::keyOf(const HistorySeedRecord& v) noexcept { return static_cast<std::uint64_t>(v.eventId); }
bool HistorySeedRecordTable::set(HistorySeedRecord v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const HistorySeedRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool HistorySeedRecordTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const HistorySeedRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const HistorySeedRecord* HistorySeedRecordTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const HistorySeedRecord& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
