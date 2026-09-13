// Intended function: Route presentation events into bounded sound-effect requests by material, weapon, machine, UI, weather, creature, and spatial context.
#include "SfxRouter.hpp"
namespace elysium::audio {
std::uint64_t SfxEventCollection::idOf(const SfxEvent& v) noexcept { return static_cast<std::uint64_t>(v.eventId); }
bool SfxEventCollection::store(SfxEvent v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SfxEvent& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SfxEventCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SfxEvent& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const SfxEvent* SfxEventCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SfxEvent& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
