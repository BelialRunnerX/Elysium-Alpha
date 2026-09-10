// Intended function: Project versioned game/accessibility/audio/graphics/input/simulation settings with validation ranges and restart requirements.
#include "SettingsScreen.hpp"
namespace elysium::ui {
std::uint64_t SettingsViewStateCollection::idOf(const SettingsViewState& v) noexcept { return static_cast<std::uint64_t>(v.viewId); }
bool SettingsViewStateCollection::store(SettingsViewState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SettingsViewState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SettingsViewStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SettingsViewState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const SettingsViewState* SettingsViewStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SettingsViewState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
