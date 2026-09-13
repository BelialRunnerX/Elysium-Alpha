// Intended function: Derive citizen professions from skills, assigned work, social role, institutions, and historical identity.
#include "ProfessionSystem.hpp"
namespace elysium::fortress {
std::uint64_t ProfessionStateStore::idOf(const ProfessionState& v) noexcept { return static_cast<std::uint64_t>(v.citizenId); }
bool ProfessionStateStore::put(ProfessionState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const ProfessionState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool ProfessionStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const ProfessionState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const ProfessionState* ProfessionStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const ProfessionState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::fortress
