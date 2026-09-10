// Intended function: Stage ECS mission/objective progress, trigger, completion, failure, reward, and Chronicle publication commands.
#include "MissionSystems.hpp"
namespace elysium::ecs {
std::uint64_t MissionCommandCollection::idOf(const MissionCommand& v) noexcept { return static_cast<std::uint64_t>(v.commandId); }
bool MissionCommandCollection::store(MissionCommand v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MissionCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool MissionCommandCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MissionCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const MissionCommand* MissionCommandCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MissionCommand& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
