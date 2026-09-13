#include "gameplay/SurvivalChallengeDirector.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Generate optional escalating survival objectives from environment, supplies, route, and settlement context.
bool SurvivalChallengeDirectorSystem::submit(const SurvivalChallengeDirectorCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const SurvivalChallengeDirectorState* SurvivalChallengeDirectorSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<SurvivalChallengeDirectorState> SurvivalChallengeDirectorSystem::states() const { std::vector<SurvivalChallengeDirectorState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool SurvivalChallengeDirectorSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void SurvivalChallengeDirectorSystem::clear() { map_.clear(); revision_=1; }
}
