// Intended function: Represent stable cargo manifests with mass, volume, ownership, hazard classification, and sealing requirements.
#include "CargoManifest.hpp"
namespace elysium::travel {
std::uint64_t CargoManifestStateRegistry::key(const CargoManifestState& r) noexcept { return static_cast<std::uint64_t>(r.manifestId); }
bool CargoManifestStateRegistry::publish(CargoManifestState r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const CargoManifestState& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool CargoManifestStateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const CargoManifestState& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const CargoManifestState* CargoManifestStateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const CargoManifestState& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::travel
