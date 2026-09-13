// Intended function: Represent future client/server stable command envelopes with sequence, authority epoch, validation result, and reconciliation tick.
#include "CommandReplication.hpp"
namespace elysium::network {
std::uint64_t ReplicatedCommandCollection::idOf(const ReplicatedCommand& v) noexcept { return static_cast<std::uint64_t>(v.commandId); }
bool ReplicatedCommandCollection::store(ReplicatedCommand v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ReplicatedCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ReplicatedCommandCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ReplicatedCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const ReplicatedCommand* ReplicatedCommandCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ReplicatedCommand& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
