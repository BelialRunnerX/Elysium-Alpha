// Intended function: Project pressure, smoke, fire, contamination, precipitation, fog, and decompression into bounded visual effects.
#include "AtmosphereVisuals.hpp"
namespace elysium::render {
std::uint64_t AtmosphereVisualStateRegistry::key(const AtmosphereVisualState& r) noexcept { return static_cast<std::uint64_t>(r.volumeId); }
bool AtmosphereVisualStateRegistry::publish(AtmosphereVisualState r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const AtmosphereVisualState& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool AtmosphereVisualStateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const AtmosphereVisualState& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const AtmosphereVisualState* AtmosphereVisualStateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const AtmosphereVisualState& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::render
