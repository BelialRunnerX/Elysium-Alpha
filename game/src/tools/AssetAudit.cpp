// Intended function: Track authored/generated asset references, missing assets, duplicate stable names, material usage, LOD coverage, and packaging state.
#include "AssetAudit.hpp"
namespace elysium::tools {
std::uint64_t AssetAuditRecordIndex::keyOf(const AssetAuditRecord& v) noexcept { return static_cast<std::uint64_t>(v.recordId); }
bool AssetAuditRecordIndex::upsert(AssetAuditRecord v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AssetAuditRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool AssetAuditRecordIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AssetAuditRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const AssetAuditRecord* AssetAuditRecordIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AssetAuditRecord& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
