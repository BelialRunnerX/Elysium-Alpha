// Intended function: Route committed construction/destruction invalidation to navigation, rooms, atmosphere, utilities, supports, and renderer rebuild queues.
#include "StructuralInvalidationBridge.hpp"
namespace elysium::integration {
std::uint64_t InvalidationIntentIndex::keyOf(const InvalidationIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool InvalidationIntentIndex::upsert(InvalidationIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InvalidationIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool InvalidationIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InvalidationIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const InvalidationIntent* InvalidationIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InvalidationIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
