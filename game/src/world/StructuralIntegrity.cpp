// Intended function: imported world implementation for StructuralIntegrity; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/StructuralIntegrity.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <tuple>

namespace elysium {
namespace {

constexpr StructuralMaterialProfile kAir{false,0,0.0f,false,false};
constexpr StructuralMaterialProfile kWeakFill{true,1,0.8f,false,false};
constexpr StructuralMaterialProfile kWood{true,5,2.5f,false,false};
constexpr StructuralMaterialProfile kStone{true,6,5.0f,false,false};
constexpr StructuralMaterialProfile kBasalt{true,7,7.0f,false,false};
constexpr StructuralMaterialProfile kSteel{true,12,12.0f,true,false};
constexpr StructuralMaterialProfile kBeacon{true,10,16.0f,true,false};
constexpr StructuralMaterialProfile kPortal{false,0,5.0f,false,false};
constexpr StructuralMaterialProfile kRubble{false,0,1.0f,false,true};

std::tuple<int,int,int,int> addressTuple(const SurfaceCellAddress& a) {
    return {static_cast<int>(a.face),a.u,a.v,a.radial};
}

} // namespace

const StructuralMaterialProfile& structuralMaterialProfile(BlockType type) {
    switch(type) {
        case BlockType::Air: return kAir;
        case BlockType::Grass:
        case BlockType::Dirt:
        case BlockType::Regolith: return kWeakFill;
        case BlockType::Planks: return kWood;
        case BlockType::Basalt: return kBasalt;
        case BlockType::Stone:
        case BlockType::CoalOre:
        case BlockType::CopperOre:
        case BlockType::TinOre:
        case BlockType::IronOre: return kStone;
        case BlockType::SteelPlate: return kSteel;
        case BlockType::RegistryBeacon: return kBeacon;
        case BlockType::DoorPanel:
        case BlockType::AirlockPanel: return kPortal;
        case BlockType::Rubble: return kRubble;
        case BlockType::Magma: return kAir;
        case BlockType::Count: break;
    }
    return kAir;
}

std::size_t SurfaceCellAddressHash::operator()(const SurfaceCellAddress& a) const noexcept {
    std::uint64_t h=1469598103934665603ULL;
    auto add=[&](std::uint64_t v){h^=v;h*=1099511628211ULL;};
    add(static_cast<std::uint64_t>(static_cast<std::uint32_t>(static_cast<int>(a.face))));
    add(static_cast<std::uint64_t>(static_cast<std::uint32_t>(a.u)));
    add(static_cast<std::uint64_t>(static_cast<std::uint32_t>(a.v)));
    add(static_cast<std::uint64_t>(static_cast<std::uint32_t>(a.radial)));
    return static_cast<std::size_t>(h);
}

bool SurfaceCellAddressLess::operator()(const SurfaceCellAddress& a,const SurfaceCellAddress& b) const noexcept {
    return addressTuple(a)<addressTuple(b);
}

SurfaceStructuralIntegritySystem::SurfaceStructuralIntegritySystem()
    : SurfaceStructuralIntegritySystem(Tuning{}) {}

SurfaceStructuralIntegritySystem::SurfaceStructuralIntegritySystem(Tuning tuning):tuning_(tuning) {
    tuning_.dirtyRadius=std::clamp(tuning_.dirtyRadius,1,64);
    tuning_.maxIslandCells=std::max(16,tuning_.maxIslandCells);
    tuning_.stepsPerTick=std::max(1,tuning_.stepsPerTick);
    tuning_.maxPresentationDebris=std::max(0,tuning_.maxPresentationDebris);
    tuning_.rubbleSearchDepth=std::max(1,tuning_.rubbleSearchDepth);
}

std::vector<SurfaceCellAddress> SurfaceStructuralIntegritySystem::neighbors(const PlanetSurface& world,SurfaceCellAddress a) {
    std::vector<SurfaceCellAddress> out;
    out.reserve(6);
    const std::array<SurfaceCellAddress,6> raw{{
        {a.face,a.u+1,a.v,a.radial},{a.face,a.u-1,a.v,a.radial},
        {a.face,a.u,a.v+1,a.radial},{a.face,a.u,a.v-1,a.radial},
        {a.face,a.u,a.v,a.radial+1},{a.face,a.u,a.v,a.radial-1}
    }};
    for(auto n:raw) {
        if(!world.radialInBounds(n.radial)) continue;
        out.push_back(world.normalize(n));
    }
    return out;
}

bool SurfaceStructuralIntegritySystem::structuralCell(const PlanetSurface& world,SurfaceCellAddress address) {
    if(!world.radialInBounds(address.radial)) return false;
    address=world.normalize(address);
    const BlockType type=world.get(address);
    return world.playerPlaced(address) && blockProperties(type).solid && structuralMaterialProfile(type).participates;
}

bool SurfaceStructuralIntegritySystem::naturalRoot(const PlanetSurface& world,SurfaceCellAddress address) const {
    // Natural terrain is self-supporting and transfers support into adjacent
    // player construction. Explicit roots cover foundations/machine anchors.
    const std::array<SurfaceCellAddress,6> raw{{
        {address.face,address.u+1,address.v,address.radial},{address.face,address.u-1,address.v,address.radial},
        {address.face,address.u,address.v+1,address.radial},{address.face,address.u,address.v-1,address.radial},
        {address.face,address.u,address.v,address.radial+1},{address.face,address.u,address.v,address.radial-1}
    }};
    for(auto n:raw) {
        if(n.radial<0) return true; // mantle-side continuation is a root.
        if(!world.radialInBounds(n.radial)) continue;
        n=world.normalize(n);
        const auto type=world.get(n);
        if(blockProperties(type).solid && !world.playerPlaced(n) && !structuralMaterialProfile(type).rubble)
            return true;
    }
    return false;
}

void SurfaceStructuralIntegritySystem::addExplicitRoot(SurfaceCellAddress address) {
    explicitRoots_.insert(address);
}

void SurfaceStructuralIntegritySystem::removeExplicitRoot(SurfaceCellAddress address) {
    explicitRoots_.erase(address);
}

bool SurfaceStructuralIntegritySystem::isExplicitRoot(SurfaceCellAddress address) const {
    return explicitRoots_.contains(address);
}

void SurfaceStructuralIntegritySystem::seedWork(const PlanetSurface& world,DirtyWork& work) {
    auto enqueue=[&](SurfaceCellAddress a,int distance) {
        if(!world.radialInBounds(a.radial)) return;
        a=world.normalize(a);
        if(!structuralCell(world,a)) return;
        if(work.queued.insert(a).second) work.frontier.push_back({a,distance});
    };
    enqueue(work.origin,0);
    for(const auto& n:neighbors(world,work.origin)) enqueue(n,1);
}

void SurfaceStructuralIntegritySystem::notifyEdit(const PlanetSurface& world,
                                                  SurfaceCellAddress origin,
                                                  StructuralDamageCause cause,
                                                  int radius) {
    if(!world.radialInBounds(origin.radial)) return;
    DirtyWork work{};
    work.origin=world.normalize(origin);
    work.radius=std::clamp(radius<0?tuning_.dirtyRadius:radius,1,64);
    work.cause=cause;
    work.serial=nextSerial_++;
    work.worldRevision=world.revision();
    seedWork(world,work);
    if(work.frontier.empty()) return;
    pending_.push_back(std::move(work));
    ++telemetry_.dirtyEvents;
}

bool SurfaceStructuralIntegritySystem::stepCollect(const PlanetSurface& world,DirtyWork& work) {
    if(work.frontier.empty()) {
        work.phase=WorkPhase::SeedSupport;
        work.seedCursor=0;
        return true;
    }

    const FrontierNode node=work.frontier.front();
    work.frontier.pop_front();
    if(work.cellIndex.contains(node.address)) return true;

    CellState state{};
    state.address=node.address;
    state.distance=node.distance;
    state.naturalRoot=naturalRoot(world,node.address);
    state.explicitRoot=explicitRoots_.contains(node.address);
    const std::size_t index=work.cells.size();
    work.cellIndex.emplace(node.address,index);
    work.cells.push_back(state);

    for(const auto& n:neighbors(world,node.address)) {
        if(!structuralCell(world,n)) continue;
        if(work.cellIndex.contains(n) || work.queued.contains(n)) continue;
        if(node.distance>=work.radius) {
            work.cells[index].boundaryRoot=true;
            work.radiusClipped=true;
            continue;
        }
        if(static_cast<int>(work.cells.size()+work.frontier.size())>=tuning_.maxIslandCells) {
            work.cells[index].boundaryRoot=true;
            work.cellCapClipped=true;
            continue;
        }
        work.queued.insert(n);
        work.frontier.push_back({n,node.distance+1});
    }
    return true;
}

bool SurfaceStructuralIntegritySystem::stepSeedSupport(const PlanetSurface& world,DirtyWork& work) {
    (void)world;
    if(work.seedCursor>=work.cells.size()) {
        work.phase=WorkPhase::PropagateSupport;
        return true;
    }
    auto& cell=work.cells[work.seedCursor];
    if(cell.naturalRoot || cell.boundaryRoot || cell.explicitRoot) {
        const int span=structuralMaterialProfile(world.get(cell.address)).maxSupportSpan;
        cell.support=std::max(cell.support,span);
        work.supportFrontier.push({cell.support,work.seedCursor});
    }
    ++work.seedCursor;
    return true;
}

int SurfaceStructuralIntegritySystem::transferSupport(const CellState& from,const CellState& to) {
    (void)to;
    return from.support-1;
}

bool SurfaceStructuralIntegritySystem::stepPropagate(const PlanetSurface& world,DirtyWork& work) {
    if(work.supportFrontier.empty()) {
        work.collapseIndices.clear();
        for(std::size_t i=0;i<work.cells.size();++i)
            if(work.cells[i].support<0) work.collapseIndices.push_back(i);
        std::sort(work.collapseIndices.begin(),work.collapseIndices.end(),[&](std::size_t a,std::size_t b){
            return SurfaceCellAddressLess{}(work.cells[a].address,work.cells[b].address);
        });
        work.emitCursor=0;
        work.phase=WorkPhase::EmitCollapse;
        return true;
    }

    const auto current=work.supportFrontier.top();
    work.supportFrontier.pop();
    if(current.cellIndex>=work.cells.size()) return true;
    const auto& from=work.cells[current.cellIndex];
    if(current.remaining!=from.support || from.support<=0) return true;

    for(const auto& n:neighbors(world,from.address)) {
        const auto it=work.cellIndex.find(n);
        if(it==work.cellIndex.end()) continue;
        auto& to=work.cells[it->second];
        const int transferred=transferSupport(from,to);
        if(transferred<0) continue;
        const int capped=std::min(transferred,structuralMaterialProfile(world.get(to.address)).maxSupportSpan);
        if(capped<=to.support) continue;
        to.support=capped;
        work.supportFrontier.push({capped,it->second});
    }
    return true;
}

bool SurfaceStructuralIntegritySystem::stepEmit(DirtyWork& work) {
    if(work.emitCursor>=work.collapseIndices.size()) return false;
    ++work.emitCursor;
    return true;
}

std::optional<StructuralEvaluationBatch> SurfaceStructuralIntegritySystem::update(const PlanetSurface& world,int stepBudget) {
    if(pending_.empty()) return std::nullopt;
    stepBudget=stepBudget<0?tuning_.stepsPerTick:std::max(1,stepBudget);

    auto& work=pending_.front();
    // A concurrent/owner-thread edit while an evaluation is staged invalidates
    // its snapshot assumptions. Restart from the same bounded event instead of
    // publishing stale structural commands.
    if(work.worldRevision!=world.revision()) {
        DirtyWork restarted{};
        restarted.origin=work.origin;
        restarted.radius=work.radius;
        restarted.cause=work.cause;
        restarted.serial=work.serial;
        restarted.worldRevision=world.revision();
        seedWork(world,restarted);
        work=std::move(restarted);
        if(work.frontier.empty()) {pending_.pop_front();return std::nullopt;}
    }

    int steps=0;
    while(steps<stepBudget) {
        bool consumed=true;
        switch(work.phase) {
            case WorkPhase::Collect: consumed=stepCollect(world,work); break;
            case WorkPhase::SeedSupport: consumed=stepSeedSupport(world,work); break;
            case WorkPhase::PropagateSupport: consumed=stepPropagate(world,work); break;
            case WorkPhase::EmitCollapse:
                if(work.emitCursor>=work.collapseIndices.size()) {
                    StructuralEvaluationBatch out{};
                    out.origin=work.origin;
                    out.dirtyRadius=work.radius;
                    out.cause=work.cause;
                    out.eventSerial=work.serial;
                    out.sourceWorldRevision=work.worldRevision;
                    out.boundedByRadius=work.radiusClipped;
                    out.boundedByCellCap=work.cellCapClipped;
                    out.evaluatedSteps=work.evaluatedSteps;
                    const int debrisLimit=tuning_.maxPresentationDebris;
                    out.collapse.reserve(work.collapseIndices.size());
                    for(std::size_t i=0;i<work.collapseIndices.size();++i) {
                        const auto& cell=work.cells[work.collapseIndices[i]];
                        out.collapse.push_back({cell.address,world.get(cell.address),work.cause,work.serial,
                                                static_cast<int>(i)<debrisLimit});
                    }
                    telemetry_.stableCells+=work.cells.size()-work.collapseIndices.size();
                    telemetry_.collapsedCells+=work.collapseIndices.size();
                    telemetry_.radiusClips+=work.radiusClipped?1:0;
                    telemetry_.cellCapClips+=work.cellCapClipped?1:0;
                    pending_.pop_front();
                    return out;
                }
                consumed=stepEmit(work);
                break;
        }
        if(consumed) {
            ++steps;
            ++work.evaluatedSteps;
            ++telemetry_.evaluatedSteps;
        }
    }
    ++telemetry_.stagedTicks;
    return std::nullopt;
}

std::optional<SurfaceCellAddress> SurfaceStructuralIntegritySystem::rubbleLanding(const PlanetSurface& world,SurfaceCellAddress source) const {
    source=world.normalize(source);
    const int minRadial=std::max(0,source.radial-tuning_.rubbleSearchDepth);
    for(int r=source.radial-1;r>=minRadial;--r) {
        SurfaceCellAddress below{source.face,source.u,source.v,r};
        below=world.normalize(below);
        if(!blockProperties(world.get(below)).solid) continue;
        SurfaceCellAddress landing{below.face,below.u,below.v,r+1};
        if(world.radialInBounds(landing.radial) && !blockProperties(world.get(landing)).solid)
            return world.normalize(landing);
        return std::nullopt;
    }
    return std::nullopt;
}

StructuralCommitResult SurfaceStructuralIntegritySystem::commit(PlanetSurface& world,
                                                                 const StructuralEvaluationBatch& batch) const {
    StructuralCommitResult out{};
    if(batch.sourceWorldRevision!=world.revision()) return out;
    if(batch.collapse.empty()) return out;

    std::vector<StructuralCollapseCommand> commands=batch.collapse;
    std::sort(commands.begin(),commands.end(),[](const auto& a,const auto& b){
        return SurfaceCellAddressLess{}(a.source,b.source);
    });
    const std::uint64_t before=world.revision();

    // Remove all unsupported structure first. This prevents evaluation order
    // from letting one falling cell become a temporary support for another.
    std::vector<StructuralCollapseCommand> applied;
    applied.reserve(commands.size());
    for(const auto& command:commands) {
        const auto source=world.normalize(command.source);
        if(!structuralCell(world,source)) continue;
        applied.push_back(command);
        world.set(source,BlockType::Air,false);
        ++out.removedCells;
    }

    // Deterministic radial settling is the persistence truth. Presentation may
    // animate only the bounded debris subset, but reload never replays physics.
    for(const auto& command:applied) {
        StructuralDebrisEvent debris{};
        debris.source=world.normalize(command.source);
        debris.sourceMaterial=command.material;
        debris.eventSerial=command.eventSerial;
        if(const auto landing=rubbleLanding(world,debris.source)) {
            world.set(*landing,BlockType::Rubble,false);
            debris.settledAt=*landing;
            ++out.rubbleCells;
        }
        if(command.presentAsDebris) out.debris.push_back(debris);
    }

    if(out.removedCells>0) {
        out.invalidations.push_back({batch.origin,batch.dirtyRadius,allStructuralInvalidations(),
                                     before,world.revision(),batch.eventSerial});
    }
    return out;
}

bool SurfaceStructuralIntegritySystem::hasPendingWork() const { return !pending_.empty(); }
std::size_t SurfaceStructuralIntegritySystem::pendingEventCount() const { return pending_.size(); }

void SurfaceStructuralIntegritySystem::clear() {
    pending_.clear();
    explicitRoots_.clear();
}

} // namespace elysium
