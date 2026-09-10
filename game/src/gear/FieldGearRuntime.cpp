#include "gear/FieldGearRuntime.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
namespace elysium::gear {
FieldGearOutput FieldGearRuntime::evaluate(const FieldGearInput&i)const{FieldGearOutput o{};double dt=safe::nonNegative(i.dt),energy=safe::nonNegative(i.availableEnergy),demand=0;if(i.industrialExo)demand+=2.0*dt;if(i.surveyKit)demand+=.5*dt;if(i.siegeKit)demand+=1.5*dt;if(i.explorerKit)demand+=.35*dt;o.energyConsumed=std::min(energy,safe::nonNegative(demand));double powered=demand<=0?1.0:(o.energyConsumed>=demand?1.0:0.0);if(i.industrialExo){o.carryCapacityScale=1.75;o.miningRateScale=powered?1.5:1.0;o.fallDamageScale=powered?.65:1.0;o.toolPowerLost=!powered;}if(i.surveyKit&&powered)o.scanRateScale=1.5;if(i.siegeKit&&powered)o.repairRateScale=1.4;if(i.explorerKit&&powered)o.movementEfficiencyScale=1.25;return o;}
} // namespace elysium::gear
