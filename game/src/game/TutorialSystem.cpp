// Intended function: Track contextual onboarding prompts for movement, mining, shelter, power, crafting, automation, combat, travel, and fortress commands.
#include "TutorialSystem.hpp"
namespace elysium::game {
std::uint64_t TutorialStateCollection::idOf(const TutorialState& v) noexcept { return static_cast<std::uint64_t>(v.tutorialId); }
bool TutorialStateCollection::store(TutorialState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TutorialState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool TutorialStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TutorialState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const TutorialState* TutorialStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TutorialState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
