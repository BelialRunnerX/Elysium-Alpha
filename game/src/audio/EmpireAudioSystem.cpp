#include "audio/EmpireAudioSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Generate Imperial announcement, Register Action, Praetor, facility, combat, and campaign musical/audio motifs.
bool EmpireAudioSystemSystem::submit(const EmpireAudioSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const EmpireAudioSystemState* EmpireAudioSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<EmpireAudioSystemState> EmpireAudioSystemSystem::states() const { std::vector<EmpireAudioSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool EmpireAudioSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void EmpireAudioSystemSystem::clear() { map_.clear(); revision_=1; }
}
