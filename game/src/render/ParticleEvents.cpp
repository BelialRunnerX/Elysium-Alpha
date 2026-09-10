// Intended function: Queue bounded presentation-only particle events from combat, mining, weather, machines, damage, and atmosphere.
#include "ParticleEvents.hpp"
namespace elysium::render {
std::uint64_t ParticleEventRegistry::key(const ParticleEvent& r) noexcept { return static_cast<std::uint64_t>(r.eventId); }
bool ParticleEventRegistry::publish(ParticleEvent r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ParticleEvent& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool ParticleEventRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ParticleEvent& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const ParticleEvent* ParticleEventRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ParticleEvent& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::render
