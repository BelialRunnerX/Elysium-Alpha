// Intended function: Represent Imperial Code rules, registration requirements, contraband policy, inspection powers, warrants, and enforcement severity.
#include "ImperialLaw.hpp"
namespace elysium::faction {
std::uint64_t ImperialLawRecordTable::keyOf(const ImperialLawRecord& v) noexcept { return static_cast<std::uint64_t>(v.lawId); }
bool ImperialLawRecordTable::set(ImperialLawRecord v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ImperialLawRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ImperialLawRecordTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ImperialLawRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const ImperialLawRecord* ImperialLawRecordTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ImperialLawRecord& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
