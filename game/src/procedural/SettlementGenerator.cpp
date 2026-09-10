// Intended function: Generate deterministic settlement site plans, districts, infrastructure anchors, resource context, faction, and history seeds.
#include "SettlementGenerator.hpp"
namespace elysium::procedural {
std::uint64_t SettlementSeedTable::keyOf(const SettlementSeed& v) noexcept { return static_cast<std::uint64_t>(v.siteId); }
bool SettlementSeedTable::set(SettlementSeed v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SettlementSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SettlementSeedTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SettlementSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const SettlementSeed* SettlementSeedTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SettlementSeed& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
