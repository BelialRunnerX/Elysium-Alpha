// Intended function: Represent platform services such as clipboard, URL open, user paths, locale, input devices, and display capabilities.
#include "PlatformServices.hpp"
namespace elysium::platform {
std::uint64_t PlatformCapabilityCollection::idOf(const PlatformCapability& v) noexcept { return static_cast<std::uint64_t>(v.capabilityId); }
bool PlatformCapabilityCollection::store(PlatformCapability v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PlatformCapability& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool PlatformCapabilityCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PlatformCapability& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const PlatformCapability* PlatformCapabilityCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PlatformCapability& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
