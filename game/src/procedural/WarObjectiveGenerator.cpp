#include "procedural/WarObjectiveGenerator.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Generate context-specific strategic war goals from claims, grievances, resources, threats, ideology, and history.
bool WarObjectiveGeneratorSystem::submit(const WarObjectiveGeneratorCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const WarObjectiveGeneratorState* WarObjectiveGeneratorSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<WarObjectiveGeneratorState> WarObjectiveGeneratorSystem::states() const { std::vector<WarObjectiveGeneratorState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool WarObjectiveGeneratorSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void WarObjectiveGeneratorSystem::clear() { map_.clear(); revision_=1; }
}
