// Intended function: Project stable read-only blocker/reason/fact models for citizens, jobs, rooms, items, machines, military, trade, and history.
#include "InspectorModels.hpp"
namespace elysium::ui {
std::uint64_t InspectorPanelTable::keyOf(const InspectorPanel& v) noexcept { return static_cast<std::uint64_t>(v.panelId); }
bool InspectorPanelTable::set(InspectorPanel v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InspectorPanel& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool InspectorPanelTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InspectorPanel& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const InspectorPanel* InspectorPanelTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InspectorPanel& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
