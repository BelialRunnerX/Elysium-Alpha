// Intended function: Translate stockpile/external haul requests into drone jobs while preserving stable item/container/dependency identities.
#include "DroneHaulBridge.hpp"
namespace elysium::integration {
std::uint64_t DroneHaulIntentIndex::keyOf(const DroneHaulIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool DroneHaulIntentIndex::upsert(DroneHaulIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DroneHaulIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool DroneHaulIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DroneHaulIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const DroneHaulIntent* DroneHaulIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DroneHaulIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
