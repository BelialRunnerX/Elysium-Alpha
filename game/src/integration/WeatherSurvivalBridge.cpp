// Intended function: Translate authoritative weather/climate hazards into player/citizen exposure inputs and preparation/readiness diagnostics.
#include "WeatherSurvivalBridge.hpp"
namespace elysium::integration {
std::uint64_t WeatherExposureIntentIndex::keyOf(const WeatherExposureIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool WeatherExposureIntentIndex::upsert(WeatherExposureIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WeatherExposureIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool WeatherExposureIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WeatherExposureIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const WeatherExposureIntent* WeatherExposureIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WeatherExposureIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
