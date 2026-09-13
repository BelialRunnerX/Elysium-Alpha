// Intended function: Represent bounded presentation/physics debris chunks from structural collapse, mining, combat, and vehicle damage with lifetime caps.
#include "DebrisSystem.hpp"
namespace elysium::physics {
std::uint64_t DebrisStateIndex::keyOf(const DebrisState& v) noexcept { return static_cast<std::uint64_t>(v.debrisId); }
bool DebrisStateIndex::upsert(DebrisState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DebrisState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool DebrisStateIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DebrisState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const DebrisState* DebrisStateIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DebrisState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
