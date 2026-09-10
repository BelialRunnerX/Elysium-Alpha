// Intended function: Generate procedural flora species from body grammar, climate niche, resource traits, defenses, growth form, and reproductive strategy.
#include "FloraGenerator.hpp"
namespace elysium::procedural {
std::uint64_t FloraSeedTable::keyOf(const FloraSeed& v) noexcept { return static_cast<std::uint64_t>(v.speciesId); }
bool FloraSeedTable::set(FloraSeed v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FloraSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool FloraSeedTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FloraSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const FloraSeed* FloraSeedTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FloraSeed& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
