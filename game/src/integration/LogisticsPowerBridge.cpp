// Intended function: Gate machine/logistics transfer and processing intents on bounded local power availability without consuming inputs on brownout.
#include "LogisticsPowerBridge.hpp"
namespace elysium::integration {
std::uint64_t PoweredTransferIntentIndex::keyOf(const PoweredTransferIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool PoweredTransferIntentIndex::upsert(PoweredTransferIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PoweredTransferIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool PoweredTransferIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PoweredTransferIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const PoweredTransferIntent* PoweredTransferIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PoweredTransferIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
