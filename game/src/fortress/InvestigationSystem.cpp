// Intended function: Track cases, evidence, witness statements, sensor records, suspects, confidence, investigator, and verdict readiness.
#include "InvestigationSystem.hpp"
namespace elysium::fortress {
std::uint64_t InvestigationCaseStore::idOf(const InvestigationCase& v) noexcept { return static_cast<std::uint64_t>(v.caseId); }
bool InvestigationCaseStore::put(InvestigationCase v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const InvestigationCase& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool InvestigationCaseStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const InvestigationCase& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const InvestigationCase* InvestigationCaseStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const InvestigationCase& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::fortress
