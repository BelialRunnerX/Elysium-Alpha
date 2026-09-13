// Intended function: Predict bounded near-term weather transitions from authoritative climate/weather state for planning and UI.
#include "WeatherForecast.hpp"
namespace elysium::climate {
std::uint64_t WeatherForecastStateStore::idOf(const WeatherForecastState& v) noexcept { return static_cast<std::uint64_t>(v.forecastId); }
bool WeatherForecastStateStore::put(WeatherForecastState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const WeatherForecastState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool WeatherForecastStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const WeatherForecastState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const WeatherForecastState* WeatherForecastStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const WeatherForecastState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::climate
