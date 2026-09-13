// Intended function: Store gameplay, accessibility, camera, UI, audio, graphics, simulation-budget, and debug settings as versioned data.
#include "SettingsState.hpp"
namespace elysium::game {
std::uint64_t SettingsRecordStore::idOf(const SettingsRecord& v) noexcept { return static_cast<std::uint64_t>(v.settingId); }
bool SettingsRecordStore::put(SettingsRecord v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SettingsRecord& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool SettingsRecordStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SettingsRecord& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const SettingsRecord* SettingsRecordStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SettingsRecord& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::game
