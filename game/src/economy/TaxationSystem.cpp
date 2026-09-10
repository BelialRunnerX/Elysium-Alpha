// Intended function: Compute optional settlement tariffs, market fees, rent/service costs, treasury income, exemptions, and policy-driven distributions.
#include "TaxationSystem.hpp"
namespace elysium::economy {
std::uint64_t TaxRecordTable::keyOf(const TaxRecord& v) noexcept { return static_cast<std::uint64_t>(v.recordId); }
bool TaxRecordTable::set(TaxRecord v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TaxRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool TaxRecordTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TaxRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const TaxRecord* TaxRecordTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TaxRecord& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
