// Intended function: Represent Court envoys, favor thresholds, commissions, audiences, judgments, ceremonial obligations, and political opportunities.
#include "CourtSystem.hpp"
namespace elysium::faction {
std::uint64_t CourtStateTable::keyOf(const CourtState& v) noexcept { return static_cast<std::uint64_t>(v.recordId); }
bool CourtStateTable::set(CourtState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CourtState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool CourtStateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CourtState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const CourtState* CourtStateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CourtState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
