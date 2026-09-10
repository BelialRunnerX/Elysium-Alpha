#include "replay/ReplayPlayback.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Feed recorded deterministic commands back into simulation while exposing divergence checkpoints and stop conditions.
bool ReplayPlaybackStore::apply(const ReplayPlaybackOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool ReplayPlaybackStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const ReplayPlaybackData* ReplayPlaybackStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<ReplayPlaybackData> ReplayPlaybackStore::snapshot() const { std::vector<ReplayPlaybackData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void ReplayPlaybackStore::clear() { data_.clear(); revision_=1; }
}
