// Intended function: Stage ECS research/experiment/knowledge/discovery commands while keeping archives and registries stable-ID based.
#include "ResearchSystems.hpp"
namespace elysium::ecs {
std::uint64_t ResearchCommandCollection::idOf(const ResearchCommand& v) noexcept { return static_cast<std::uint64_t>(v.commandId); }
bool ResearchCommandCollection::store(ResearchCommand v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ResearchCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ResearchCommandCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ResearchCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const ResearchCommand* ResearchCommandCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ResearchCommand& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
