// Intended function: Capture crash/session metadata, build identity, recent log/event hashes, save slot, and optional diagnostic attachments.
#include "CrashReport.hpp"
namespace elysium::platform {
std::uint64_t CrashRecordCollection::idOf(const CrashRecord& v) noexcept { return static_cast<std::uint64_t>(v.crashId); }
bool CrashRecordCollection::store(CrashRecord v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CrashRecord& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool CrashRecordCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CrashRecord& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const CrashRecord* CrashRecordCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CrashRecord& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
