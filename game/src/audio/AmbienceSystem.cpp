// Intended function: Select layered ambient loops from biome, weather, machinery, settlement activity, danger, and interior/exterior context.
#include "AmbienceSystem.hpp"
namespace elysium::audio {
std::uint64_t AmbienceLayerRegistry::key(const AmbienceLayer& r) noexcept { return static_cast<std::uint64_t>(r.layerId); }
bool AmbienceLayerRegistry::publish(AmbienceLayer r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const AmbienceLayer& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool AmbienceLayerRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const AmbienceLayer& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const AmbienceLayer* AmbienceLayerRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const AmbienceLayer& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::audio
