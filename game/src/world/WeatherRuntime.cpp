// Intended function: order-independent deterministic weather selection.
#include "world/WeatherRuntime.hpp"

#include "core/Determinism.hpp"
#include "core/Saturating.hpp"

#include <algorithm>
#include <cmath>

namespace elysium {
namespace {
double unit(std::uint64_t h) noexcept {
    return static_cast<double>((mix64(h)>>11U)&((1ULL<<53U)-1ULL))/static_cast<double>(1ULL<<53U);
}
}

WeatherStateSample WeatherRuntime::sample(
    std::uint64_t timeBucket,
    const PlanetFieldSample& field,
    std::span<const WeatherStateDefinition> defs) const noexcept {
    double total=0.0;
    std::vector<double> weights; weights.reserve(defs.size());
    for(const auto& d:defs){
        if(d.weatherId==0){weights.push_back(0.0);continue;}
        const double dt=field.temperature01-safe::finiteClamp(d.targetTemperature01,0.0,1.0);
        const double dm=field.moisture01-safe::finiteClamp(d.targetMoisture01,0.0,1.0);
        const double dr=field.radiation01-safe::finiteClamp(d.targetRadiation01,0.0,1.0);
        const double fit=std::exp(-(dt*dt+dm*dm+dr*dr)*3.0);
        const double w=safe::nonNegative(d.baseOccurrence)*fit;
        weights.push_back(w); total=safe::nonNegative(total+w);
    }
    if(total<=0.0 || defs.empty()) return {};
    const double roll=unit(mix64(seed_^mix64(timeBucket)^0x57454154484552ULL))*total;
    double acc=0.0; std::size_t chosen=defs.size()-1;
    for(std::size_t i=0;i<defs.size();++i){acc+=weights[i]; if(roll<=acc){chosen=i;break;}}
    const auto& d=defs[chosen];
    return {.weatherId=d.weatherId,.timeBucket=timeBucket,
        .hazardMultiplier=safe::nonNegative(d.hazardMultiplier),
        .visibilityMultiplier=safe::nonNegative(d.visibilityMultiplier),
        .precipitation01=safe::finiteClamp(d.precipitation01,0.0,1.0),
        .windMultiplier=safe::nonNegative(d.windMultiplier),.presentationTag=d.presentationTag};
}

} // namespace elysium
