// Intended function: Manage detention cells, custody assignments, sentence clocks, transfer/exile requests, and prisoner welfare.
#include "PrisonSystem.hpp"
namespace elysium::fortress {
std::uint64_t CustodyStateStore::idOf(const CustodyState& v) noexcept { return static_cast<std::uint64_t>(v.prisonerId); }
bool CustodyStateStore::put(CustodyState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const CustodyState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool CustodyStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const CustodyState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const CustodyState* CustodyStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const CustodyState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::fortress
