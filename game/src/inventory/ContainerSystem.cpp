// Intended function: Track stable item containers, capacities, filters, ownership, sealing, hazard policy, access, and nested-container restrictions.
#include "ContainerSystem.hpp"
namespace elysium::inventory {
std::uint64_t ContainerStateIndex::keyOf(const ContainerState& v) noexcept { return static_cast<std::uint64_t>(v.containerId); }
bool ContainerStateIndex::upsert(ContainerState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ContainerState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ContainerStateIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ContainerState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const ContainerState* ContainerStateIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ContainerState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
