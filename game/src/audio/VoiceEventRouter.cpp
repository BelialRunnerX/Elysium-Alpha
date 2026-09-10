// Intended function: Route dialogue/combat/alert/effort voice events by speaker profile, priority, cooldown, subtitle key, and localization state.
#include "VoiceEventRouter.hpp"
namespace elysium::audio {
std::uint64_t VoiceEventCollection::idOf(const VoiceEvent& v) noexcept { return static_cast<std::uint64_t>(v.eventId); }
bool VoiceEventCollection::store(VoiceEvent v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VoiceEvent& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool VoiceEventCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VoiceEvent& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const VoiceEvent* VoiceEventCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VoiceEvent& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
