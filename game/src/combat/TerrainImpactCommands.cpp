#include "combat/TerrainImpactCommands.hpp"
#include <algorithm>
#include <cmath>
namespace elysium::combat {
bool TerrainImpactCommands::setPolicy(TerrainDamageCategory c,TerrainCategoryPolicy p) {
    if(c<TerrainDamageCategory::FragileMicrodetail||c>TerrainDamageCategory::StructuralBreach||
       !p.maxCells||p.maxCells>4096||!std::isfinite(p.maximumDamage)||p.maximumDamage<=0||p.maximumDamage>1e9) return false;
    policies_[c]=p;return true;
}
bool TerrainImpactCommands::beginTick(std::uint64_t tick) {
    if(tick<tick_) return false;if(tick>tick_) {tick_=tick;spent_=0;}return true;
}
TerrainImpactPlan TerrainImpactCommands::submit(const ImpactInput& input) {
    TerrainImpactPlan out;out.event=input.impact.eventId;
    const auto& hit=input.impact;
    if(!hit.eventId||!hit.attackerId||!std::isfinite(hit.landedDamage)||hit.landedDamage<0||
       !std::isfinite(hit.toolBreachScale)||hit.toolBreachScale<0||hit.toolBreachScale>1e6||
       static_cast<unsigned>(hit.category)>4||input.cells.size()>4096) {out.reason=TerrainImpactReason::Invalid;return out;}
    if(accepted_.count(hit.eventId)) {out.reason=TerrainImpactReason::Duplicate;return out;}
    if(hit.category==TerrainDamageCategory::None||hit.landedDamage==0) return out;
    auto cells=input.cells;
    std::sort(cells.begin(),cells.end(),[](auto& a,auto& b){return a.address<b.address;});
    for(std::size_t i=0;i<cells.size();++i) if(!cells[i].address||!cells[i].material||
        !std::isfinite(cells[i].blastResistance)||cells[i].blastResistance<0||
        (i&&cells[i-1].address==cells[i].address)) {out.reason=TerrainImpactReason::Invalid;return out;}
    if(hit.category==TerrainDamageCategory::MarkOnly) {
        for(auto& cell:cells) {if(out.cosmeticMarks.size()==16)break;out.cosmeticMarks.push_back(cell.address);}
        accepted_.insert(hit.eventId);out.reason=TerrainImpactReason::Cosmetic;return out;
    }
    auto policy=policies_.find(hit.category);
    if(policy==policies_.end()) {out.reason=TerrainImpactReason::Invalid;return out;}
    const auto& rules=policy->second;
    auto translated=adapter_.translate(hit);
    const double damage=std::min(rules.maximumDamage,translated.structuralDamage);
    if(!std::isfinite(damage)||damage<=0) return out;
    const unsigned available=std::min(rules.maxCells,budget_-spent_);
    for(auto& cell:cells) {
        if(cell.protectedByPolicy||cell.unbreakable) {out.rejectedCells.push_back(cell.address);out.reason=TerrainImpactReason::Protected;continue;}
        if(input.toolTier<std::max(rules.minimumToolTier,cell.minimumToolTier)||
           damage<=cell.blastResistance||(hit.category==TerrainDamageCategory::FragileMicrodetail&&!cell.fragileDetail)) {
            out.rejectedCells.push_back(cell.address);out.reason=TerrainImpactReason::Material;continue;
        }
        if(out.edits.size()>=available) {out.rejectedCells.push_back(cell.address);out.reason=TerrainImpactReason::Budget;continue;}
        out.edits.push_back({hit.eventId,hit.attackerId,cell.address,cell.revision,cell.material,
            damage-cell.blastResistance,hit.category,true,hit.category==TerrainDamageCategory::FragileMicrodetail});
    }
    // Repeated projectile delivery never produces a second terrain edit. An
    // explicit retry needs a new event and fresh terrain snapshot.
    accepted_.insert(hit.eventId);spent_+=unsigned(out.edits.size());
    if(!out.edits.empty()) out.reason=TerrainImpactReason::Ready;
    return out;
}
TerrainImpactSnapshot TerrainImpactCommands::snapshot() const {
    return {tick_,spent_,std::vector<std::uint64_t>(accepted_.begin(),accepted_.end())};
}
bool TerrainImpactCommands::restore(const TerrainImpactSnapshot& s) {
    if(s.spent>budget_) return false;std::set<std::uint64_t> ids;
    for(auto id:s.accepted) if(!id||!ids.insert(id).second) return false;
    tick_=s.tick;spent_=s.spent;accepted_.swap(ids);return true;
}
}
