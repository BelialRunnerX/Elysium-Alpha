#include "gear/FieldEquipmentQueries.hpp"
#include <algorithm>
#include <cmath>
#include <tuple>
namespace elysium::gear {
namespace { bool valid(double v) { return std::isfinite(v) && v>=0 && v<=1e6; } }
bool FieldEquipmentQueries::publish(FieldEffectDefinition d) {
    if (!d.id || !d.passiveId || d.kind>=FieldEffect::Count ||
        !valid(d.magnitude) || !valid(d.energyPerSecond) || !valid(d.signature)) return false;
    if ((d.kind==FieldEffect::EnergyEfficiency || d.kind==FieldEffect::HazardTraversal) && d.magnitude>1)
        return false;
    return definitions_.emplace(d.id,d).second;
}
bool FieldEquipmentQueries::restore(std::vector<ActiveFieldEffect> items) {
    std::sort(items.begin(),items.end(),[](auto& a,auto& b){return std::tie(a.item,a.effect)<std::tie(b.item,b.effect);});
    for (std::size_t i=0;i<items.size();++i) {
        if (!items[i].item || !definitions_.count(items[i].effect)) return false;
        if (i && items[i].item==items[i-1].item && items[i].effect==items[i-1].effect) return false;
    }
    equipped_.swap(items); return true;
}
FieldQueryOutput FieldEquipmentQueries::evaluate(const FieldQueryInput& input) const {
    FieldQueryOutput out;
    if (!valid(input.seconds) || !valid(input.energy) || !valid(input.baseScanRange) || !valid(input.baseRepairRate)) {
        out.reason=FieldQueryReason::InvalidInput; return out;
    }
    out.scanRange=input.baseScanRange; out.repairRate=input.baseRepairRate;
    double remaining=input.energy;
    for (const auto& active:equipped_) if (active.enabled) {
        const auto& d=definitions_.at(active.effect);
        const double cost=d.energyPerSecond*input.seconds;
        // A zero-length query cannot grant powered effects for free.
        if (d.energyPerSecond>0 && (input.seconds<=0 || remaining<cost || remaining<=0)) {
            out.unpoweredItems.push_back(active.item); out.reason=FieldQueryReason::EnergyUnavailable; continue;
        }
        remaining-=cost; out.energyConsumed+=cost;
        out.signature=std::min(1e6,out.signature+d.signature);
        out.contributingPassives.push_back(d.passiveId);
        switch(d.kind) {
        case FieldEffect::ScanRange: out.scanRange=std::min(1e6,out.scanRange+d.magnitude); break;
        case FieldEffect::AnalysisDetail: out.analysisDetail=std::min(100u,out.analysisDetail+unsigned(std::min(100.0,d.magnitude))); break;
        case FieldEffect::RepairRate: out.repairRate=std::min(1e6,out.repairRate+input.baseRepairRate*d.magnitude); break;
        case FieldEffect::HeavyAmmo: out.canHandleHeavyAmmo=input.authorizedHeavyAmmo; if (!input.authorizedHeavyAmmo) out.reason=FieldQueryReason::PermissionDenied; break;
        case FieldEffect::TurretUse: out.canUseTurret=input.authorizedTurret; if (!input.authorizedTurret) out.reason=FieldQueryReason::PermissionDenied; break;
        case FieldEffect::GroundMobility: out.mobilityScale=std::min(8.0,out.mobilityScale+d.magnitude); break;
        case FieldEffect::EnergyEfficiency: out.movementEnergyScale=std::max(.1,out.movementEnergyScale*(1-d.magnitude)); break;
        case FieldEffect::HazardTraversal: out.traversalBonus=std::min(1.0,out.traversalBonus+d.magnitude); break;
        default: break;
        }
    }
    std::sort(out.contributingPassives.begin(),out.contributingPassives.end());
    out.contributingPassives.erase(std::unique(out.contributingPassives.begin(),out.contributingPassives.end()),out.contributingPassives.end());
    return out;
}
}
