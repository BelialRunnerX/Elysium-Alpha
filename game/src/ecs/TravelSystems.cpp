// Intended function: Stage ECS vehicle/ship seat, pilot, docking, warp, route, and promotion/demotion commands.
#include "TravelSystems.hpp"
namespace elysium::ecs {
std::uint64_t TravelCommandCollection::idOf(const TravelCommand& v) noexcept { return static_cast<std::uint64_t>(v.commandId); }
bool TravelCommandCollection::store(TravelCommand v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TravelCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool TravelCommandCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TravelCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const TravelCommand* TravelCommandCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TravelCommand& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
