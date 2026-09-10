#include "fortress/MegaProjectSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Represent multi-stage megaprojects with districts, logistics, workforce, utilities, research, milestones, and history policy.
bool MegaProjectSystemSystem::submit(const MegaProjectSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const MegaProjectSystemState* MegaProjectSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<MegaProjectSystemState> MegaProjectSystemSystem::states() const { std::vector<MegaProjectSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool MegaProjectSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void MegaProjectSystemSystem::clear() { map_.clear(); revision_=1; }
}
