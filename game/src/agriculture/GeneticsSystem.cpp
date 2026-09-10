// Intended function: Track deterministic crop/livestock genetic lines, traits, mutation, selection, breeding, and lineage identity.
#include "GeneticsSystem.hpp"
namespace elysium::agriculture {
std::uint64_t GeneticLineStore::idOf(const GeneticLine& v) noexcept { return static_cast<std::uint64_t>(v.lineId); }
bool GeneticLineStore::put(GeneticLine v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const GeneticLine& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool GeneticLineStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const GeneticLine& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const GeneticLine* GeneticLineStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const GeneticLine& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::agriculture
