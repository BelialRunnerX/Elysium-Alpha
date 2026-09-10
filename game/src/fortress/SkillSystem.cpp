// Intended function: Track simulation skills, practice, learning rate, effective penalties, mentorship, and profession-facing proficiency.
#include "SkillSystem.hpp"
namespace elysium::fortress {
std::uint64_t SkillStateStore::idOf(const SkillState& v) noexcept { return static_cast<std::uint64_t>(v.citizenId); }
bool SkillStateStore::put(SkillState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SkillState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool SkillStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SkillState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const SkillState* SkillStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SkillState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::fortress
