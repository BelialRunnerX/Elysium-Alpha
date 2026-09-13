// Intended function: Promote/demote strategic caravans into local vehicle/cargo actors while preserving stable convoy, manifest, and ownership identity.
#include "VehicleCaravanBridge.hpp"
namespace elysium::integration {
std::uint64_t VehiclePromotionIntentIndex::keyOf(const VehiclePromotionIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool VehiclePromotionIntentIndex::upsert(VehiclePromotionIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VehiclePromotionIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool VehiclePromotionIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VehiclePromotionIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const VehiclePromotionIntent* VehiclePromotionIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VehiclePromotionIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
