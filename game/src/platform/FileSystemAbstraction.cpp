// Intended function: Represent platform-independent save/config/cache/mod/log paths and atomic file operation requests.
#include "FileSystemAbstraction.hpp"
namespace elysium::platform {
std::uint64_t FileOperationCollection::idOf(const FileOperation& v) noexcept { return static_cast<std::uint64_t>(v.operationId); }
bool FileOperationCollection::store(FileOperation v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FileOperation& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool FileOperationCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FileOperation& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const FileOperation* FileOperationCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FileOperation& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
