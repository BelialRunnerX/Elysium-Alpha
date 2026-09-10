// Intended function: Sample deterministic galaxy/planet/chunk/worldgen inputs and outputs with named seed streams and compatibility fingerprints.
#include "WorldgenInspector.hpp"
namespace elysium::tools {
std::uint64_t WorldgenInspectionRecordIndex::keyOf(const WorldgenInspectionRecord& v) noexcept { return static_cast<std::uint64_t>(v.recordId); }
bool WorldgenInspectionRecordIndex::upsert(WorldgenInspectionRecord v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WorldgenInspectionRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool WorldgenInspectionRecordIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WorldgenInspectionRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const WorldgenInspectionRecord* WorldgenInspectionRecordIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WorldgenInspectionRecord& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
