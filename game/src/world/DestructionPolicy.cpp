// Intended function: imported world implementation for DestructionPolicy; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/DestructionPolicy.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <tuple>

namespace elysium {

TerrainDamageDecision DestructionPolicy::decide(BlockType material,
                                                TerrainDamageCategory category,
                                                float energy) const {
    TerrainDamageDecision out{};
    if(material==BlockType::Air || material==BlockType::Magma || energy<=0.0f) return out;
    const auto& block=blockProperties(material);
    const auto& structural=structuralMaterialProfile(material);
    const float hardness=std::max(0.1f,block.hardness);
    const float blast=std::max(hardness,structural.blastResistance);

    switch(category) {
        case TerrainDamageCategory::LightWeapon:
            // Ordinary combat cannot erase macro terrain. It may only chip a
            // refinable surface after sufficient energy.
            if(energy>=hardness*0.65f) out={TerrainDamageAction::MicroChip,1,0,false};
            else out={TerrainDamageAction::Cosmetic,0,0,false};
            break;
        case TerrainDamageCategory::MiningTool:
            if(block.mineable && energy>=hardness) out={TerrainDamageAction::MacroBreak,0,0,true};
            else if(block.mineable && energy>=hardness*0.45f) out={TerrainDamageAction::MicroChip,1,0,false};
            else out={TerrainDamageAction::Cosmetic,0,0,false};
            break;
        case TerrainDamageCategory::Explosive:
            if(energy>=blast) out={TerrainDamageAction::MacroBreak,0,0,true};
            else if(energy>=blast*0.30f) out={TerrainDamageAction::MicroChip,2,0,false};
            else out={TerrainDamageAction::Cosmetic,0,0,false};
            break;
        case TerrainDamageCategory::VehicleImpact:
            if(energy>=blast*1.15f) out={TerrainDamageAction::MacroBreak,0,0,true};
            else if(energy>=blast*0.40f) out={TerrainDamageAction::MicroChip,2,0,false};
            else out={TerrainDamageAction::Cosmetic,0,0,false};
            break;
        case TerrainDamageCategory::SiegeBreach:
            if(energy>=blast*0.80f) out={TerrainDamageAction::Breach,0,1,true};
            else if(energy>=blast*0.25f) out={TerrainDamageAction::MicroChip,2,0,false};
            else out={TerrainDamageAction::Cosmetic,0,0,false};
            break;
        case TerrainDamageCategory::Megathreat:
            if(energy>=blast*0.55f) out={TerrainDamageAction::Breach,0,1,true};
            else if(energy>=blast*0.20f) out={TerrainDamageAction::MicroChip,3,0,false};
            else out={TerrainDamageAction::Cosmetic,0,0,false};
            break;
    }
    return out;
}

StructuralDamageCause DestructionPolicy::structuralCause(TerrainDamageCategory category) {
    switch(category) {
        case TerrainDamageCategory::Explosive: return StructuralDamageCause::Explosion;
        case TerrainDamageCategory::VehicleImpact: return StructuralDamageCause::VehicleImpact;
        case TerrainDamageCategory::SiegeBreach: return StructuralDamageCause::SiegeImpact;
        case TerrainDamageCategory::Megathreat: return StructuralDamageCause::MegathreatImpact;
        case TerrainDamageCategory::LightWeapon:
        case TerrainDamageCategory::MiningTool: return StructuralDamageCause::Removal;
    }
    return StructuralDamageCause::Removal;
}

int DestructionPolicy::chipCell(PlanetSurface& world,
                                SurfaceCellAddress cell,
                                const std::optional<SurfaceMicroAddress>& microImpact,
                                int microRadius) {
    if(microRadius<=0 || !world.radialInBounds(cell.radial)) return 0;
    cell=world.normalize(cell);
    if(!blockProperties(world.get(cell)).solid) return 0;

    int cx=MicroBrick::Resolution/2;
    int cy=MicroBrick::Resolution-1;
    int cz=MicroBrick::Resolution/2;
    if(microImpact && world.normalize(microImpact->cell)==cell) {
        cx=std::clamp(microImpact->u,0,MicroBrick::Resolution-1);
        cy=std::clamp(microImpact->radial,0,MicroBrick::Resolution-1);
        cz=std::clamp(microImpact->v,0,MicroBrick::Resolution-1);
    }

    // microRadius is a policy tier, translated to a bounded physical chip.
    const int radius=std::clamp(microRadius*2,1,6);
    int removed=0;
    for(int y=std::max(0,cy-radius);y<=std::min(MicroBrick::Resolution-1,cy+radius);++y)
        for(int z=std::max(0,cz-radius);z<=std::min(MicroBrick::Resolution-1,cz+radius);++z)
            for(int x=std::max(0,cx-radius);x<=std::min(MicroBrick::Resolution-1,cx+radius);++x) {
                const int dx=x-cx,dy=y-cy,dz=z-cz;
                if(dx*dx+dy*dy+dz*dz>radius*radius) continue;
                if(!blockProperties(world.microGet(cell,x,y,z)).solid) continue;
                world.setMicro(cell,x,y,z,BlockType::Air);
                ++removed;
            }
    return removed;
}

TerrainDamageResult DestructionPolicy::apply(PlanetSurface& world,
                                             SurfaceStructuralIntegritySystem& structural,
                                             const TerrainDamageRequest& request) const {
    TerrainDamageResult out{};
    if(!world.radialInBounds(request.target.radial)) return out;
    const auto target=world.normalize(request.target);
    const auto type=world.get(target);
    out.primary=decide(type,request.category,request.energy);
    if(out.primary.action==TerrainDamageAction::None) return out;
    if(out.primary.action==TerrainDamageAction::Cosmetic) {out.cosmeticMarks=1;return out;}
    if(!request.allowNaturalTerrain && !world.playerPlaced(target)) return out;

    if(out.primary.action==TerrainDamageAction::MicroChip) {
        out.microCellsRemoved=chipCell(world,target,request.microImpact,out.primary.microRadius);
        return out;
    }

    const int radius=out.primary.action==TerrainDamageAction::Breach?
                     std::clamp(request.breachRadius,1,3):0;
    struct Candidate { SurfaceCellAddress address{}; int distance{}; };
    std::vector<Candidate> candidates{{target,0}};
    if(radius>0) {
        // Small bounded Manhattan sphere, normalized by the planet rather than
        // an edge-case seam table.
        for(int dr=-radius;dr<=radius;++dr)
            for(int dv=-radius;dv<=radius;++dv)
                for(int du=-radius;du<=radius;++du) {
                    const int d=std::abs(du)+std::abs(dv)+std::abs(dr);
                    if(d==0 || d>radius) continue;
                    const int rr=target.radial+dr;
                    if(!world.radialInBounds(rr)) continue;
                    candidates.push_back({world.normalize({target.face,target.u+du,target.v+dv,rr}),d});
                }
    }
    std::sort(candidates.begin(),candidates.end(),[](const auto& a,const auto& b){
        const auto ta=std::tuple{a.distance,static_cast<int>(a.address.face),a.address.u,a.address.v,a.address.radial};
        const auto tb=std::tuple{b.distance,static_cast<int>(b.address.face),b.address.u,b.address.v,b.address.radial};
        return ta<tb;
    });
    candidates.erase(std::unique(candidates.begin(),candidates.end(),[](const auto& a,const auto& b){return a.address==b.address;}),candidates.end());

    for(const auto& candidate:candidates) {
        const auto current=world.get(candidate.address);
        if(!blockProperties(current).solid || current==BlockType::Magma) continue;
        if(!request.allowNaturalTerrain && !world.playerPlaced(candidate.address)) continue;
        const float attenuated=request.energy/(1.0f+static_cast<float>(candidate.distance)*0.65f);
        const auto decision=decide(current,request.category,attenuated);
        if(decision.action==TerrainDamageAction::MacroBreak || decision.action==TerrainDamageAction::Breach) {
            world.set(candidate.address,BlockType::Air,false);
            out.changedMacroCells.push_back(candidate.address);
            ++out.macroCellsRemoved;
            structural.notifyEdit(world,candidate.address,structuralCause(request.category),std::max(2,radius+2));
        } else if(decision.action==TerrainDamageAction::MicroChip) {
            out.microCellsRemoved+=chipCell(world,candidate.address,
                                            candidate.address==target?request.microImpact:std::nullopt,
                                            decision.microRadius);
        } else if(decision.action==TerrainDamageAction::Cosmetic) {
            ++out.cosmeticMarks;
        }
    }
    return out;
}

} // namespace elysium
