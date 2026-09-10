// Intended function: Stage ECS citizen/job/room/stockpile/institution/governance/military commands for owner-thread structural commit.
#include "SettlementSystems.hpp"
namespace elysium::ecs {
std::uint64_t SettlementCommandCollection::idOf(const SettlementCommand& v) noexcept { return static_cast<std::uint64_t>(v.commandId); }
bool SettlementCommandCollection::store(SettlementCommand v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SettlementCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SettlementCommandCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SettlementCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const SettlementCommand* SettlementCommandCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SettlementCommand& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
