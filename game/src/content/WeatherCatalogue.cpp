// Intended function: Provide stable weather definitions, transitions, hazard multipliers, visibility, precipitation, and VFX/audio tags.
#include "WeatherCatalogue.hpp"
namespace elysium::content {
std::uint64_t WeatherRecordRegistry::key(const WeatherRecord& r) noexcept { return static_cast<std::uint64_t>(r.weatherId); }
bool WeatherRecordRegistry::publish(WeatherRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const WeatherRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool WeatherRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const WeatherRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const WeatherRecord* WeatherRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const WeatherRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::content
