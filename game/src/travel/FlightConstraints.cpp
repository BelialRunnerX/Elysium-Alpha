#include "travel/FlightConstraints.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
#include <cmath>
namespace elysium::travel {
namespace {
double nonnegative(double v) noexcept { return safe::nonNegative(std::isfinite(v)?v:0.0); }
double unit(double v) noexcept { return std::clamp(nonnegative(v),0.0,1.0); }
bool legal(FlightMode a,FlightMode b) noexcept {
 if(a==b)return true;
 return (a==FlightMode::Landed&&b==FlightMode::Atmospheric)||
        (a==FlightMode::Atmospheric&&(b==FlightMode::Landed||b==FlightMode::Entry))||
        (a==FlightMode::Entry&&(b==FlightMode::Atmospheric||b==FlightMode::Orbital))||
        (a==FlightMode::Orbital&&b==FlightMode::Entry);
}
FlightBlockReason first(const FlightConstraintSnapshot&s) noexcept {return s.blockers.empty()?FlightBlockReason::None:s.blockers.front();}
}
bool FlightConstraintRuntime::create(FlightStateRecord s){if(!s.stableShipId||!s.revision)return false;s.planetDirection=elysium::normalize(s.planetDirection);return states_.emplace(s.stableShipId,s).second;}
const FlightStateRecord* FlightConstraintRuntime::find(std::uint64_t id)const{auto it=states_.find(id);return it==states_.end()?nullptr:&it->second;}
FlightConstraintSnapshot FlightConstraintRuntime::evaluate(const FlightEnvironment&e,const FlightCapabilities&c,const FlightConstraintLimits&l,bool entry)const{
 FlightConstraintSnapshot o{};
 const bool finite=std::isfinite(e.gravityMps2)&&std::isfinite(e.cargoMassKg)&&std::isfinite(e.weatherSeverity)&&std::isfinite(e.thermalEntryLoad)&&std::isfinite(e.radiationEntryLoad)&&std::isfinite(c.handlingRating)&&std::isfinite(c.thermalMitigation)&&std::isfinite(c.radiationMitigation)&&std::isfinite(l.maximumCargoMassKg)&&std::isfinite(l.maximumWeatherSeverity)&&std::isfinite(l.maximumResidualThermalLoad)&&std::isfinite(l.maximumResidualRadiationLoad)&&std::isfinite(l.gravityHandlingWeight)&&std::isfinite(l.cargoHandlingWeight)&&std::isfinite(l.weatherHandlingWeight)&&std::isfinite(l.entryHandlingWeight);
 if(!finite){o.blockers.push_back(FlightBlockReason::InvalidInput);return o;}
 o.owner=elysium::directionToFaceUv(e.planetDirection);
 const double gravity=nonnegative(e.gravityMps2),cargo=nonnegative(e.cargoMassKg),weather=nonnegative(e.weatherSeverity);
 o.residualThermalLoad=nonnegative(e.thermalEntryLoad)*(1.0-unit(c.thermalMitigation));
 o.residualRadiationLoad=nonnegative(e.radiationEntryLoad)*(1.0-unit(c.radiationMitigation));
 o.availableHandling=nonnegative(c.handlingRating)*(c.vectorControl?2.0:1.0);
 o.requiredHandling=nonnegative(gravity*nonnegative(l.gravityHandlingWeight)+cargo*nonnegative(l.cargoHandlingWeight)+weather*nonnegative(l.weatherHandlingWeight)+(entry?nonnegative(l.entryHandlingWeight):0.0));
 if(!c.atmosphericThruster)o.blockers.push_back(FlightBlockReason::MissingAtmosphericThruster);
 if(l.maximumCargoMassKg>0.0&&cargo>l.maximumCargoMassKg)o.blockers.push_back(FlightBlockReason::CargoOverLimit);
 if(l.maximumWeatherSeverity>0.0&&weather>l.maximumWeatherSeverity)o.blockers.push_back(FlightBlockReason::StormOverLimit);
 if(entry&&l.maximumResidualThermalLoad>0.0&&o.residualThermalLoad>l.maximumResidualThermalLoad)o.blockers.push_back(FlightBlockReason::ThermalEntryUnsafe);
 if(entry&&l.maximumResidualRadiationLoad>0.0&&o.residualRadiationLoad>l.maximumResidualRadiationLoad)o.blockers.push_back(FlightBlockReason::RadiationEntryUnsafe);
 if(o.availableHandling<o.requiredHandling)o.blockers.push_back(FlightBlockReason::InsufficientHandling);
 o.valid=o.blockers.empty();return o;
}
FlightTransition FlightConstraintRuntime::checkTransition(std::uint64_t id,FlightMode target,const FlightEnvironment&e,const FlightCapabilities&c,const FlightConstraintLimits&l)const{
 FlightTransition t{};auto it=states_.find(id);if(it==states_.end()){t.reason=FlightBlockReason::MissingFlight;return t;}t.from=it->second.mode;t.requested=target;t.suggestedFallback=t.from;
 if(!legal(t.from,target)){t.reason=FlightBlockReason::IllegalTransition;return t;}
 const bool atmospheric=(t.from!=FlightMode::Orbital||target!=FlightMode::Orbital)&&(t.from!=FlightMode::Landed||target!=FlightMode::Landed);
 const bool entry=(t.from==FlightMode::Entry||target==FlightMode::Entry);
 if(atmospheric)t.constraints=evaluate(e,c,l,entry);else t.constraints.valid=true;
 if(!t.constraints.valid){t.reason=first(t.constraints);t.suggestedFallback=entry?FlightMode::Orbital:t.from;return t;}
 t.allowed=true;return t;
}
bool FlightConstraintRuntime::commitTransition(std::uint64_t id,std::uint64_t rev,const FlightTransition&t){auto it=states_.find(id);if(it==states_.end()||it->second.revision!=rev||!t.allowed||t.from!=it->second.mode||t.reason!=FlightBlockReason::None)return false;it->second.mode=t.requested;it->second.planetDirection=elysium::faceUvToDirection(t.constraints.owner.face,t.constraints.owner.u,t.constraints.owner.v);it->second.revision=safe::saturatingIncrement(it->second.revision);return true;}
FlightTransition FlightConstraintRuntime::abortToOrbit(std::uint64_t id,const FlightEnvironment&e,const FlightCapabilities&c,const FlightConstraintLimits&l)const{auto it=states_.find(id);if(it==states_.end()){FlightTransition t{};t.reason=FlightBlockReason::MissingFlight;return t;}if(it->second.mode!=FlightMode::Entry){FlightTransition t{};t.from=it->second.mode;t.requested=FlightMode::Orbital;t.reason=FlightBlockReason::IllegalTransition;return t;}auto t=checkTransition(id,FlightMode::Orbital,e,c,l);if(!t.allowed){t.allowed=true;t.reason=FlightBlockReason::None;t.suggestedFallback=FlightMode::Orbital;}return t;}
FlightConstraintRuntimeSnapshot FlightConstraintRuntime::snapshot()const{FlightConstraintRuntimeSnapshot s;for(const auto&[_,v]:states_)s.states.push_back(v);return s;}
bool FlightConstraintRuntime::restore(const FlightConstraintRuntimeSnapshot&s){FlightConstraintRuntime n;for(auto v:s.states)if(!n.create(v))return false;*this=std::move(n);return true;}
} // namespace elysium::travel
