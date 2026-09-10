// Intended function: Translate claim/beacon actions, expansion, extraction, violations, and destruction into system-local Suspicion/Favor/Empire consequences.
#include "ClaimEmpireBridge.hpp"
namespace elysium::integration {
std::uint64_t ClaimConsequenceIntentIndex::keyOf(const ClaimConsequenceIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool ClaimConsequenceIntentIndex::upsert(ClaimConsequenceIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ClaimConsequenceIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ClaimConsequenceIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ClaimConsequenceIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const ClaimConsequenceIntent* ClaimConsequenceIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ClaimConsequenceIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
