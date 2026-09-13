// Intended function: Project discoveries, claims, sites, routes, hazards, missions, and historical layers into a renderer-neutral map model.
#include "MapViewModel.hpp"
namespace elysium::ui {
std::uint64_t MapMarkerRegistry::key(const MapMarker& r) noexcept { return static_cast<std::uint64_t>(r.markerId); }
bool MapMarkerRegistry::publish(MapMarker r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MapMarker& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool MapMarkerRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MapMarker& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const MapMarker* MapMarkerRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MapMarker& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::ui
