// Intended function: Gather immutable committed subsystem snapshots into versioned save publication envelopes with stable section identity.
#include "SaveSnapshotBridge.hpp"
namespace elysium::integration {
std::uint64_t SaveSectionIntentIndex::keyOf(const SaveSectionIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool SaveSectionIntentIndex::upsert(SaveSectionIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SaveSectionIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SaveSectionIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SaveSectionIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const SaveSectionIntent* SaveSectionIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SaveSectionIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
