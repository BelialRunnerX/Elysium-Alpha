// Intended function: Score bounded action candidates using needs, danger, orders, opportunity, skill, relationships, and deterministic tie-breaks.
#include "UtilityAi.hpp"
namespace elysium::ai {
std::uint64_t UtilityCandidateTable::keyOf(const UtilityCandidate& v) noexcept { return static_cast<std::uint64_t>(v.candidateId); }
bool UtilityCandidateTable::set(UtilityCandidate v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const UtilityCandidate& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool UtilityCandidateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const UtilityCandidate& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const UtilityCandidate* UtilityCandidateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const UtilityCandidate& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
