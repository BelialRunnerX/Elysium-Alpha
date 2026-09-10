// Intended function: Track settlement/strategic policies for labor, rationing, trade, law, military posture, immigration, and emergency response.
#include "PolicySystem.hpp"
namespace elysium::strategy {
std::uint64_t PolicyStateStore::idOf(const PolicyState& v) noexcept { return static_cast<std::uint64_t>(v.policyId); }
bool PolicyStateStore::put(PolicyState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const PolicyState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool PolicyStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const PolicyState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const PolicyState* PolicyStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const PolicyState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::strategy
