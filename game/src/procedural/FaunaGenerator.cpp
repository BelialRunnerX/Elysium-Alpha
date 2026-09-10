// Intended function: Generate procedural fauna from body grammar, locomotion, diet, senses, temperament, attacks, defenses, and habitat niche.
#include "FaunaGenerator.hpp"
namespace elysium::procedural {
std::uint64_t FaunaSeedTable::keyOf(const FaunaSeed& v) noexcept { return static_cast<std::uint64_t>(v.speciesId); }
bool FaunaSeedTable::set(FaunaSeed v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FaunaSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool FaunaSeedTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FaunaSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const FaunaSeed* FaunaSeedTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FaunaSeed& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
