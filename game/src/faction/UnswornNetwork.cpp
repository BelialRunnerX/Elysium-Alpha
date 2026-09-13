// Intended function: Track Unsworn haven reputation, smuggling access, mutual-aid links, safehouses, favors, and suspicion-decay support.
#include "UnswornNetwork.hpp"
namespace elysium::faction {
std::uint64_t UnswornContactTable::keyOf(const UnswornContact& v) noexcept { return static_cast<std::uint64_t>(v.contactId); }
bool UnswornContactTable::set(UnswornContact v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const UnswornContact& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool UnswornContactTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const UnswornContact& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const UnswornContact* UnswornContactTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const UnswornContact& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
