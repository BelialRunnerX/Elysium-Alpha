// Intended function: Translate stable item ownership/provenance/access facts into theft, contraband, evidence, custody, and restitution intents.
#include "JusticeOwnershipBridge.hpp"
namespace elysium::integration {
std::uint64_t JusticeEvidenceIntentIndex::keyOf(const JusticeEvidenceIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool JusticeEvidenceIntentIndex::upsert(JusticeEvidenceIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const JusticeEvidenceIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool JusticeEvidenceIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const JusticeEvidenceIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const JusticeEvidenceIntent* JusticeEvidenceIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const JusticeEvidenceIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
