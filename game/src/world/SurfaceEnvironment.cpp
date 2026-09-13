// Intended function: imported world implementation for SurfaceEnvironment; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/SurfaceEnvironment.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <sstream>
#include <tuple>

namespace elysium {
namespace {

constexpr float kEpsilon = 1e-5f;
constexpr float kOpenVentPressureRate = 0.80f;
constexpr float kOpenVentGasRate = 1.05f;
constexpr float kThermalDiffusion = 0.12f;
constexpr float kThermalAmbientRelax = 0.04f;
constexpr float kFireHeatPerSecond = 140.0f;
constexpr float kFireSmokePerSecond = 0.12f;
constexpr float kFireSpreadTemperature = 120.0f;
constexpr float kFireFuelBurnPerSecond = 1.0f;

std::tuple<int,int,int,int> addressTuple(const SurfaceCellAddress& a) {
    return {static_cast<int>(a.face),a.radial,a.v,a.u};
}

std::uint64_t stableAddressKey(const SurfaceCellAddress& a) {
    std::uint64_t h=mix64(0x454E564144445231ULL ^ static_cast<std::uint64_t>(a.face));
    h=mix64(h ^ static_cast<std::uint64_t>(static_cast<std::uint32_t>(a.u)));
    h=mix64(h ^ (static_cast<std::uint64_t>(static_cast<std::uint32_t>(a.v))<<1U));
    h=mix64(h ^ (static_cast<std::uint64_t>(static_cast<std::uint32_t>(a.radial))<<2U));
    return h;
}

std::array<SurfaceCellAddress,6> gasNeighbors(const PlanetSurface& planet, SurfaceCellAddress c) {
    std::array<SurfaceCellAddress,6> n{{
        {c.face,c.u+1,c.v,c.radial},
        {c.face,c.u-1,c.v,c.radial},
        {c.face,c.u,c.v+1,c.radial},
        {c.face,c.u,c.v-1,c.radial},
        {c.face,c.u,c.v,c.radial+1},
        {c.face,c.u,c.v,c.radial-1}
    }};
    for(std::size_t i=0;i<4;++i) n[i]=planet.normalize(n[i]);
    return n;
}

std::array<SurfaceCellAddress,4> lateralNeighbors(const PlanetSurface& planet, SurfaceCellAddress c) {
    return {{
        planet.normalize({c.face,c.u+1,c.v,c.radial}),
        planet.normalize({c.face,c.u-1,c.v,c.radial}),
        planet.normalize({c.face,c.u,c.v+1,c.radial}),
        planet.normalize({c.face,c.u,c.v-1,c.radial})
    }};
}

bool sameKindOrEmpty(const std::optional<SurfaceFluidCell>& target, SurfaceFluidKind kind) {
    return !target || target->amount<=kEpsilon || target->kind==kind;
}

float clamp01(float v) { return std::clamp(v,0.0f,1.0f); }

} // namespace

bool SurfaceCellAddressLess::operator()(const SurfaceCellAddress& a, const SurfaceCellAddress& b) const noexcept {
    return addressTuple(a)<addressTuple(b);
}

void SurfaceRoomGraph::refreshRevision(const PlanetSurface& planet) {
    if(cachedRevision_==planet.revision()) return;
    rooms_.clear();
    membership_.clear();
    cachedRevision_=planet.revision();
    telemetry_.geometryRevision=cachedRevision_;
    ++telemetry_.invalidations;
    telemetry_.cachedRooms=0;
    telemetry_.sealedRooms=0;
    telemetry_.unboundedRooms=0;
}

void SurfaceRoomGraph::invalidate() {
    rooms_.clear();
    membership_.clear();
    cachedRevision_=0;
    ++telemetry_.invalidations;
    telemetry_.cachedRooms=0;
    telemetry_.sealedRooms=0;
    telemetry_.unboundedRooms=0;
}

std::uint64_t SurfaceRoomGraph::makeRuntimeRoomId(const SurfaceRoomDescriptor& room) const {
    if(room.cells.empty()) return 0;
    const auto* canonical=&room.cells.front();
    for(const auto& cell:room.cells) if(SurfaceCellAddressLess{}(cell,*canonical)) canonical=&cell;
    std::uint64_t h=mix64(worldSeed_^0x524F4F4D47524150ULL^stableAddressKey(*canonical));
    h=mix64(h^static_cast<std::uint64_t>(room.cells.size()));
    for(const auto& cell:room.cells) h=mix64(h^stableAddressKey(cell));
    h=mix64(h^(room.sealed?0x5345414C4544ULL:0x4F50454EULL));
    return h==0?1:h;
}

SurfaceRoomDescriptor SurfaceRoomGraph::query(const PlanetSurface& planet, SurfaceCellAddress start, int maxCells) {
    refreshRevision(planet);
    maxCells=std::max(1,maxCells);
    telemetry_.visitedCellsLastQuery=0;
    if(!planet.radialInBounds(start.radial)) return {};
    start=planet.normalize(start);
    if(!planet.gasPassableAt(start)) return {};

    if(const auto hit=membership_.find(start); hit!=membership_.end() && hit->second<rooms_.size()) {
        const auto& cached=rooms_[hit->second];
        if(!cached.budgetExhausted) {
            ++telemetry_.cacheHits;
            if(cached.sealed && static_cast<int>(cached.cells.size())>=maxCells) {
                SurfaceRoomDescriptor limited=cached;
                limited.cells.resize(static_cast<std::size_t>(maxCells));
                limited.sealed=false;
                limited.unbounded=true;
                limited.budgetExhausted=true;
                limited.runtimeId=makeRuntimeRoomId(limited);
                telemetry_.visitedCellsLastQuery=maxCells;
                return limited;
            }
            telemetry_.visitedCellsLastQuery=static_cast<int>(cached.cells.size());
            return cached;
        }
    }
    ++telemetry_.cacheMisses;

    SurfaceRoomDescriptor room{};
    room.anchor=start;
    room.geometryRevision=planet.revision();
    std::vector<SurfaceCellAddress> queue;
    queue.reserve(static_cast<std::size_t>(std::min(maxCells,8192)));
    std::set<SurfaceCellAddress,SurfaceCellAddressLess> visited;
    queue.push_back(start);
    visited.insert(start);
    std::size_t head=0;

    while(head<queue.size()) {
        const auto c=queue[head++];
        room.cells.push_back(c);
        ++telemetry_.visitedCellsLastQuery;
        if(static_cast<int>(room.cells.size())>=maxCells) {
            room.budgetExhausted=true;
            room.unbounded=true;
            room.sealed=false;
            break;
        }

        for(auto n:gasNeighbors(planet,c)) {
            if(n.radial<0) continue;
            if(n.radial>=PlanetSurface::RadialLayers) {
                // Direction-space sky is conceptually unbounded. We do not
                // materialize exterior cells; reaching it proves this fill would
                // fail to terminate inside the intentional room budget.
                room.unbounded=true;
                room.sealed=false;
                head=queue.size();
                break;
            }
            if(!planet.gasPassableAt(n)) continue;
            if(visited.insert(n).second) queue.push_back(n);
        }
        if(room.unbounded) break;
    }

    if(!room.unbounded && !room.budgetExhausted) room.sealed=true;
    std::sort(room.cells.begin(),room.cells.end(),SurfaceCellAddressLess{});
    room.runtimeId=makeRuntimeRoomId(room);
    if(!room.budgetExhausted) {
        const std::size_t index=rooms_.size();
        rooms_.push_back(room);
        for(const auto& cell:room.cells) membership_[cell]=index;
        telemetry_.cachedRooms=static_cast<int>(rooms_.size());
        if(room.sealed) ++telemetry_.sealedRooms;
        else ++telemetry_.unboundedRooms;
    }
    return room;
}

SurfaceEnvironmentSystem::SurfaceEnvironmentSystem(std::uint64_t worldSeed,
                                                   int roomBudget,
                                                   int maxFluidTransfersPerTick,
                                                   int maxThermalCellsPerTick)
    : worldSeed_(worldSeed),
      roomBudget_(std::max(1,roomBudget)),
      maxFluidTransfersPerTick_(std::max(1,maxFluidTransfersPerTick)),
      maxThermalCellsPerTick_(std::max(1,maxThermalCellsPerTick)),
      roomGraph_(worldSeed) {}

std::uint64_t SurfaceEnvironmentSystem::allocateStableId() {
    for(;;) {
        const std::uint64_t id=mix64(worldSeed_^0x454E564F424AULL^nextSerial_++);
        if(id==0) continue;
        const bool gasCollision=std::any_of(gasLinks_.begin(),gasLinks_.end(),[&](const auto& v){return v.stableId==id;});
        const bool pumpCollision=std::any_of(fluidPumps_.begin(),fluidPumps_.end(),[&](const auto& v){return v.stableId==id;});
        if(!gasCollision && !pumpCollision) return id;
    }
}

SurfaceGasState SurfaceEnvironmentSystem::sanitizeGas(SurfaceGasState gas) {
    gas.pressure=std::clamp(gas.pressure,0.0f,1.5f);
    gas.oxygen=std::clamp(gas.oxygen,0.0f,gas.pressure);
    gas.smoke=std::clamp(gas.smoke,0.0f,gas.pressure);
    gas.toxin=std::clamp(gas.toxin,0.0f,gas.pressure);
    return gas;
}

bool SurfaceEnvironmentSystem::gasNonZero(const SurfaceGasState& gas) {
    return gas.pressure>kEpsilon || gas.oxygen>kEpsilon || gas.smoke>kEpsilon || gas.toxin>kEpsilon;
}

bool SurfaceEnvironmentSystem::flammable(BlockType block) {
    return block==BlockType::Planks || block==BlockType::Grass;
}

float SurfaceEnvironmentSystem::ambientTemperature(const PlanetSurface& planet) {
    switch(planet.planetClass()) {
        case PlanetClass::Scorched: return 65.0f;
        case PlanetClass::Barren: return -35.0f;
        case PlanetClass::Temperate: return 20.0f;
        case PlanetClass::Frozen: return -60.0f;
        case PlanetClass::Toxic: return 34.0f;
        case PlanetClass::Irradiated: return 28.0f;
        case PlanetClass::Oceanic: return 12.0f;
        case PlanetClass::Anomalous: return 5.0f;
    }
    return 20.0f;
}

std::optional<std::size_t> SurfaceEnvironmentSystem::gasRegionContaining(SurfaceCellAddress cell) const {
    for(std::size_t i=0;i<gasRegions_.size();++i) {
        const auto& cells=gasRegions_[i].room.cells;
        if(std::binary_search(cells.begin(),cells.end(),cell,SurfaceCellAddressLess{})) return i;
    }
    return std::nullopt;
}

void SurfaceEnvironmentSystem::reconcileGasTopology(const PlanetSurface& planet) {
    if(gasGeometryRevision_==planet.revision()) return;

    const auto old=gasRegions_;
    std::set<SurfaceCellAddress,SurfaceCellAddressLess> candidateCells;
    for(const auto& region:old) for(const auto& cell:region.room.cells) candidateCells.insert(cell);
    for(const auto& link:gasLinks_) { candidateCells.insert(link.a); candidateCells.insert(link.b); }

    std::vector<GasRegion> rebuilt;
    std::set<SurfaceCellAddress,SurfaceCellAddressLess> assigned;
    for(const auto& cell:candidateCells) {
        if(assigned.contains(cell) || !planet.radialInBounds(cell.radial) || !planet.gasPassableAt(cell)) continue;
        auto room=roomGraph_.query(planet,cell,roomBudget_);
        if(room.cells.empty()) continue;
        GasRegion region{};
        region.room=room;

        double weight=0.0,pressure=0.0,oxygen=0.0,smoke=0.0,toxin=0.0;
        for(const auto& previous:old) {
            int overlap=0;
            for(const auto& c:room.cells)
                if(std::binary_search(previous.room.cells.begin(),previous.room.cells.end(),c,SurfaceCellAddressLess{})) ++overlap;
            if(overlap<=0) continue;
            weight+=overlap;
            pressure+=static_cast<double>(previous.gas.pressure)*overlap;
            oxygen+=static_cast<double>(previous.gas.oxygen)*overlap;
            smoke+=static_cast<double>(previous.gas.smoke)*overlap;
            toxin+=static_cast<double>(previous.gas.toxin)*overlap;
        }
        if(weight>0.0) {
            region.gas=sanitizeGas({static_cast<float>(pressure/weight),
                                    static_cast<float>(oxygen/weight),
                                    static_cast<float>(smoke/weight),
                                    static_cast<float>(toxin/weight)});
        }
        for(const auto& c:room.cells) assigned.insert(c);
        rebuilt.push_back(std::move(region));
    }

    // Preserve explicit gas seeds whose anchors were not part of the previous
    // room cell set (for example state restored before geometry promotion).
    for(const auto& previous:old) {
        if(assigned.contains(previous.room.anchor) || !planet.gasPassableAt(previous.room.anchor)) continue;
        auto room=roomGraph_.query(planet,previous.room.anchor,roomBudget_);
        if(room.cells.empty()) continue;
        rebuilt.push_back({room,previous.gas});
        for(const auto& c:room.cells) assigned.insert(c);
    }

    for(const auto& previous:old) {
        if(previous.room.sealed && previous.gas.pressure>=0.25f) {
            bool survivedSealed=false;
            for(const auto& now:rebuilt) {
                if(!now.room.sealed) continue;
                if(std::binary_search(now.room.cells.begin(),now.room.cells.end(),previous.room.anchor,SurfaceCellAddressLess{})) {
                    survivedSealed=true;
                    break;
                }
            }
            if(!survivedSealed) emitEvent(SurfaceEnvironmentEventKind::PressureLoss,previous.room.anchor,previous.gas.pressure);
        }
    }

    gasRegions_=std::move(rebuilt);
    gasGeometryRevision_=planet.revision();
}

std::optional<std::size_t> SurfaceEnvironmentSystem::gasRegionForAnchor(const PlanetSurface& planet, SurfaceCellAddress anchor) {
    reconcileGasTopology(planet);
    if(auto found=gasRegionContaining(planet.normalize(anchor))) return found;
    auto room=roomGraph_.query(planet,anchor,roomBudget_);
    if(room.cells.empty()) return std::nullopt;
    gasRegions_.push_back({room,{}});
    return gasRegions_.size()-1;
}

bool SurfaceEnvironmentSystem::setRoomGas(const PlanetSurface& planet, SurfaceCellAddress anchor, SurfaceGasState gas) {
    auto index=gasRegionForAnchor(planet,anchor);
    if(!index) return false;
    gasRegions_[*index].gas=sanitizeGas(gas);
    return true;
}

std::optional<SurfaceGasRegionState> SurfaceEnvironmentSystem::gasAt(const PlanetSurface& planet, SurfaceCellAddress cell) {
    reconcileGasTopology(planet);
    if(auto index=gasRegionContaining(planet.normalize(cell))) {
        const auto& r=gasRegions_[*index];
        return SurfaceGasRegionState{r.room.runtimeId,r.room.anchor,r.room.sealed,r.room.unbounded,static_cast<int>(r.room.cells.size()),r.gas};
    }
    return std::nullopt;
}

bool SurfaceEnvironmentSystem::injectGas(const PlanetSurface& planet,
                                         SurfaceCellAddress anchor,
                                         float pressure,
                                         float oxygen,
                                         float smoke,
                                         float toxin) {
    auto index=gasRegionForAnchor(planet,anchor);
    if(!index) return false;
    auto& gas=gasRegions_[*index].gas;
    gas.pressure+=std::max(0.0f,pressure);
    gas.oxygen+=std::max(0.0f,oxygen);
    gas.smoke+=std::max(0.0f,smoke);
    gas.toxin+=std::max(0.0f,toxin);
    gas=sanitizeGas(gas);
    return true;
}

std::uint64_t SurfaceEnvironmentSystem::addGasLink(SurfaceGasLinkType type,
                                                    SurfaceCellAddress a,
                                                    SurfaceCellAddress b,
                                                    float flowPerSecond,
                                                    float filterEfficiency) {
    SurfaceGasLink link{};
    link.stableId=allocateStableId();
    link.type=type;
    link.a=a;
    link.b=b;
    link.flowPerSecond=std::max(0.0f,flowPerSecond);
    link.filterEfficiency=clamp01(filterEfficiency);
    gasLinks_.push_back(link);
    std::sort(gasLinks_.begin(),gasLinks_.end(),[](const auto& x,const auto& y){return x.stableId<y.stableId;});
    gasGeometryRevision_=0;
    return link.stableId;
}

bool SurfaceEnvironmentSystem::restoreGasLink(const SurfaceGasLink& link) {
    if(link.stableId==0 || findGasLink(link.stableId)) return false;
    auto copy=link;
    copy.flowPerSecond=std::max(0.0f,copy.flowPerSecond);
    copy.filterEfficiency=clamp01(copy.filterEfficiency);
    gasLinks_.push_back(copy);
    std::sort(gasLinks_.begin(),gasLinks_.end(),[](const auto& x,const auto& y){return x.stableId<y.stableId;});
    gasGeometryRevision_=0;
    return true;
}

bool SurfaceEnvironmentSystem::removeGasLink(std::uint64_t stableId) {
    const auto it=std::find_if(gasLinks_.begin(),gasLinks_.end(),[&](const auto& l){return l.stableId==stableId;});
    if(it==gasLinks_.end()) return false;
    gasLinks_.erase(it);
    gasGeometryRevision_=0;
    return true;
}

SurfaceGasLink* SurfaceEnvironmentSystem::findGasLink(std::uint64_t stableId) {
    const auto it=std::find_if(gasLinks_.begin(),gasLinks_.end(),[&](const auto& l){return l.stableId==stableId;});
    return it==gasLinks_.end()?nullptr:&*it;
}

void SurfaceEnvironmentSystem::updateGasLinks(const PlanetSurface& planet, float dt) {
    reconcileGasTopology(planet);
    for(const auto& link:gasLinks_) {
        if(!link.enabled || !link.powered || link.flowPerSecond<=0.0f) continue;
        auto ia=gasRegionForAnchor(planet,link.a);
        auto ib=gasRegionForAnchor(planet,link.b);
        if(!ia || !ib || *ia==*ib) continue;
        auto& a=gasRegions_[*ia];
        auto& b=gasRegions_[*ib];
        const float va=static_cast<float>(std::max<std::size_t>(1,a.room.cells.size()));
        const float vb=static_cast<float>(std::max<std::size_t>(1,b.room.cells.size()));

        std::size_t from=*ia,to=*ib;
        if(link.type!=SurfaceGasLinkType::Pump) {
            if(std::abs(a.gas.pressure-b.gas.pressure)<1e-4f) continue;
            if(b.gas.pressure>a.gas.pressure) {from=*ib;to=*ia;}
        }
        auto& source=gasRegions_[from];
        auto& target=gasRegions_[to];
        const float sourceV=from==*ia?va:vb;
        const float targetV=to==*ia?va:vb;
        const float pressureDelta=link.type==SurfaceGasLinkType::Pump
            ? source.gas.pressure
            : std::max(0.0f,source.gas.pressure-target.gas.pressure);
        if(pressureDelta<=kEpsilon || source.gas.pressure<=kEpsilon) continue;
        const float q=std::min({link.flowPerSecond*dt,
                                sourceV*0.35f,
                                source.gas.pressure*sourceV,
                                std::max(0.0f,(1.5f-target.gas.pressure)*targetV)});
        if(q<=kEpsilon) continue;

        const float pressurePerV=source.gas.pressure;
        const float oxygenPerV=source.gas.oxygen;
        const float smokePerV=source.gas.smoke;
        const float toxinPerV=source.gas.toxin;
        const float pressureMass=pressurePerV*q;
        const float oxygenMass=oxygenPerV*q;
        float smokeMass=smokePerV*q;
        float toxinMass=toxinPerV*q;
        if(link.type==SurfaceGasLinkType::Filter) {
            smokeMass*=1.0f-link.filterEfficiency;
            toxinMass*=1.0f-link.filterEfficiency;
        }

        source.gas.pressure-=pressureMass/sourceV;
        source.gas.oxygen-=oxygenMass/sourceV;
        source.gas.smoke-=smokePerV*q/sourceV;
        source.gas.toxin-=toxinPerV*q/sourceV;
        target.gas.pressure+=pressureMass/targetV;
        target.gas.oxygen+=oxygenMass/targetV;
        target.gas.smoke+=smokeMass/targetV;
        target.gas.toxin+=toxinMass/targetV;
        source.gas=sanitizeGas(source.gas);
        target.gas=sanitizeGas(target.gas);
        ++telemetry_.gasTransfers;
        ++telemetry_.boundedWorkUnits;
    }
}

void SurfaceEnvironmentSystem::updateOpenGas(float dt) {
    for(auto& region:gasRegions_) {
        if(region.room.sealed) continue;
        const float before=region.gas.pressure;
        region.gas.pressure=std::max(0.0f,region.gas.pressure-kOpenVentPressureRate*dt);
        region.gas.oxygen=std::max(0.0f,region.gas.oxygen-kOpenVentGasRate*dt);
        region.gas.smoke=std::max(0.0f,region.gas.smoke-kOpenVentGasRate*dt);
        region.gas.toxin=std::max(0.0f,region.gas.toxin-kOpenVentGasRate*dt);
        region.gas=sanitizeGas(region.gas);
        if(before>=0.25f && region.gas.pressure<0.25f)
            emitEvent(SurfaceEnvironmentEventKind::PressureLoss,region.room.anchor,before);
    }
}

void SurfaceEnvironmentSystem::setFluid(SurfaceCellAddress cell, SurfaceFluidKind kind, float amount, float temperatureC) {
    amount=clamp01(amount);
    if(amount<=kEpsilon) { fluids_.erase(cell); return; }
    fluids_[cell]={kind,amount,temperatureC};
    fluidSettled_=false;
}

std::optional<SurfaceFluidCell> SurfaceEnvironmentSystem::fluidAt(SurfaceCellAddress cell) const {
    const auto it=fluids_.find(cell);
    return it==fluids_.end()?std::nullopt:std::optional<SurfaceFluidCell>{it->second};
}

float SurfaceEnvironmentSystem::totalFluidAmount() const {
    double total=0.0;
    for(const auto& [_,f]:fluids_) total+=f.amount;
    return static_cast<float>(total);
}

std::uint64_t SurfaceEnvironmentSystem::addFluidPump(SurfaceCellAddress from,
                                                      SurfaceCellAddress to,
                                                      float ratePerSecond,
                                                      std::optional<SurfaceFluidKind> kindFilter) {
    SurfaceFluidPump pump{};
    pump.stableId=allocateStableId();
    pump.from=from;
    pump.to=to;
    pump.ratePerSecond=std::max(0.0f,ratePerSecond);
    pump.kindFilter=kindFilter;
    fluidPumps_.push_back(pump);
    std::sort(fluidPumps_.begin(),fluidPumps_.end(),[](const auto& a,const auto& b){return a.stableId<b.stableId;});
    return pump.stableId;
}

bool SurfaceEnvironmentSystem::restoreFluidPump(const SurfaceFluidPump& pump) {
    if(pump.stableId==0 || findFluidPump(pump.stableId)) return false;
    auto copy=pump;
    copy.ratePerSecond=std::max(0.0f,copy.ratePerSecond);
    fluidPumps_.push_back(copy);
    std::sort(fluidPumps_.begin(),fluidPumps_.end(),[](const auto& a,const auto& b){return a.stableId<b.stableId;});
    return true;
}

SurfaceFluidPump* SurfaceEnvironmentSystem::findFluidPump(std::uint64_t stableId) {
    const auto it=std::find_if(fluidPumps_.begin(),fluidPumps_.end(),[&](const auto& p){return p.stableId==stableId;});
    return it==fluidPumps_.end()?nullptr:&*it;
}

void SurfaceEnvironmentSystem::updateFluidPumps(const PlanetSurface& planet, float dt) {
    for(const auto& pump:fluidPumps_) {
        if(!pump.enabled || !pump.powered || pump.ratePerSecond<=0.0f) continue;
        const auto srcIt=fluids_.find(pump.from);
        if(srcIt==fluids_.end() || srcIt->second.amount<=kEpsilon || (pump.kindFilter && *pump.kindFilter!=srcIt->second.kind)) continue;
        if(!planet.radialInBounds(pump.to.radial) || !planet.gasPassableAt(pump.to)) {
            emitEvent(SurfaceEnvironmentEventKind::FluidPumpBlocked,pump.to,1.0f,pump.stableId);
            continue;
        }
        const auto dstIt=fluids_.find(pump.to);
        if(dstIt!=fluids_.end() && dstIt->second.amount>kEpsilon && dstIt->second.kind!=srcIt->second.kind) {
            emitEvent(SurfaceEnvironmentEventKind::FluidPumpBlocked,pump.to,1.0f,pump.stableId);
            continue;
        }
        const float dstAmount=dstIt==fluids_.end()?0.0f:dstIt->second.amount;
        const float move=std::min({srcIt->second.amount,1.0f-dstAmount,pump.ratePerSecond*dt});
        if(move<=kEpsilon) continue;
        const auto kind=srcIt->second.kind;
        const float temp=srcIt->second.temperatureC;
        auto& src=fluids_[pump.from];
        src.amount-=move;
        auto& dst=fluids_[pump.to];
        if(dst.amount<=kEpsilon) {dst.kind=kind;dst.temperatureC=temp;}
        else dst.temperatureC=(dst.temperatureC*dst.amount+temp*move)/(dst.amount+move);
        dst.amount+=move;
        if(src.amount<=kEpsilon) fluids_.erase(pump.from);
        ++telemetry_.fluidTransfers;
        ++telemetry_.boundedWorkUnits;
        fluidSettled_=false;
    }
}

void SurfaceEnvironmentSystem::updateFluids(const PlanetSurface& planet, float /*dt*/) {
    if(fluids_.empty()) {fluidSettled_=true;return;}
    const auto snapshot=fluids_;
    std::map<SurfaceCellAddress,float,SurfaceCellAddressLess> delta;
    std::map<SurfaceCellAddress,SurfaceFluidCell,SurfaceCellAddressLess> introduced;
    int transfers=0;

    auto currentAmount=[&](const SurfaceCellAddress& c, SurfaceFluidKind kind)->float {
        float amount=0.0f;
        if(const auto it=snapshot.find(c);it!=snapshot.end() && it->second.kind==kind) amount=it->second.amount;
        if(const auto d=delta.find(c);d!=delta.end()) amount+=d->second;
        return std::clamp(amount,0.0f,1.0f);
    };

    for(const auto& [cell,fluid]:snapshot) {
        if(transfers>=maxFluidTransfersPerTick_) break;
        float available=currentAmount(cell,fluid.kind);
        if(available<=kEpsilon) continue;

        const SurfaceCellAddress down{cell.face,cell.u,cell.v,cell.radial-1};
        if(down.radial>=0 && planet.gasPassableAt(down)) {
            auto target=fluidAt(down);
            if(sameKindOrEmpty(target,fluid.kind)) {
                const float capacity=1.0f-currentAmount(down,fluid.kind);
                const float move=std::min(available,capacity);
                if(move>kEpsilon) {
                    delta[cell]-=move;delta[down]+=move;
                    introduced[down]={fluid.kind,0.0f,fluid.temperatureC};
                    available-=move;++transfers;
                }
            }
        }
        if(available<=kEpsilon || transfers>=maxFluidTransfersPerTick_) continue;

        for(const auto& side:lateralNeighbors(planet,cell)) {
            if(transfers>=maxFluidTransfersPerTick_) break;
            if(!planet.radialInBounds(side.radial) || !planet.gasPassableAt(side)) continue;
            auto target=fluidAt(side);
            if(!sameKindOrEmpty(target,fluid.kind)) continue;
            const float here=currentAmount(cell,fluid.kind);
            const float there=currentAmount(side,fluid.kind);
            const float diff=here-there;
            if(diff<=0.05f) continue;
            const float move=std::min({0.25f*diff,here,1.0f-there});
            if(move<=kEpsilon) continue;
            delta[cell]-=move;delta[side]+=move;
            introduced[side]={fluid.kind,0.0f,fluid.temperatureC};
            ++transfers;
        }
    }

    for(const auto& [cell,d]:delta) {
        auto it=fluids_.find(cell);
        if(it==fluids_.end()) {
            auto proto=introduced.find(cell);
            if(proto==introduced.end() || d<=kEpsilon) continue;
            auto f=proto->second;f.amount=d;fluids_[cell]=f;
        } else {
            it->second.amount=clamp01(it->second.amount+d);
            if(it->second.amount<=kEpsilon) fluids_.erase(it);
        }
    }

    for(auto& [cell,fluid]:fluids_) {
        if(fluid.kind==SurfaceFluidKind::Acid) contaminate(cell,SurfaceContaminant::Corrosive,0.02f*fluid.amount);
        else if(fluid.kind==SurfaceFluidKind::Industrial) contaminate(cell,SurfaceContaminant::Toxin,0.01f*fluid.amount);
        else if(fluid.kind==SurfaceFluidKind::Lava) addHeat(cell,20.0f*fluid.amount);
        if(fluid.amount>=0.85f) emitEvent(SurfaceEnvironmentEventKind::Flood,cell,fluid.amount);
    }

    telemetry_.fluidTransfers+=transfers;
    telemetry_.boundedWorkUnits+=transfers;
    fluidSettled_=transfers==0;
}

void SurfaceEnvironmentSystem::addHeat(SurfaceCellAddress cell, float deltaC) {
    thermal_[cell]=temperatureAt(cell)+deltaC;
}

float SurfaceEnvironmentSystem::temperatureAt(SurfaceCellAddress cell) const {
    const auto it=thermal_.find(cell);
    return it==thermal_.end()?20.0f:it->second;
}

bool SurfaceEnvironmentSystem::ignite(const PlanetSurface& planet, SurfaceCellAddress solidCell, float intensity, float fuelSeconds) {
    if(!planet.radialInBounds(solidCell.radial) || !flammable(planet.get(solidCell))) return false;
    auto& fire=fires_[planet.normalize(solidCell)];
    const bool wasOut=fire.intensity<=kEpsilon;
    fire.intensity=std::max(fire.intensity,clamp01(intensity));
    fire.fuelSeconds=std::max(fire.fuelSeconds,std::max(0.1f,fuelSeconds));
    addHeat(planet.normalize(solidCell),80.0f*fire.intensity);
    if(wasOut) emitEvent(SurfaceEnvironmentEventKind::Fire,planet.normalize(solidCell),fire.intensity);
    return true;
}

void SurfaceEnvironmentSystem::updateThermal(const PlanetSurface& planet, float dt) {
    if(thermal_.empty()) return;
    const float ambient=ambientTemperature(planet);
    const auto snapshot=thermal_;
    std::map<SurfaceCellAddress,float,SurfaceCellAddressLess> delta;
    int work=0;
    for(const auto& [cell,temp]:snapshot) {
        if(work>=maxThermalCellsPerTick_) break;
        delta[cell]+=(ambient-temp)*kThermalAmbientRelax*dt;
        for(const auto& n:gasNeighbors(planet,cell)) {
            if(work>=maxThermalCellsPerTick_) break;
            if(!planet.radialInBounds(n.radial)) continue;
            const float nt=temperatureAt(n);
            const float exchange=(temp-nt)*kThermalDiffusion*dt/6.0f;
            delta[cell]-=exchange;delta[n]+=exchange;
            ++work;
        }
    }
    for(const auto& [cell,d]:delta) {
        const float next=temperatureAt(cell)+d;
        if(std::abs(next-ambient)<0.25f && !fires_.contains(cell)) thermal_.erase(cell);
        else thermal_[cell]=next;
    }
    telemetry_.boundedWorkUnits+=work;
}

void SurfaceEnvironmentSystem::updateFire(const PlanetSurface& planet, float dt) {
    if(fires_.empty()) return;
    const auto snapshot=fires_;
    std::vector<SurfaceCellAddress> extinguish;
    std::vector<SurfaceCellAddress> igniteList;
    for(const auto& [cell,fire]:snapshot) {
        if(!flammable(planet.get(cell)) || fire.fuelSeconds<=kEpsilon || fire.intensity<=kEpsilon) {extinguish.push_back(cell);continue;}
        addHeat(cell,kFireHeatPerSecond*fire.intensity*dt);
        for(const auto& n:gasNeighbors(planet,cell)) {
            if(!planet.radialInBounds(n.radial)) continue;
            addHeat(n,0.15f*kFireHeatPerSecond*fire.intensity*dt);
            if(planet.gasPassableAt(n)) {
                if(auto region=gasRegionForAnchor(planet,n)) {
                    gasRegions_[*region].gas.smoke+=kFireSmokePerSecond*fire.intensity*dt;
                    gasRegions_[*region].gas=sanitizeGas(gasRegions_[*region].gas);
                }
            } else if(flammable(planet.get(n)) && temperatureAt(n)>=kFireSpreadTemperature && !fires_.contains(n)) {
                igniteList.push_back(n);
            }
        }
        auto it=fires_.find(cell);
        if(it!=fires_.end()) {
            it->second.fuelSeconds=std::max(0.0f,it->second.fuelSeconds-kFireFuelBurnPerSecond*dt);
            if(it->second.fuelSeconds<=kEpsilon) extinguish.push_back(cell);
        }
        emitEvent(SurfaceEnvironmentEventKind::Fire,cell,fire.intensity);
    }
    std::sort(igniteList.begin(),igniteList.end(),SurfaceCellAddressLess{});
    igniteList.erase(std::unique(igniteList.begin(),igniteList.end()),igniteList.end());
    for(const auto& cell:igniteList) ignite(planet,cell,0.35f,4.0f);
    for(const auto& cell:extinguish) fires_.erase(cell);
}

void SurfaceEnvironmentSystem::contaminate(SurfaceCellAddress cell, SurfaceContaminant contaminant, float amount) {
    if(amount<=0.0f) return;
    auto& c=contamination_[cell];
    const auto i=static_cast<std::size_t>(contaminant);
    c.intensity[i]=std::clamp(c.intensity[i]+amount,0.0f,1.0f);
}

void SurfaceEnvironmentSystem::decontaminate(SurfaceCellAddress cell, SurfaceContaminant contaminant, float amount) {
    auto it=contamination_.find(cell);if(it==contamination_.end()) return;
    const auto i=static_cast<std::size_t>(contaminant);
    it->second.intensity[i]=std::max(0.0f,it->second.intensity[i]-std::max(0.0f,amount));
    bool any=false;for(float v:it->second.intensity) any=any||v>kEpsilon;
    if(!any) contamination_.erase(it);
}

SurfaceContaminationCell SurfaceEnvironmentSystem::contaminationAt(SurfaceCellAddress cell) const {
    const auto it=contamination_.find(cell);
    return it==contamination_.end()?SurfaceContaminationCell{}:it->second;
}

void SurfaceEnvironmentSystem::updateContamination(float dt) {
    std::vector<SurfaceCellAddress> erase;
    for(auto& [cell,c]:contamination_) {
        bool any=false;
        for(float& value:c.intensity) {
            value=std::max(0.0f,value-0.0025f*dt);
            any=any||value>kEpsilon;
        }
        const float radiation=c.intensity[static_cast<std::size_t>(SurfaceContaminant::Radiation)];
        const float toxin=c.intensity[static_cast<std::size_t>(SurfaceContaminant::Toxin)];
        const float maxValue=*std::max_element(c.intensity.begin(),c.intensity.end());
        if(radiation>=0.25f) emitEvent(SurfaceEnvironmentEventKind::Radiation,cell,radiation);
        if(toxin>=0.25f) emitEvent(SurfaceEnvironmentEventKind::ToxicGas,cell,toxin);
        if(maxValue>=0.50f) emitEvent(SurfaceEnvironmentEventKind::Contamination,cell,maxValue);
        if(!any) erase.push_back(cell);
    }
    for(const auto& cell:erase) contamination_.erase(cell);
}

void SurfaceEnvironmentSystem::emitEvent(SurfaceEnvironmentEventKind kind,
                                         SurfaceCellAddress cell,
                                         float severity,
                                         std::uint64_t sourceStableId) {
    if(severity<=0.0f) return;
    events_.push_back({kind,cell,severity,sourceStableId});
}

void SurfaceEnvironmentSystem::update(PlanetSurface& planet, float dt) {
    dt=std::clamp(dt,0.0f,1.0f);
    telemetry_={};
    reconcileGasTopology(planet);
    updateGasLinks(planet,dt);
    updateOpenGas(dt);
    updateFluidPumps(planet,dt);
    updateFluids(planet,dt);
    updateFire(planet,dt);
    updateThermal(planet,dt);
    updateContamination(dt);

    std::sort(events_.begin(),events_.end(),[](const auto& a,const auto& b){
        if(SurfaceCellAddressLess{}(a.cell,b.cell)) return true;
        if(SurfaceCellAddressLess{}(b.cell,a.cell)) return false;
        if(a.kind!=b.kind) return static_cast<int>(a.kind)<static_cast<int>(b.kind);
        return a.sourceStableId<b.sourceStableId;
    });
    events_.erase(std::unique(events_.begin(),events_.end(),[](const auto& a,const auto& b){
        return a.kind==b.kind && a.cell==b.cell && a.sourceStableId==b.sourceStableId;
    }),events_.end());

    telemetry_.gasRegions=static_cast<int>(gasRegions_.size());
    telemetry_.sealedGasRegions=static_cast<int>(std::count_if(gasRegions_.begin(),gasRegions_.end(),[](const auto& r){return r.room.sealed;}));
    telemetry_.gasLinks=static_cast<int>(gasLinks_.size());
    telemetry_.activeFluidCells=static_cast<int>(fluids_.size());
    telemetry_.activeThermalCells=static_cast<int>(thermal_.size());
    telemetry_.activeFires=static_cast<int>(fires_.size());
    telemetry_.contaminationCells=static_cast<int>(contamination_.size());
    telemetry_.eventsEmitted=static_cast<int>(events_.size());
    telemetry_.fluidSettled=fluidSettled_;
}

std::vector<SurfaceEnvironmentEvent> SurfaceEnvironmentSystem::consumeEvents() {
    auto out=std::move(events_);
    events_.clear();
    return out;
}

SurfaceEnvironmentInspection SurfaceEnvironmentSystem::inspect(const PlanetSurface& planet, SurfaceCellAddress cell) {
    SurfaceEnvironmentInspection out{};
    out.cell=planet.normalize(cell);
    out.room=roomGraph_.query(planet,out.cell,roomBudget_);
    if(auto gas=gasAt(planet,out.cell)) {out.gas=gas->gas;out.hasGas=true;}
    out.fluid=fluidAt(out.cell);
    out.temperatureC=temperatureAt(out.cell);
    out.contamination=contaminationAt(out.cell);
    if(const auto it=fires_.find(out.cell);it!=fires_.end()) out.fireIntensity=it->second.intensity;

    std::ostringstream read,decision,changed;
    read << "room=" << (out.room.sealed?"sealed":out.room.unbounded?"open":"none")
         << " cells=" << out.room.cells.size()
         << " pressure=" << out.gas.pressure
         << " oxygen=" << out.gas.oxygen
         << " fluid=" << (out.fluid?out.fluid->amount:0.0f)
         << " tempC=" << out.temperatureC;
    decision << (out.gas.breathable()?"breathable":"not-breathable")
             << "; fire=" << (out.fireIntensity>0.0f?"active":"none")
             << "; contamination-max=" << *std::max_element(out.contamination.intensity.begin(),out.contamination.intensity.end());
    changed << "spatial fields are sparse; structural world mutation is not owned by SurfaceEnvironmentSystem";
    out.read=read.str();out.decided=decision.str();out.changed=changed.str();
    return out;
}

SurfaceEnvironmentSnapshot SurfaceEnvironmentSystem::snapshot() const {
    SurfaceEnvironmentSnapshot out{};
    for(const auto& region:gasRegions_) if(gasNonZero(region.gas)) out.gasSeeds.push_back({region.room.anchor,region.gas});
    out.gasLinks=gasLinks_;
    for(const auto& [cell,fluid]:fluids_) out.fluids.push_back({cell,fluid});
    out.fluidPumps=fluidPumps_;
    for(const auto& [cell,temp]:thermal_) out.thermal.push_back({cell,temp});
    for(const auto& [cell,c]:contamination_) out.contamination.push_back({cell,c});
    for(const auto& [cell,fire]:fires_) out.fires.push_back({cell,fire});
    return out;
}

bool SurfaceEnvironmentSystem::restore(const PlanetSurface& planet,
                                       const SurfaceEnvironmentSnapshot& snapshotState,
                                       std::string* error) {
    auto fail=[&](const std::string& message){if(error)*error=message;return false;};
    gasRegions_.clear();gasLinks_.clear();fluids_.clear();fluidPumps_.clear();thermal_.clear();contamination_.clear();fires_.clear();events_.clear();
    roomGraph_.invalidate();gasGeometryRevision_=0;

    std::set<std::uint64_t> ids;
    for(const auto& link:snapshotState.gasLinks) {
        if(link.stableId==0 || !ids.insert(link.stableId).second) return fail("duplicate/zero gas-link stable ID");
        if(!restoreGasLink(link)) return fail("invalid gas-link record");
    }
    for(const auto& pump:snapshotState.fluidPumps) {
        if(pump.stableId==0 || !ids.insert(pump.stableId).second) return fail("duplicate/zero fluid-pump stable ID");
        if(!restoreFluidPump(pump)) return fail("invalid fluid-pump record");
    }
    for(const auto& r:snapshotState.fluids) setFluid(r.cell,r.fluid.kind,r.fluid.amount,r.fluid.temperatureC);
    for(const auto& r:snapshotState.thermal) thermal_[r.cell]=r.temperatureC;
    for(const auto& r:snapshotState.contamination) contamination_[r.cell]=r.contamination;
    for(const auto& r:snapshotState.fires) fires_[r.cell]=r.fire;
    for(const auto& seed:snapshotState.gasSeeds) {
        if(!setRoomGas(planet,seed.anchor,seed.gas)) return fail("gas seed anchor is not a passable bounded query start");
    }
    if(error) error->clear();
    return true;
}

} // namespace elysium
