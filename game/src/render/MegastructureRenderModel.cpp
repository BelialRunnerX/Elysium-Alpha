#include "render/MegastructureRenderModel.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Build renderer-neutral megastructure sectors, damage, lights, construction, utilities, and distance-detail records.
bool MegastructureRenderModelSystem::submit(const MegastructureRenderModelCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const MegastructureRenderModelState* MegastructureRenderModelSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<MegastructureRenderModelState> MegastructureRenderModelSystem::states() const { std::vector<MegastructureRenderModelState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool MegastructureRenderModelSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void MegastructureRenderModelSystem::clear() { map_.clear(); revision_=1; }
}
