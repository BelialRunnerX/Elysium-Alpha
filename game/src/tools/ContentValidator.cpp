// Intended function: Validate content IDs, namespaces, references, recipes, loot tables, structures, factions, localization, and mod compatibility.
#include "ContentValidator.hpp"
namespace elysium::tools {
std::uint64_t ContentValidationRecordIndex::keyOf(const ContentValidationRecord& v) noexcept { return static_cast<std::uint64_t>(v.recordId); }
bool ContentValidationRecordIndex::upsert(ContentValidationRecord v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ContentValidationRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ContentValidationRecordIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ContentValidationRecord& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const ContentValidationRecord* ContentValidationRecordIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ContentValidationRecord& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
