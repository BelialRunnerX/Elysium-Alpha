// Intended function: Prioritize casualties by airway, bleeding, shock, pain, infection risk, mobility, and available treatment capacity.
#include "TriageSystem.hpp"
namespace elysium::medical {
std::uint64_t TriageRecordStore::idOf(const TriageRecord& v) noexcept { return static_cast<std::uint64_t>(v.patientId); }
bool TriageRecordStore::put(TriageRecord v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const TriageRecord& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool TriageRecordStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const TriageRecord& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const TriageRecord* TriageRecordStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const TriageRecord& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::medical
