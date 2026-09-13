#include "orbital/CustomsInspectionPolicy.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
namespace elysium::orbital {namespace{constexpr std::uint64_t Provider=0x435553544f4dULL;}
CustomsDecision CustomsInspectionPolicy::inspect(const CustomsContext&c,std::span<const ManifestLine>m)const{
 CustomsDecision o{};o.inspectionId=c.inspectionId;o.warrantId=c.warrantId;o.clearanceId=c.clearanceId;
 if(!c.inspectionId||!c.jurisdictionId||!c.shipId||!c.inspectionSourceId||!std::isfinite(c.suspicion)||!std::isfinite(c.tariffScale)||!std::isfinite(c.exposureModifier)){o.reasons.add({reason::ReasonCode::InvalidRequest,Provider,c.shipId,0});return o;}
 std::uint64_t taxable=0;const double globalExposure=safe::nonNegative(c.exposureModifier);
 for(const auto&x:m){if(!x.contentId||!x.quantity||!std::isfinite(x.exposureModifier))continue;CargoClassificationLine cl{};cl.contentId=x.contentId;cl.quantity=x.quantity;cl.exposedQuantity=safe::nonNegative(static_cast<double>(x.quantity)*globalExposure*safe::nonNegative(x.exposureModifier));
  if(x.contraband)cl.classification=CargoClassification::Contraband;else if(c.quarantine&&x.quarantineRestricted)cl.classification=CargoClassification::QuarantineRestricted;else if(x.registered)cl.classification=CargoClassification::Registered;else cl.classification=CargoClassification::Unregistered;
  o.classified.push_back(cl);
  switch(cl.classification){
   case CargoClassification::Contraband:o.detectedContrabandExposure=safe::nonNegative(o.detectedContrabandExposure+cl.exposedQuantity);if(!c.clearanceId){o.hold=true;o.requestInterdiction=true;o.reasons.add({reason::ReasonCode::ProvenanceForbidden,Provider,x.contentId,10});}break;
   case CargoClassification::QuarantineRestricted:if(!c.clearanceId){o.hold=true;o.reasons.add({reason::ReasonCode::MissingCapability,Provider,x.contentId,20});}break;
   case CargoClassification::Unregistered:if(c.warrantId){o.hold=true;o.requestInterdiction=true;}break;
   case CargoClassification::Registered:taxable=safe::saturatingAdd(taxable,x.quantity);break;
  }
 }
 o.tariffUnits=safe::nonNegative(static_cast<double>(taxable)*safe::nonNegative(c.tariffScale));o.cleared=!o.hold;return o;
}

bool InterdictionRegistry::create(InterdictionField f){if(!f.fieldId||!f.systemId||!f.jurisdictionId||!f.sourceInspectionId||!f.revision||f.expiresTick<=f.startTick||!std::isfinite(f.routeCostMultiplier)||!std::isfinite(f.routeRiskAdd)||!std::isfinite(f.accessFriction))return false;f.routeCostMultiplier=std::max(1.0,safe::nonNegative(f.routeCostMultiplier));f.routeRiskAdd=safe::nonNegative(f.routeRiskAdd);f.accessFriction=safe::nonNegative(f.accessFriction);f.state=InterdictionState::Active;return fields_.emplace(f.fieldId,f).second;}
bool InterdictionRegistry::cancel(std::uint64_t id,std::uint64_t rev){auto it=fields_.find(id);if(it==fields_.end()||it->second.revision!=rev||it->second.state!=InterdictionState::Active)return false;it->second.state=InterdictionState::Cancelled;it->second.revision=safe::saturatingIncrement(it->second.revision);return true;}
void InterdictionRegistry::expire(std::uint64_t tick){for(auto&[_,f]:fields_)if(f.state==InterdictionState::Active&&tick>=f.expiresTick){f.state=InterdictionState::Expired;f.revision=safe::saturatingIncrement(f.revision);}}
const InterdictionField*InterdictionRegistry::find(std::uint64_t id)const{auto it=fields_.find(id);return it==fields_.end()?nullptr:&it->second;}
InterdictionEstimate InterdictionRegistry::estimate(std::uint64_t system,std::uint64_t tick)const{InterdictionEstimate e{};for(const auto&[_,f]:fields_)if(f.systemId==system&&f.state==InterdictionState::Active&&tick>=f.startTick&&tick<f.expiresTick){e.routeCostMultiplier=safe::nonNegative(e.routeCostMultiplier*f.routeCostMultiplier);if(e.routeCostMultiplier<1.0)e.routeCostMultiplier=1.0;e.routeRiskAdd=safe::nonNegative(e.routeRiskAdd+f.routeRiskAdd);e.accessFriction=safe::nonNegative(e.accessFriction+f.accessFriction);e.activeFields=safe::saturatingAdd(e.activeFields,1u);}e.alternateRoutePreserved=true;return e;}
InterdictionSnapshot InterdictionRegistry::snapshot()const{InterdictionSnapshot s;for(auto&[_,f]:fields_)s.fields.push_back(f);return s;}
bool InterdictionRegistry::restore(const InterdictionSnapshot&s){InterdictionRegistry n;for(auto f:s.fields){if(!f.fieldId||!f.systemId||!f.jurisdictionId||!f.sourceInspectionId||!f.revision||f.expiresTick<=f.startTick||!std::isfinite(f.routeCostMultiplier)||f.routeCostMultiplier<1.0||!n.fields_.emplace(f.fieldId,f).second)return false;}*this=std::move(n);return true;}
} // namespace elysium::orbital
