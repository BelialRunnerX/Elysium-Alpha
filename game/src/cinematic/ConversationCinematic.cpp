#include "cinematic/ConversationCinematic.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Build presentation cues for dialogue scenes from participants, relationship, location, mood, and authored beats.
bool ConversationCinematicStore::apply(const ConversationCinematicOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool ConversationCinematicStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const ConversationCinematicData* ConversationCinematicStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<ConversationCinematicData> ConversationCinematicStore::snapshot() const { std::vector<ConversationCinematicData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void ConversationCinematicStore::clear() { data_.clear(); revision_=1; }
}
