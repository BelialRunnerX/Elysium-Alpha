// Intended function: Project searchable figures, sites, artifacts, battles, migrations, institutions, discoveries, and cross-linked event timelines.
#include "ChronicleScreen.hpp"
namespace elysium::ui {
std::uint64_t ChronicleViewStateTable::keyOf(const ChronicleViewState& v) noexcept { return static_cast<std::uint64_t>(v.viewId); }
bool ChronicleViewStateTable::set(ChronicleViewState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ChronicleViewState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ChronicleViewStateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ChronicleViewState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const ChronicleViewState* ChronicleViewStateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ChronicleViewState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
