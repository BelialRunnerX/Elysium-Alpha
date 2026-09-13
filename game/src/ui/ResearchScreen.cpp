// Intended function: Project available research, prerequisites, experiments, specimens, apparatus, confidence, unlocks, and archive links.
#include "ResearchScreen.hpp"
namespace elysium::ui {
std::uint64_t ResearchViewStateTable::keyOf(const ResearchViewState& v) noexcept { return static_cast<std::uint64_t>(v.viewId); }
bool ResearchViewStateTable::set(ResearchViewState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ResearchViewState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ResearchViewStateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ResearchViewState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const ResearchViewState* ResearchViewStateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ResearchViewState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
