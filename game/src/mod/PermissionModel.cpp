// Intended function: Define bounded mod permissions for data registration, presentation hooks, commands, file access, scripting, and multiplayer compatibility.
#include "PermissionModel.hpp"
namespace elysium::mod {
std::uint64_t ModPermissionStateCollection::idOf(const ModPermissionState& v) noexcept { return static_cast<std::uint64_t>(v.modId); }
bool ModPermissionStateCollection::store(ModPermissionState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ModPermissionState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ModPermissionStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ModPermissionState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const ModPermissionState* ModPermissionStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ModPermissionState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
