// Intended function: Track camera pose, local gravity frame, third/first person mode, zoom, smoothing, collision, floating origin, and transitions.
#include "CameraSystem.hpp"
namespace elysium::render {
std::uint64_t CameraStateCollection::idOf(const CameraState& v) noexcept { return static_cast<std::uint64_t>(v.cameraId); }
bool CameraStateCollection::store(CameraState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CameraState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool CameraStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CameraState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const CameraState* CameraStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CameraState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
