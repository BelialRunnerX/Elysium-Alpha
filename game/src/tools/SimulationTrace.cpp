// Intended function: Capture bounded per-tick simulation phase timings, decisions, commands, conflicts, state revisions, and reason codes.
#include "SimulationTrace.hpp"
namespace elysium::tools {
std::uint64_t SimulationTraceRecordIndex::keyOf(const SimulationTraceRecord& v) noexcept { return static_cast<std::uint64_t>(v.recordId); }
bool SimulationTraceRecordIndex::upsert(SimulationTraceRecord v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SimulationTraceRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SimulationTraceRecordIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SimulationTraceRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const SimulationTraceRecord* SimulationTraceRecordIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SimulationTraceRecord& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
