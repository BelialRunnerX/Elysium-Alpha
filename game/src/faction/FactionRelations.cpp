// Intended function: Track normalized bilateral standing, treaties, hostilities, trade access, borders, guarantees, and historical grievance pressure.
#include "FactionRelations.hpp"
namespace elysium::faction {
std::uint64_t FactionRelationTable::keyOf(const FactionRelation& v) noexcept { return static_cast<std::uint64_t>(v.relationId); }
bool FactionRelationTable::set(FactionRelation v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FactionRelation& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool FactionRelationTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FactionRelation& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const FactionRelation* FactionRelationTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FactionRelation& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
