// Intended function: Inspect versioned save sections, stable IDs, generator fingerprints, checksums, tombstones, migrations, and dependency closure.
#include "SaveInspector.hpp"
namespace elysium::tools {
std::uint64_t SaveInspectionRecordIndex::keyOf(const SaveInspectionRecord& v) noexcept { return static_cast<std::uint64_t>(v.recordId); }
bool SaveInspectionRecordIndex::upsert(SaveInspectionRecord v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SaveInspectionRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SaveInspectionRecordIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SaveInspectionRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const SaveInspectionRecord* SaveInspectionRecordIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SaveInspectionRecord& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
