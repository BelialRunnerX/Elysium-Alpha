// Intended function: Stage ECS combat intents, damage applications, deaths, effect commands, and deterministic structural commit requests.
#include "CombatSystems.hpp"
namespace elysium::ecs {
std::uint64_t CombatCommandCollection::idOf(const CombatCommand& v) noexcept { return static_cast<std::uint64_t>(v.commandId); }
bool CombatCommandCollection::store(CombatCommand v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CombatCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool CombatCommandCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CombatCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const CombatCommand* CombatCommandCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CombatCommand& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
