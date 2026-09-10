#include "travel/ShipModuleBehavior.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
#include <cmath>
#include <set>
namespace elysium::travel {
namespace {
double unit(double v) noexcept { return std::isfinite(v) ? std::clamp(v,0.0,1.0) : 0.0; }
double combineMitigation(double a,double b) noexcept {
 a=unit(a); b=unit(b); return unit(1.0-(1.0-a)*(1.0-b));
}
ModuleBlockReason blockerFor(const ModuleBehaviorDefinition& d,const InstalledModuleBehaviorState& s) noexcept {
 if(d.availability==ModuleAvailability::ProposedUnavailable) return ModuleBlockReason::ProposedUnavailable;
 if(!s.enabled) return ModuleBlockReason::Disabled;
 if(!std::isfinite(s.durability)||s.durability<=0.0) return ModuleBlockReason::Broken;
 if(d.requiresPower&&!s.powered) return ModuleBlockReason::Unpowered;
 if(d.requiresFuel&&!s.fueled) return ModuleBlockReason::Unfueled;
 if(d.requiredClearanceId && s.clearanceId!=d.requiredClearanceId) return ModuleBlockReason::MissingClearance;
 if(d.requiredDependencyId && !s.dependencyAvailable) return ModuleBlockReason::MissingDependency;
 return ModuleBlockReason::None;
}
}

bool ShipModuleBehaviorRuntime::publish(ModuleBehaviorDefinition d){
 if(!d.contentId||!std::isfinite(d.hullBonus)||d.hullBonus<0.0||
    !std::isfinite(d.thermalMitigation)||!std::isfinite(d.radiationMitigation)||
    !std::isfinite(d.customsExposureMultiplier)||d.customsExposureMultiplier<0.0) return false;
 d.hullBonus=safe::nonNegative(d.hullBonus);
 d.thermalMitigation=unit(d.thermalMitigation);
 d.radiationMitigation=unit(d.radiationMitigation);
 d.customsExposureMultiplier=safe::nonNegative(d.customsExposureMultiplier);
 return definitions_.emplace(d.contentId,std::move(d)).second;
}

bool ShipModuleBehaviorRuntime::install(InstalledModuleBehaviorState s){
 if(!s.stableItemId||!s.moduleContentId||!s.revision||!std::isfinite(s.durability)||s.durability<0.0) return false;
 if(!definitions_.contains(s.moduleContentId)) return false;
 s.durability=unit(s.durability);
 return installed_.emplace(s.stableItemId,std::move(s)).second;
}

bool ShipModuleBehaviorRuntime::update(const InstalledModuleBehaviorState& expected, InstalledModuleBehaviorState r){
 auto it=installed_.find(expected.stableItemId);
 if(it==installed_.end()||it->second.revision!=expected.revision||r.stableItemId!=expected.stableItemId||
    !definitions_.contains(r.moduleContentId)||!std::isfinite(r.durability)||r.durability<0.0) return false;
 r.durability=unit(r.durability);
 r.revision=safe::saturatingIncrement(expected.revision);
 it->second=std::move(r); return true;
}

bool ShipModuleBehaviorRuntime::remove(std::uint64_t id,std::uint64_t rev){
 auto it=installed_.find(id); if(it==installed_.end()||it->second.revision!=rev)return false; installed_.erase(it); return true;
}
const InstalledModuleBehaviorState* ShipModuleBehaviorRuntime::find(std::uint64_t id)const{
 auto it=installed_.find(id); return it==installed_.end()?nullptr:&it->second;
}

ShipModuleProjection ShipModuleBehaviorRuntime::project(const std::vector<std::uint64_t>& ids)const{
 ShipModuleProjection out{}; std::set<std::uint64_t> seen; auto ordered=ids; std::sort(ordered.begin(),ordered.end());
 for(auto id:ordered){
  if(!id||!seen.insert(id).second){out.valid=false;out.blockers.push_back({id,ModuleBlockReason::DuplicateStableItem});continue;}
  auto it=installed_.find(id); if(it==installed_.end()){out.valid=false;out.blockers.push_back({id,ModuleBlockReason::MissingDefinition});continue;}
  auto dt=definitions_.find(it->second.moduleContentId); if(dt==definitions_.end()){out.valid=false;out.blockers.push_back({id,ModuleBlockReason::MissingDefinition});continue;}
  const auto& d=dt->second; const auto& s=it->second; const auto why=blockerFor(d,s);
  if(why!=ModuleBlockReason::None){out.blockers.push_back({id,why});continue;}
  const double condition=unit(s.durability);
  out.fuelCapacityBonus=safe::saturatingAdd(out.fuelCapacityBonus,d.fuelCapacityBonus);
  out.cargoCapacityBonus=safe::saturatingAdd(out.cargoCapacityBonus,d.cargoCapacityBonus);
  out.hullBonus=safe::nonNegative(out.hullBonus+d.hullBonus*condition);
  out.thermalMitigation=combineMitigation(out.thermalMitigation,d.thermalMitigation*condition);
  out.radiationMitigation=combineMitigation(out.radiationMitigation,d.radiationMitigation*condition);
  out.scannerGradeBonus=safe::saturatingAdd(out.scannerGradeBonus,d.scannerGradeBonus);
  out.customsExposureMultiplier=safe::nonNegative(out.customsExposureMultiplier*d.customsExposureMultiplier);
  out.activeEffects|=d.effects;
 }
 return out;
}

ShipModuleBehaviorSnapshot ShipModuleBehaviorRuntime::snapshot()const{
 ShipModuleBehaviorSnapshot s; for(const auto&[_,v]:definitions_)s.definitions.push_back(v);for(const auto&[_,v]:installed_)s.installed.push_back(v);return s;
}
bool ShipModuleBehaviorRuntime::restore(const ShipModuleBehaviorSnapshot&s){
 ShipModuleBehaviorRuntime n;for(auto v:s.definitions)if(!n.publish(std::move(v)))return false;for(auto v:s.installed)if(!n.install(std::move(v)))return false;*this=std::move(n);return true;
}
} // namespace elysium::travel
