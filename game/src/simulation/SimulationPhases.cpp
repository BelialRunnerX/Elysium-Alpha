// Intended function: Coordinate Sense, Plan, Resolve, Commit, Persist, and Present phase sequencing with deterministic tick identity.
#include "SimulationPhases.hpp"
namespace elysium::simulation {
std::uint64_t PhaseRecordTable::keyOf(const PhaseRecord& v) noexcept { return static_cast<std::uint64_t>(v.tick); }
bool PhaseRecordTable::set(PhaseRecord v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PhaseRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool PhaseRecordTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PhaseRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const PhaseRecord* PhaseRecordTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PhaseRecord& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
