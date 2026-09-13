// Intended function: Project weather into sky, fog, precipitation, wind effects, lighting, wetness, particles, and audio cue parameters.
#include "WeatherPresentation.hpp"
namespace elysium::render {
std::uint64_t WeatherRenderStateCollection::idOf(const WeatherRenderState& v) noexcept { return static_cast<std::uint64_t>(v.weatherId); }
bool WeatherRenderStateCollection::store(WeatherRenderState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WeatherRenderState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool WeatherRenderStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WeatherRenderState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const WeatherRenderState* WeatherRenderStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WeatherRenderState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
