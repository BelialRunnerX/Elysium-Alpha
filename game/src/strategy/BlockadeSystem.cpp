#include "strategy/BlockadeSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track blockade coverage, interception probability, smuggling, shortages, relief attempts, and diplomatic consequences.
bool BlockadeSystemSystem::submit(const BlockadeSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const BlockadeSystemState* BlockadeSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<BlockadeSystemState> BlockadeSystemSystem::states() const { std::vector<BlockadeSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool BlockadeSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void BlockadeSystemSystem::clear() { map_.clear(); revision_=1; }
}
