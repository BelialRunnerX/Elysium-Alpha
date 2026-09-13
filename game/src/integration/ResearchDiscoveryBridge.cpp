// Intended function: Translate scans, experiments, specimen analysis, and research completion into discoveries, unlocks, maps, and Chronicle events.
#include "ResearchDiscoveryBridge.hpp"
namespace elysium::integration {
std::uint64_t DiscoveryIntentIndex::keyOf(const DiscoveryIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool DiscoveryIntentIndex::upsert(DiscoveryIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DiscoveryIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool DiscoveryIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DiscoveryIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const DiscoveryIntent* DiscoveryIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DiscoveryIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
