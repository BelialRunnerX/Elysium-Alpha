// Intended function: Select adaptive music states from exploration, combat, fortress crises, Imperial pressure, Rift threat, and story beats.
#include "MusicDirector.hpp"
namespace elysium::audio {
std::uint64_t MusicStateRegistry::key(const MusicState& r) noexcept { return static_cast<std::uint64_t>(r.stateId); }
bool MusicStateRegistry::publish(MusicState r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MusicState& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool MusicStateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MusicState& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const MusicState* MusicStateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MusicState& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::audio
