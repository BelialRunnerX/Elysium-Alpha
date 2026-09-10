// Intended function: Track embodied operative skill-use progression, diminishing returns, training bonuses, unlock thresholds, and milestones.
#include "SkillProgression.hpp"
namespace elysium::gameplay {
std::uint64_t OperativeSkillStore::idOf(const OperativeSkill& v) noexcept { return static_cast<std::uint64_t>(v.skillId); }
bool OperativeSkillStore::put(OperativeSkill v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const OperativeSkill& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool OperativeSkillStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const OperativeSkill& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const OperativeSkill* OperativeSkillStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const OperativeSkill& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::gameplay
