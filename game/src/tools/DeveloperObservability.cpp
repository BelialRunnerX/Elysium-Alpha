// Intended function: imported tools implementation for DeveloperObservability; preserves the agent-authored subsystem contract for later integration/debugging.
#include "tools/DeveloperObservability.hpp"
#include "world/MachineRuntime.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>

namespace elysium {
namespace {

bool isLogisticsMachine(MachineType type) {
    return type==MachineType::Conveyor || type==MachineType::Sorter || type==MachineType::CargoLoader;
}

int inventoryStacks(const SurfaceMachineObject& machine) {
    return static_cast<int>(std::count_if(machine.inventory.begin(),machine.inventory.end(),[](const auto& s){return s.itemId>0 && s.count>0;}));
}

bool itemCouldFit(const SurfaceMachineObject& machine,int itemId,int count=1) {
    if(itemId<=0 || count<=0) return false;
    for(const auto& stack:machine.inventory) {
        if(stack.itemId==itemId && stack.count+count<=SurfaceIndustrySystem::MaxStackCount) return true;
    }
    return inventoryStacks(machine)<SurfaceIndustrySystem::inventoryCapacity(machine);
}

std::vector<std::uint64_t> sortedIds(const std::vector<std::uint64_t>& input) {
    auto out=input;
    std::sort(out.begin(),out.end());
    out.erase(std::unique(out.begin(),out.end()),out.end());
    return out;
}

std::string boolText(bool value) { return value?"true":"false"; }

std::string floatText(float value,int precision=2) {
    std::ostringstream out;
    out<<std::fixed<<std::setprecision(precision)<<value;
    return out.str();
}

bool sameChunk(const PlanetSurface& planet,SurfaceCellAddress address,const PlanetChunkAddress& chunk) {
    return planet.chunkOf(address)==chunk;
}

} // namespace

const char* stableObjectKindName(StableObjectKind kind) {
    switch(kind) {
        case StableObjectKind::Missing: return "missing";
        case StableObjectKind::Machine: return "machine";
        case StableObjectKind::Portal: return "portal";
        case StableObjectKind::AirlockAssembly: return "airlock-assembly";
        case StableObjectKind::AutomationRule: return "automation-rule";
    }
    return "unknown";
}

std::string surfaceAddressText(SurfaceCellAddress a) {
    std::ostringstream out;
    out<<toString(a.face)<<":"<<a.u<<","<<a.v<<","<<a.radial;
    return out.str();
}

std::string chunkAddressText(PlanetChunkAddress a) {
    std::ostringstream out;
    out<<toString(a.face)<<":"<<a.u<<","<<a.v<<","<<a.radial;
    return out.str();
}

SystemTraceRecorder::SystemTraceRecorder(std::size_t capacity)
    : capacity_(std::max<std::size_t>(1,capacity)) {}

void SystemTraceRecorder::record(DiagnosticTraceEvent event) {
    std::lock_guard lock(mutex_);
    if(events_.size()>=capacity_) {
        events_.pop_front();
        ++dropped_;
    }
    events_.push_back({nextSequence_++,std::move(event)});
}

std::vector<RecordedTraceEvent> SystemTraceRecorder::snapshot() const {
    std::lock_guard lock(mutex_);
    return {events_.begin(),events_.end()};
}

std::vector<RecordedTraceEvent> SystemTraceRecorder::forStableId(std::uint64_t stableId) const {
    std::vector<RecordedTraceEvent> out;
    std::lock_guard lock(mutex_);
    for(const auto& event:events_) if(event.event.stableId==stableId) out.push_back(event);
    return out;
}

void SystemTraceRecorder::clear() {
    std::lock_guard lock(mutex_);
    events_.clear();
    dropped_=0;
}

std::size_t SystemTraceRecorder::dropped() const {
    std::lock_guard lock(mutex_);
    return dropped_;
}

StableObjectInspection DeveloperObservability::lookup(const SurfaceInfrastructure& infrastructure,
                                                       std::uint64_t stableId) {
    StableObjectInspection out{};
    out.stableId=stableId;
    if(const auto* machine=infrastructure.find(stableId)) {
        out.kind=StableObjectKind::Machine;
        out.loaded=true;
        out.ownerAddress=machine->anchor;
        out.label=machineName(machine->type);
        return out;
    }
    if(const auto* portal=infrastructure.findPortal(stableId)) {
        out.kind=StableObjectKind::Portal;
        out.loaded=true;
        out.ownerAddress=portal->anchor;
        out.label=surfacePortalName(portal->type);
        return out;
    }
    if(const auto* airlock=infrastructure.findAirlockAssembly(stableId)) {
        out.kind=StableObjectKind::AirlockAssembly;
        out.loaded=true;
        out.ownerAddress=airlock->chamberAnchor;
        out.label="Airlock Assembly";
        return out;
    }
    if(const auto* rule=infrastructure.findAutomationRule(stableId)) {
        out.kind=StableObjectKind::AutomationRule;
        out.loaded=true;
        if(const auto* controller=infrastructure.find(rule->controllerMachineId)) out.ownerAddress=controller->anchor;
        out.label="Automation Rule";
        return out;
    }
    out.label="StableId not loaded";
    return out;
}

RoomAtmosphereInspection DeveloperObservability::room(const PlanetSurface& planet,
                                                       const SurfaceInfrastructure& infrastructure,
                                                       std::uint64_t stableId,
                                                       int maxRoomCells) {
    RoomAtmosphereInspection out{};
    out.sourceStableId=stableId;
    const auto* machine=infrastructure.find(stableId);
    if(!machine || machine->type!=MachineType::AtmosphereUnit) {
        out.cause=machine?"stable object is not an Atmosphere Unit":"StableId is not loaded";
        return out;
    }
    out.sourceLoaded=true;
    out.anchor=machine->anchor;
    out.pressure=machine->roomPressure;
    out.oxygen=machine->roomOxygen;
    out.enabled=machine->enabled;
    out.powered=machine->powered;
    const auto volume=planet.sealedVolume(machine->anchor,maxRoomCells);
    out.sealed=volume.sealed && !volume.truncated;
    out.truncated=volume.truncated;
    // Authoritative bounded-volume contract exposes sealed/truncated/cells only.
    // A completed, non-truncated, non-empty flood fill that is not sealed reached
    // the radial sky boundary. Frontier coordinates are intentionally not part
    // of the authoritative query contract.
    out.leakedToSky=!volume.sealed && !volume.truncated && !volume.cells.empty();
    out.hasLeakFrontier=false;
    out.leakFrontier={};
    out.reachableCells=volume.cells.size();
    if(out.truncated) out.cause="reachable gas volume exhausted the configured pressurization budget";
    else if(out.leakedToSky) out.cause="reachable gas volume has a frontier open to sky";
    else if(!volume.sealed) out.cause="start cell is solid/out of bounds or volume is otherwise open";
    else if(!machine->enabled) out.cause="room is sealed but Atmosphere Unit is disabled";
    else if(!machine->powered) out.cause="room is sealed but Atmosphere Unit is shed by power allocation";
    else if(machine->roomPressure<0.55f || machine->roomOxygen<0.45f) out.cause="room is sealed and filling but is not yet breathable";
    else out.cause="sealed breathable room";
    return out;
}

PowerGraphInspection DeveloperObservability::power(const SurfaceInfrastructure& infrastructure) {
    PowerGraphInspection out{};
    out.authoritativeSummary=infrastructure.summary();
    std::map<std::uint64_t,PowerNetworkInspection> networks;
    for(const auto& machine:infrastructure.objects()) {
        const std::uint64_t networkId=machine.powerNetworkId;
        auto& network=networks[networkId];
        network.networkId=networkId;
        PowerNodeInspection node{};
        node.stableId=machine.stableId;
        node.type=machine.type;
        node.enabled=machine.enabled;
        node.powered=machine.powered;
        node.generation=(machine.enabled && machine.type==MachineType::BurnerGenerator && machine.fuelSeconds>0.0f)
            ? machinePowerGeneration(machine.type) : 0.0f;
        node.demand=machine.enabled?machinePowerDemand(machine.type):0.0f;
        node.priority=machinePowerPriority(machine.type);
        node.storedEnergy=machine.type==MachineType::BatteryBank?machine.storedEnergy:0.0f;
        node.storageCapacity=machinePowerStorageCapacity(machine.type);
        network.generation+=node.generation;
        network.demand+=node.demand;
        if(node.powered) network.supplied+=node.demand;
        network.storedEnergy+=node.storedEnergy;
        network.storageCapacity+=node.storageCapacity;
        network.nodes.push_back(node);
    }
    for(auto& [_,network]:networks) {
        std::sort(network.nodes.begin(),network.nodes.end(),[](const auto& a,const auto& b){return a.stableId<b.stableId;});
        network.brownout=std::any_of(network.nodes.begin(),network.nodes.end(),[](const auto& node){
            return node.enabled && node.demand>0.0f && !node.powered;
        });
        out.networks.push_back(std::move(network));
    }
    return out;
}

LogisticsGraphInspection DeveloperObservability::logistics(const SurfaceInfrastructure& infrastructure) {
    LogisticsGraphInspection out{};
    for(const auto& transport:infrastructure.objects()) {
        if(!isLogisticsMachine(transport.type)) continue;
        LogisticsEdgeInspection edge{};
        edge.transportStableId=transport.stableId;
        edge.transportType=transport.type;
        edge.sourceStableId=transport.logisticsSourceStableId;
        edge.targetStableId=transport.logisticsTargetStableId;
        edge.alternateTargetStableId=transport.logisticsAlternateTargetStableId;
        edge.enabled=transport.enabled;
        edge.powered=transport.powered;
        edge.blocked=transport.logisticsBlocked;
        const auto* source=infrastructure.find(edge.sourceStableId);
        const auto* target=infrastructure.find(edge.targetStableId);
        const auto* alternate=edge.alternateTargetStableId?infrastructure.find(edge.alternateTargetStableId):nullptr;
        edge.sourceLoaded=source!=nullptr;
        edge.targetLoaded=target!=nullptr;
        edge.alternateLoaded=edge.alternateTargetStableId==0 || alternate!=nullptr;
        if(source) edge.sourceStacks=inventoryStacks(*source);
        if(target) {
            edge.targetStacks=inventoryStacks(*target);
            edge.targetStackCapacity=SurfaceIndustrySystem::inventoryCapacity(*target);
        }
        if(!transport.enabled) edge.bottleneck="transport disabled";
        else if(!edge.sourceStableId || !edge.targetStableId || (transport.type==MachineType::Sorter && !edge.alternateTargetStableId))
            edge.bottleneck="missing stable source/target link";
        else if(!source) edge.bottleneck="source StableId is unloaded or missing";
        else if(!target) edge.bottleneck="target StableId is unloaded or missing";
        else if(transport.type==MachineType::Sorter && !alternate) edge.bottleneck="sorter alternate target StableId is unloaded or missing";
        else if(!transport.powered) edge.bottleneck="brownout: transport is not powered";
        else if(source->inventory.empty()) edge.bottleneck="source inventory empty";
        else {
            bool routable=false;
            for(const auto& stack:source->inventory) {
                const SurfaceMachineObject* chosen=target;
                if(transport.type==MachineType::Sorter && stack.itemId!=transport.sorterFilterItemId) chosen=alternate;
                if(chosen && itemCouldFit(*chosen,stack.itemId,1)) {routable=true;break;}
            }
            if(!routable || transport.logisticsBlocked) edge.bottleneck="destination capacity/filter produces a jam";
            else edge.bottleneck="flow ready";
        }
        out.edges.push_back(std::move(edge));
    }
    std::sort(out.edges.begin(),out.edges.end(),[](const auto& a,const auto& b){return a.transportStableId<b.transportStableId;});
    return out;
}

WhyInspection DeveloperObservability::why(const PlanetSurface& planet,
                                           const SurfaceInfrastructure& infrastructure,
                                           std::uint64_t stableId,
                                           int maxRoomCells) {
    WhyInspection out{};
    out.subject=lookup(infrastructure,stableId);
    if(!out.subject.loaded) {
        out.blockers.push_back("StableId is not present in the active infrastructure snapshot.");
        out.remediation.push_back("Load/promote the owning shard or inspect the persistence/Chronicle record by StableId.");
        return out;
    }
    if(const auto* machine=infrastructure.find(stableId)) {
        out.currentState.push_back(std::string("type=")+machineName(machine->type));
        out.currentState.push_back("anchor="+surfaceAddressText(machine->anchor));
        out.currentState.push_back("enabled="+boolText(machine->enabled)+" powered="+boolText(machine->powered));
        out.currentState.push_back("powerNetworkId="+std::to_string(machine->powerNetworkId));
        const float demand=machinePowerDemand(machine->type);
        if(demand>0.0f) {
            out.relevantInputs.push_back("power demand="+floatText(demand)+" priority="+std::to_string(machinePowerPriority(machine->type)));
            if(machine->enabled && !machine->powered) {
                out.blockers.push_back("Power allocator shed this load (brownout or isolated network).");
                out.remediation.push_back("Increase local generation/storage, reduce higher-priority demand, or restore network proximity.");
            } else if(machine->powered) out.lastDecisions.push_back("Power allocator supplied this load.");
        }
        if(machine->type==MachineType::AtmosphereUnit) {
            const auto r=room(planet,infrastructure,stableId,maxRoomCells);
            out.relevantInputs.push_back("reachable gas cells="+std::to_string(r.reachableCells));
            out.currentState.push_back("pressure="+floatText(r.pressure)+" oxygen="+floatText(r.oxygen));
            if(!r.sealed) {
                out.blockers.push_back("Atmosphere cannot pressurize: "+r.cause+".");
                if(r.hasLeakFrontier) out.dependencies.push_back("leak frontier="+surfaceAddressText(r.leakFrontier));
                out.remediation.push_back(r.truncated?"Split the room or raise the intentional pressurization budget after design review.":"Seal the reported frontier/open portal, then allow the unit to repressurize.");
            } else out.lastDecisions.push_back("Bounded gas flood fill terminated before budget: room is sealed.");
        }
        if(machine->activeRecipeId!=0) {
            if(const auto* recipe=surfaceRecipe(static_cast<SurfaceRecipeId>(machine->activeRecipeId))) {
                out.currentState.push_back("active recipe="+std::string(recipe->name)+" progress="+floatText(machine->processProgressSeconds)+"/"+floatText(recipe->processSeconds));
            }
        } else if(machine->selectedRecipeId!=0) {
            if(const auto* recipe=surfaceRecipe(static_cast<SurfaceRecipeId>(machine->selectedRecipeId)))
                out.currentState.push_back("selected recipe="+std::string(recipe->name));
        }
        if(isLogisticsMachine(machine->type)) {
            out.dependencies.push_back("source StableId="+std::to_string(machine->logisticsSourceStableId));
            out.dependencies.push_back("target StableId="+std::to_string(machine->logisticsTargetStableId));
            for(const auto& edge:logistics(infrastructure).edges) if(edge.transportStableId==stableId) {
                out.currentState.push_back("logistics="+edge.bottleneck);
                if(edge.bottleneck!="flow ready") out.blockers.push_back(edge.bottleneck+".");
                break;
            }
        }
        return out;
    }
    if(const auto* portal=infrastructure.findPortal(stableId)) {
        out.currentState.push_back(std::string("type=")+surfacePortalName(portal->type));
        out.currentState.push_back("anchor="+surfaceAddressText(portal->anchor));
        out.currentState.push_back("open="+boolText(portal->open));
        if(const auto* assembly=infrastructure.airlockForPortal(stableId)) {
            out.dependencies.push_back("airlock assembly StableId="+std::to_string(assembly->stableId));
            out.currentState.push_back(std::string("assembly state=")+surfaceAirlockStateName(assembly->state));
        }
        return out;
    }
    if(const auto* airlock=infrastructure.findAirlockAssembly(stableId)) {
        out.currentState.push_back(std::string("state=")+surfaceAirlockStateName(airlock->state));
        out.currentState.push_back("chamber pressure="+floatText(airlock->chamberPressure)+" oxygen="+floatText(airlock->chamberOxygen));
        out.dependencies.push_back("controller="+std::to_string(airlock->controllerMachineId));
        out.dependencies.push_back("inner portal="+std::to_string(airlock->innerPortalId));
        out.dependencies.push_back("outer portal="+std::to_string(airlock->outerPortalId));
        if(airlock->state==SurfaceAirlockState::Fault) {
            out.blockers.push_back("Airlock interlock is in Fault state.");
            out.remediation.push_back("Inspect controller power and both stable portal references before retrying the cycle.");
        }
        return out;
    }
    if(const auto* rule=infrastructure.findAutomationRule(stableId)) {
        out.currentState.push_back(std::string("trigger=")+surfaceAutomationTriggerName(rule->trigger));
        out.currentState.push_back(std::string("action=")+surfaceAutomationActionName(rule->action));
        out.currentState.push_back("enabled="+boolText(rule->enabled));
        out.dependencies.push_back("controller="+std::to_string(rule->controllerMachineId));
        out.dependencies.push_back("source="+std::to_string(rule->sourceStableId));
        out.dependencies.push_back("target="+std::to_string(rule->targetStableId));
    }
    return out;
}

PathInspection DeveloperObservability::path(const PlanetSurface& planet,
                                             const SurfaceNavigationService& navigation,
                                             const SurfaceWorldReadService& read,
                                             Vec3 start,Vec3 goal,float hoverOffset,float agentRadius,float maxStepHeight) {
    PathInspection out{};
    out.worldRevision=planet.revision();
    out.cacheRevisionBefore=navigation.cachedRevision();
    out.cacheWasStale=out.cacheRevisionBefore!=out.worldRevision && navigation.cachedRouteCount()>0;
    const auto before=navigation.stats();
    const auto result=navigation.findPath(read,start,goal,hoverOffset,agentRadius,maxStepHeight);
    const auto after=navigation.stats();
    out.cacheRevisionAfter=navigation.cachedRevision();
    out.cacheInvalidatedByQuery=after.revisionInvalidations>before.revisionInvalidations;
    out.found=result.found;
    out.fromCache=result.fromCache;
    out.expanded=result.expanded;
    out.maxExpanded=navigation.maxExpandedNodes();
    for(const auto& waypoint:result.waypoints) {
        auto a=planet.locate(waypoint);
        a.radial=PlanetSurface::ReferenceRadial;
        if(out.routeColumns.empty() || !(out.routeColumns.back()==a)) out.routeColumns.push_back(a);
    }
    if(out.cacheInvalidatedByQuery) out.cause="cached routes were stale because PlanetSurface revision changed; cache was invalidated";
    else if(result.fromCache) out.cause="route served from current-revision cache";
    else if(result.found) out.cause="bounded A* found a route after cache miss";
    else if(result.expanded>=navigation.maxExpandedNodes()) out.cause="route search exhausted the bounded expansion budget";
    else out.cause="no traversable route was found";
    return out;
}

ChunkInspection DeveloperObservability::chunk(const PlanetSurface& planet,const PlanetChunkAddress& address) {
    ChunkInspection out{};
    out.address=address;
    out.chunkRevision=planet.chunkRevision(address);
    out.influence=planet.editInfluenceSummary(address);
    if(const auto* journal=planet.journal(address)) {
        out.macroEdits=static_cast<int>(journal->macroEdits.size());
        out.placedMarkers=static_cast<int>(journal->placedMarkers.size());
        out.refinedCells=static_cast<int>(journal->microBricks.size());
        out.microOverrides=journal->microOverrideCount();
    }
    return out;
}

PersistenceDiffInspection DeveloperObservability::persistenceDiff(const PlanetSurface& planet,
                                                                   const SurfaceInfrastructure& infrastructure,
                                                                   const PlanetChunkAddress& address) {
    PersistenceDiffInspection out{};
    out.address=address;
    out.worldRevision=planet.revision();
    if(const auto* journal=planet.journal(address)) {
        std::vector<int> indices;
        indices.reserve(journal->macroEdits.size());
        for(const auto& [idx,_]:journal->macroEdits) indices.push_back(idx);
        std::sort(indices.begin(),indices.end());
        for(const int idx:indices) {
            const auto a=planet.cellFromFlatIndex(idx);
            out.macroDeltas.push_back({a,planet.baseline(a),planet.get(a),planet.playerPlaced(a)});
        }
        out.refinedCells=static_cast<int>(journal->microBricks.size());
        out.microOverrides=journal->microOverrideCount();
    }
    for(const auto& machine:infrastructure.objects()) if(sameChunk(planet,machine.anchor,address)) out.machineIds.push_back(machine.stableId);
    for(const auto& portal:infrastructure.portals()) if(sameChunk(planet,portal.anchor,address)) out.portalIds.push_back(portal.stableId);
    out.machineIds=sortedIds(out.machineIds);
    out.portalIds=sortedIds(out.portalIds);
    return out;
}

InfrastructureJournalInspection DeveloperObservability::journal(const std::vector<InfrastructureJournalRecord>& records) {
    InfrastructureJournalInspection out{};
    for(const auto& record:records) {
        if(record.op==InfrastructureJournalOp::Upsert) ++out.upserts;
        else ++out.tombstones;
        std::string error;
        if(!validateInfrastructureJournalRecord(record,&error)) {
            ++out.invalidRecords;
            out.issues.push_back(std::string(infrastructureRecordKindName(record.kind))+" "+std::to_string(record.stableId)+": "+error);
        }
    }
    std::string closureError;
    if(!records.empty() && !validateInfrastructureSnapshotClosure(records,&closureError))
        out.issues.push_back("snapshot closure: "+closureError);
    return out;
}

WorkerTelemetryInspection DeveloperObservability::workers(const JobSystem& jobs) {
    return {jobs.stats()};
}

RendererTelemetryInspection DeveloperObservability::renderer(const PlanetSurfaceRenderer& renderer) {
    RendererTelemetryInspection out{};
    out.pendingJobs=renderer.pendingJobs();
    out.dirtyChunks=renderer.dirtyChunks();
    out.rebuiltChunksLastSync=renderer.rebuiltChunksLastSync();
    out.quads=renderer.quads();
    out.triangles=renderer.triangles();
    out.fullDetailChunks=renderer.fullDetailChunks();
    out.nearFieldChunks=renderer.nearFieldChunks();
    out.farFieldChunks=renderer.farFieldChunks();
    out.cache=renderer.cpuCacheStats();
    return out;
}

JobDebugInspection DeveloperObservability::job(const JobDiagnosticSnapshot& snapshot) {
    JobDebugInspection out{};
    out.jobStableId=snapshot.jobStableId;
    out.summary=snapshot.jobType+" state="+snapshot.state;
    for(const auto& candidate:snapshot.candidates) if(candidate.eligible) out.eligibleWorkers.push_back(candidate.workerStableId);
    out.eligibleWorkers=sortedIds(out.eligibleWorkers);
    if(out.eligibleWorkers.empty()) {
        out.blockers.push_back("no eligible worker");
        std::set<std::string> reasons;
        for(const auto& candidate:snapshot.candidates) for(const auto& reason:candidate.rejectionReasons) if(!reason.empty()) reasons.insert(reason);
        for(const auto& reason:reasons) out.blockers.push_back("worker eligibility: "+reason);
    }
    for(const auto& reservation:snapshot.reservations) if(!reservation.acquired) {
        std::string reason="reservation failed for StableId="+std::to_string(reservation.resourceStableId);
        if(reservation.ownerJobStableId && reservation.ownerJobStableId!=snapshot.jobStableId)
            reason += " claimed by job="+std::to_string(reservation.ownerJobStableId);
        if(!reservation.failureReason.empty()) reason += " ("+reservation.failureReason+")";
        out.blockers.push_back(std::move(reason));
    }
    if(!snapshot.unmetDependencyIds.empty()) {
        auto ids=sortedIds(snapshot.unmetDependencyIds);
        std::ostringstream line; line<<"unmet dependencies:";
        for(auto id:ids) line<<' '<<id;
        out.blockers.push_back(line.str());
    }
    if(snapshot.pathRequested && !snapshot.pathFound)
        out.blockers.push_back("path/access failure: "+(snapshot.pathFailureReason.empty()?std::string("no route"):snapshot.pathFailureReason));
    if(!snapshot.explicitFailureReason.empty()) out.blockers.push_back("job failure: "+snapshot.explicitFailureReason);
    if(out.blockers.empty()) out.summary += " (no current blocker reported by authoritative snapshot)";
    return out;
}


EntityInspection DeveloperObservability::entity(const EntityDiagnosticSnapshot& snapshot) {
    EntityInspection out{};
    out.stableId=snapshot.stableId;
    out.loaded=snapshot.loaded;
    out.remote=snapshot.remote;
    out.kind=snapshot.kind;
    out.shard=snapshot.shard;
    out.components=snapshot.components;
    std::stable_sort(out.components.begin(),out.components.end(),[](const auto& a,const auto& b){return a.component<b.component;});
    if(snapshot.loaded) out.lookupNote="resolved from authoritative active snapshot by StableId";
    else if(snapshot.remote) out.lookupNote="StableId is remote/compacted; active components are unavailable by design";
    else out.lookupNote="StableId is not loaded; no process-local entity handle was guessed";
    return out;
}

GraphicsTelemetryInspection DeveloperObservability::graphics(const GraphicsTelemetrySnapshot& snapshot) {
    return {snapshot};
}

CitizenInspection DeveloperObservability::citizen(const CitizenDiagnosticSnapshot& snapshot) {
    CitizenInspection out{};
    out.stableId=snapshot.stableId;
    out.loaded=snapshot.loaded;
    out.remote=snapshot.remote;
    std::ostringstream summary;
    summary<<(snapshot.name.empty()?"unnamed citizen":snapshot.name)<<" state="<<(snapshot.state.empty()?"unknown":snapshot.state)
           <<" stress="<<floatText(snapshot.stress)<<" focus="<<floatText(snapshot.focus);
    if(snapshot.currentJobStableId) summary<<" job="<<snapshot.currentJobStableId;
    if(!snapshot.loaded && snapshot.remote) summary<<" [remote/compacted]";
    else if(!snapshot.loaded) summary<<" [unloaded]";
    out.summary=summary.str();
    out.attentionReasons=snapshot.explicitAttentionReasons;
    for(const auto& need:snapshot.influentialNeeds) if(!need.empty()) out.attentionReasons.push_back("need: "+need);
    for(const auto& limit:snapshot.capabilityLimits) if(!limit.empty()) out.attentionReasons.push_back("capability: "+limit);
    return out;
}

RelationshipGraphInspection DeveloperObservability::relationships(std::uint64_t focusStableId,
                                                                    const std::vector<RelationshipEdgeDiagnostic>& edges) {
    RelationshipGraphInspection out{};
    out.focusStableId=focusStableId;
    for(const auto& edge:edges) {
        if(focusStableId!=0 && edge.sourceStableId!=focusStableId && edge.targetStableId!=focusStableId) continue;
        out.edges.push_back(edge);
        if(edge.sourceStableId) out.referencedStableIds.push_back(edge.sourceStableId);
        if(edge.targetStableId) out.referencedStableIds.push_back(edge.targetStableId);
    }
    std::sort(out.edges.begin(),out.edges.end(),[](const auto& a,const auto& b){
        if(a.sourceStableId!=b.sourceStableId) return a.sourceStableId<b.sourceStableId;
        if(a.targetStableId!=b.targetStableId) return a.targetStableId<b.targetStableId;
        return a.type<b.type;
    });
    out.referencedStableIds=sortedIds(out.referencedStableIds);
    return out;
}

EnvironmentalVolumeInspection DeveloperObservability::environment(const EnvironmentalVolumeDiagnosticSnapshot& snapshot) {
    EnvironmentalVolumeInspection out{};
    out.stableId=snapshot.stableId;
    std::ostringstream summary;
    summary<<(snapshot.kind.empty()?"environment volume":snapshot.kind)<<" StableId="<<snapshot.stableId
           <<" loaded="<<boolText(snapshot.loaded)<<" cells="<<snapshot.cells
           <<" bounded="<<boolText(snapshot.bounded)<<" truncated="<<boolText(snapshot.truncated)
           <<" pressure="<<floatText(snapshot.pressure)<<" oxygen="<<floatText(snapshot.oxygen)
           <<" temperature="<<floatText(snapshot.temperature)<<" contamination="<<floatText(snapshot.contamination);
    out.summary=summary.str();
    if(!snapshot.loaded) out.hazards.push_back("volume is not loaded; inspect its persisted/remote record instead of assuming local state");
    if(snapshot.truncated) out.hazards.push_back("bounded query exhausted its configured budget");
    for(const auto& source:snapshot.activeSources) if(!source.empty()) out.hazards.push_back("source: "+source);
    return out;
}

MedicalInspection DeveloperObservability::medical(const MedicalDiagnosticSnapshot& snapshot) {
    MedicalInspection out{};
    out.citizenStableId=snapshot.citizenStableId;
    out.urgent=snapshot.urgent;
    if(!snapshot.loaded) out.blockers.push_back("citizen body state is not loaded; use persisted medical/history record");
    for(const auto& diagnosis:snapshot.diagnoses) if(!diagnosis.empty()) out.blockers.push_back("diagnosis: "+diagnosis);
    for(const auto& part:snapshot.bodyParts) {
        if(part.missing) out.blockers.push_back(part.part+": missing");
        for(const auto& wound:part.wounds) if(!wound.empty()) out.blockers.push_back(part.part+": "+wound);
        for(const auto& capability:part.capabilityLosses) if(!capability.empty()) out.blockers.push_back("capability: "+capability);
    }
    out.treatmentPlan=snapshot.treatmentPlan;
    return out;
}

MilitaryReadinessInspection DeveloperObservability::military(const MilitaryReadinessSnapshot& snapshot) {
    MilitaryReadinessInspection out{};
    out.squadStableId=snapshot.squadStableId;
    if(snapshot.assignedMembers<=0) {
        out.readiness=0.0f;
        out.blockers.push_back("no assigned members");
    } else {
        const float present=static_cast<float>(std::clamp(snapshot.presentMembers,0,snapshot.assignedMembers))/static_cast<float>(snapshot.assignedMembers);
        const float equipped=static_cast<float>(std::clamp(snapshot.equippedMembers,0,snapshot.assignedMembers))/static_cast<float>(snapshot.assignedMembers);
        const float trained=static_cast<float>(std::clamp(snapshot.trainedMembers,0,snapshot.assignedMembers))/static_cast<float>(snapshot.assignedMembers);
        const float ammo=snapshot.requiredAmmo<=0?1.0f:std::clamp(static_cast<float>(snapshot.availableAmmo)/static_cast<float>(snapshot.requiredAmmo),0.0f,1.0f);
        out.readiness=(present+equipped+trained+ammo)*0.25f;
        if(snapshot.presentMembers<snapshot.assignedMembers) out.blockers.push_back("members absent: "+std::to_string(snapshot.assignedMembers-snapshot.presentMembers));
        if(snapshot.equippedMembers<snapshot.assignedMembers) out.blockers.push_back("equipment incomplete for "+std::to_string(snapshot.assignedMembers-snapshot.equippedMembers)+" members");
        if(snapshot.trainedMembers<snapshot.assignedMembers) out.blockers.push_back("training incomplete for "+std::to_string(snapshot.assignedMembers-snapshot.trainedMembers)+" members");
        if(snapshot.availableAmmo<snapshot.requiredAmmo) out.blockers.push_back("ammo shortfall: "+std::to_string(snapshot.requiredAmmo-snapshot.availableAmmo));
    }
    for(const auto& fault:snapshot.explicitFaults) if(!fault.empty()) out.blockers.push_back(fault);
    return out;
}

ChronicleInspection DeveloperObservability::chronicle(std::uint64_t focusStableId,
                                                        const std::vector<ChronicleEventDiagnostic>& events,
                                                        std::size_t limit) {
    ChronicleInspection out{};
    out.focusStableId=focusStableId;
    for(const auto& event:events) {
        bool matches=focusStableId==0 || event.eventStableId==focusStableId || event.siteStableId==focusStableId ||
            std::find(event.participantStableIds.begin(),event.participantStableIds.end(),focusStableId)!=event.participantStableIds.end();
        if(!matches) continue;
        out.events.push_back(event);
    }
    std::stable_sort(out.events.begin(),out.events.end(),[](const auto& a,const auto& b){
        if(a.timeKey!=b.timeKey) return a.timeKey<b.timeKey;
        return a.eventStableId<b.eventStableId;
    });
    if(limit>0 && out.events.size()>limit) out.events.erase(out.events.begin(),out.events.end()-static_cast<std::ptrdiff_t>(limit));
    return out;
}

WorldgenWindowInspection DeveloperObservability::worldgenWindow(const PlanetSurface& planet,
                                                                 CubeFace face,int uBegin,int vBegin,int width,int height,
                                                                 std::size_t maxSamples) {
    WorldgenWindowInspection out{};
    out.face=face;
    out.uBegin=uBegin;
    out.vBegin=vBegin;
    out.width=std::max(0,width);
    out.height=std::max(0,height);
    maxSamples=std::max<std::size_t>(1,maxSamples);
    for(int dv=0;dv<out.height;++dv) {
        for(int du=0;du<out.width;++du) {
            if(out.samples.size()>=maxSamples) {out.truncated=true; return out;}
            const int u=uBegin+du, v=vBegin+dv;
            if(u<0 || v<0 || u>=PlanetSurface::FaceResolution || v>=PlanetSurface::FaceResolution) continue;
            const int radial=planet.surfaceRadial(face,u,v);
            const SurfaceCellAddress address{face,u,v,radial};
            out.samples.push_back({address,planet.get(address)});
        }
    }
    return out;
}

CrossSectionInspection DeveloperObservability::crossSection(const PlanetSurface& planet,
                                                             CubeFace face,int fixedV,int uBegin,int width,
                                                             int radialBegin,int radialEnd,std::size_t maxSamples) {
    CrossSectionInspection out{};
    out.face=face;
    out.fixedV=fixedV;
    out.uBegin=uBegin;
    out.width=std::max(0,width);
    out.radialBegin=std::clamp(std::min(radialBegin,radialEnd),0,PlanetSurface::RadialLayers-1);
    out.radialEnd=std::clamp(std::max(radialBegin,radialEnd),0,PlanetSurface::RadialLayers-1);
    maxSamples=std::max<std::size_t>(1,maxSamples);
    if(fixedV<0 || fixedV>=PlanetSurface::FaceResolution) return out;
    for(int du=0;du<out.width;++du) {
        const int u=uBegin+du;
        if(u<0 || u>=PlanetSurface::FaceResolution) continue;
        for(int radial=out.radialEnd;radial>=out.radialBegin;--radial) {
            if(out.samples.size()>=maxSamples) {out.truncated=true; return out;}
            const SurfaceCellAddress address{face,u,fixedV,radial};
            out.samples.push_back({address,planet.get(address)});
        }
    }
    return out;
}

MeshPacketInspection DeveloperObservability::mesh(const PlanetSurface& planet,
                                                   const PlanetChunkAddress& address,
                                                   bool fieldProxy,int fieldStep) {
    MeshPacketInspection out{};
    out.address=address;
    out.fieldProxy=fieldProxy;
    out.fieldStep=fieldProxy?std::max(1,fieldStep):1;
    const auto snapshot=planet.snapshot();
    const auto packet=fieldProxy
        ? buildPlanetSurfaceFieldChunkMesh(snapshot,address,out.fieldStep,true)
        : buildPlanetSurfaceChunkMesh(snapshot,address);
    out.vertices=packet.vertexCount();
    out.quads=packet.quads;
    out.triangles=packet.triangleCount();
    out.macroQuads=packet.macroQuads;
    out.microQuads=packet.microQuads;
    out.aoDarkenedCorners=packet.aoDarkenedCorners;
    out.materialRanges=packet.materialRanges;
    return out;
}

ChunkRecordInspection DeveloperObservability::chunkRecord(const SurfaceChunkRecord& record) {
    ChunkRecordInspection out{};
    out.planetSlot=record.planetSlot;
    out.planetSeed=record.planetSeed;
    out.generatorVersion=record.generatorVersion;
    out.generatorFingerprint=record.generatorFingerprint;
    out.saveGeneration=record.saveGeneration;
    out.address=record.address;
    out.macroEdits=static_cast<int>(record.journal.macroEdits.size());
    out.placedMarkers=static_cast<int>(record.journal.placedMarkers.size());
    out.refinedCells=static_cast<int>(record.journal.microBricks.size());
    out.microOverrides=record.journal.microOverrideCount();
    for(const auto& v:record.machines) out.machineIds.push_back(v.stableId);
    for(const auto& v:record.portals) out.portalIds.push_back(v.stableId);
    for(const auto& v:record.airlocks) out.airlockIds.push_back(v.stableId);
    for(const auto& v:record.automationRules) out.automationRuleIds.push_back(v.stableId);
    out.machineIds=sortedIds(out.machineIds);
    out.portalIds=sortedIds(out.portalIds);
    out.airlockIds=sortedIds(out.airlockIds);
    out.automationRuleIds=sortedIds(out.automationRuleIds);
    return out;
}

FramePhaseTelemetryInspection DeveloperObservability::framePhases(const FramePhaseTelemetrySnapshot& snapshot) {
    FramePhaseTelemetryInspection out{};
    out.snapshot=snapshot;
    out.simulationMs=snapshot.snapshotMs+snapshot.senseMs+snapshot.planMs+snapshot.resolveMs+snapshot.commitMs+snapshot.persistMs;
    out.accountedMs=out.simulationMs+snapshot.presentMs;
    return out;
}

std::string DeveloperObservability::toText(const WhyInspection& report) {
    std::ostringstream out;
    out<<"WHY "<<stableObjectKindName(report.subject.kind)<<" StableId="<<report.subject.stableId;
    if(report.subject.loaded) out<<" "<<report.subject.label<<" @ "<<surfaceAddressText(report.subject.ownerAddress);
    out<<'\n';
    auto emit=[&](const char* label,const std::vector<std::string>& values){
        if(values.empty()) return;
        out<<label<<":\n";
        for(const auto& value:values) out<<"  - "<<value<<'\n';
    };
    emit("state",report.currentState); emit("inputs",report.relevantInputs); emit("decisions",report.lastDecisions);
    emit("blockers",report.blockers); emit("dependencies",report.dependencies); emit("remediation",report.remediation);
    return out.str();
}

std::string DeveloperObservability::toText(const PowerGraphInspection& report) {
    std::ostringstream out;
    const auto& s=report.authoritativeSummary;
    out<<"POWER networks="<<s.networkCount<<" generation="<<floatText(s.generation)<<" demand="<<floatText(s.demand)
       <<" supplied="<<floatText(s.supplied)<<" shedLoads="<<s.shedLoads<<'\n';
    for(const auto& network:report.networks) {
        out<<"  network "<<network.networkId<<" generation="<<floatText(network.generation)<<" demand="<<floatText(network.demand)
           <<" supplied="<<floatText(network.supplied)<<" battery="<<floatText(network.storedEnergy)<<"/"<<floatText(network.storageCapacity)
           <<(network.brownout?" BROWNOUT":"")<<'\n';
        for(const auto& node:network.nodes)
            out<<"    "<<node.stableId<<" "<<machineName(node.type)<<" P"<<node.priority<<" demand="<<floatText(node.demand)
               <<" powered="<<boolText(node.powered)<<'\n';
    }
    return out.str();
}

std::string DeveloperObservability::toText(const LogisticsGraphInspection& report) {
    std::ostringstream out; out<<"LOGISTICS edges="<<report.edges.size()<<'\n';
    for(const auto& edge:report.edges)
        out<<"  "<<edge.transportStableId<<" "<<machineName(edge.transportType)<<" "<<edge.sourceStableId<<" -> "<<edge.targetStableId
           <<" : "<<edge.bottleneck<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const PathInspection& report) {
    std::ostringstream out;
    out<<"PATH revision="<<report.worldRevision<<" cache="<<report.cacheRevisionBefore<<"->"<<report.cacheRevisionAfter
       <<" found="<<boolText(report.found)<<" cached="<<boolText(report.fromCache)<<" expanded="<<report.expanded<<"/"<<report.maxExpanded<<'\n';
    out<<"  cause: "<<report.cause<<'\n';
    for(const auto& a:report.routeColumns) out<<"  - "<<surfaceAddressText(a)<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const PersistenceDiffInspection& report) {
    std::ostringstream out;
    out<<"PERSISTENCE DIFF chunk="<<chunkAddressText(report.address)<<" worldRevision="<<report.worldRevision
       <<" macro="<<report.macroDeltas.size()<<" refined="<<report.refinedCells<<" microOverrides="<<report.microOverrides<<'\n';
    for(const auto& delta:report.macroDeltas)
        out<<"  "<<surfaceAddressText(delta.address)<<" baseline="<<blockProperties(delta.baseline).name<<" current="<<blockProperties(delta.current).name
           <<(delta.playerPlaced?" playerPlaced":"")<<'\n';
    if(!report.machineIds.empty()) { out<<"  machines:"; for(auto id:report.machineIds) out<<' '<<id; out<<'\n'; }
    if(!report.portalIds.empty()) { out<<"  portals:"; for(auto id:report.portalIds) out<<' '<<id; out<<'\n'; }
    return out.str();
}

std::string DeveloperObservability::toText(const SystemTraceRecorder& recorder) {
    std::ostringstream out;
    const auto events=recorder.snapshot();
    out<<"SYSTEM TRACE events="<<events.size()<<" dropped="<<recorder.dropped()<<'\n';
    for(const auto& recorded:events) {
        const auto& e=recorded.event;
        out<<"  #"<<recorded.sequence<<" "<<(e.system?e.system:"?")<<"/"<<diagnosticPhaseName(e.phase)<<"/"<<diagnosticAccessName(e.access);
        if(e.stableId) out<<" StableId="<<e.stableId;
        if(!e.subject.empty()) out<<" ["<<e.subject<<"]";
        if(!e.detail.empty()) out<<" "<<e.detail;
        out<<'\n';
    }
    return out.str();
}


std::string DeveloperObservability::toText(const EntityInspection& report) {
    std::ostringstream out;
    out<<"ENTITY StableId="<<report.stableId<<" kind="<<report.kind<<" shard="<<report.shard
       <<" loaded="<<boolText(report.loaded)<<" remote="<<boolText(report.remote)<<'\n'
       <<"  "<<report.lookupNote<<'\n';
    for(const auto& component:report.components) out<<"  "<<component.component<<" = "<<component.value<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const GraphicsTelemetryInspection& report) {
    const auto& s=report.snapshot;
    std::ostringstream out;
    out<<"GRAPHICS gpuBytes="<<s.residentGpuBytes<<" uploadedBytes="<<s.uploadedBytesThisFrame
       <<" drawCalls="<<s.drawCalls<<" meshUploads="<<s.meshUploads<<" meshDestroys="<<s.meshDestroys
       <<" triangles="<<s.triangles<<'\n';
    for(const auto& warning:s.backendWarnings) out<<"  warning: "<<warning<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const CitizenInspection& report) {
    std::ostringstream out;
    out<<"CITIZEN StableId="<<report.stableId<<" loaded="<<boolText(report.loaded)<<" remote="<<boolText(report.remote)<<"\n  "<<report.summary<<'\n';
    for(const auto& reason:report.attentionReasons) out<<"  - "<<reason<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const RelationshipGraphInspection& report) {
    std::ostringstream out;
    out<<"RELATIONSHIPS focus="<<report.focusStableId<<" edges="<<report.edges.size()<<'\n';
    for(const auto& edge:report.edges)
        out<<"  "<<edge.sourceStableId<<" -> "<<edge.targetStableId<<" "<<edge.type
           <<" affinity="<<floatText(edge.affinity)<<" trust="<<floatText(edge.trust)
           <<" respect="<<floatText(edge.respect)<<" fear="<<floatText(edge.fear)
           <<(edge.grievance.empty()?"":" grievance="+edge.grievance)<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const EnvironmentalVolumeInspection& report) {
    std::ostringstream out;
    out<<"ENVIRONMENT StableId="<<report.stableId<<"\n  "<<report.summary<<'\n';
    for(const auto& hazard:report.hazards) out<<"  - "<<hazard<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const MedicalInspection& report) {
    std::ostringstream out;
    out<<"MEDICAL StableId="<<report.citizenStableId<<" urgent="<<boolText(report.urgent)<<'\n';
    for(const auto& blocker:report.blockers) out<<"  - "<<blocker<<'\n';
    for(const auto& treatment:report.treatmentPlan) out<<"  treatment: "<<treatment<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const MilitaryReadinessInspection& report) {
    std::ostringstream out;
    out<<"MILITARY squad="<<report.squadStableId<<" readiness="<<floatText(report.readiness*100.0f,1)<<"%\n";
    for(const auto& blocker:report.blockers) out<<"  - "<<blocker<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const ChronicleInspection& report) {
    std::ostringstream out;
    out<<"CHRONICLE focus="<<report.focusStableId<<" events="<<report.events.size()<<'\n';
    for(const auto& event:report.events)
        out<<"  ["<<event.timeKey<<"] #"<<event.eventStableId<<" "<<event.type<<" site="<<event.siteStableId<<" "<<event.summary<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const WorldgenWindowInspection& report) {
    std::ostringstream out;
    out<<"WORLDGEN WINDOW "<<toString(report.face)<<" u="<<report.uBegin<<" v="<<report.vBegin<<" "<<report.width<<"x"<<report.height
       <<" samples="<<report.samples.size()<<(report.truncated?" TRUNCATED":"")<<'\n';
    for(const auto& sample:report.samples)
        out<<"  "<<surfaceAddressText(sample.surface)<<" "<<blockProperties(sample.material).name<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const CrossSectionInspection& report) {
    std::ostringstream out;
    out<<"CROSS SECTION "<<toString(report.face)<<" v="<<report.fixedV<<" u="<<report.uBegin<<".."<<(report.uBegin+report.width-1)
       <<" radial="<<report.radialBegin<<".."<<report.radialEnd<<" samples="<<report.samples.size()<<(report.truncated?" TRUNCATED":"")<<'\n';
    for(const auto& sample:report.samples)
        out<<"  "<<surfaceAddressText(sample.address)<<" "<<blockProperties(sample.material).name<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const MeshPacketInspection& report) {
    std::ostringstream out;
    out<<"MESH "<<chunkAddressText(report.address)<<" mode="<<(report.fieldProxy?"field":"full")<<" step="<<report.fieldStep
       <<" vertices="<<report.vertices<<" quads="<<report.quads<<" triangles="<<report.triangles
       <<" macro="<<report.macroQuads<<" micro="<<report.microQuads<<" aoDark="<<report.aoDarkenedCorners<<'\n';
    for(const auto& range:report.materialRanges)
        out<<"  "<<blockProperties(range.material).name<<" firstVertex="<<range.firstVertex<<" vertexCount="<<range.vertexCount<<" quads="<<range.quads<<'\n';
    return out.str();
}

std::string DeveloperObservability::toText(const ChunkRecordInspection& report) {
    std::ostringstream out;
    out<<"CHUNK RECORD format="<<report.recordFormatVersion<<" planetSlot="<<report.planetSlot<<" seed="<<report.planetSeed
       <<" generator="<<report.generatorVersion<<" fingerprint="<<report.generatorFingerprint<<" saveGeneration="<<report.saveGeneration
       <<" chunk="<<chunkAddressText(report.address)<<'\n';
    out<<"  macroEdits="<<report.macroEdits<<" placed="<<report.placedMarkers<<" refined="<<report.refinedCells<<" microOverrides="<<report.microOverrides<<'\n';
    auto ids=[&](const char* label,const std::vector<std::uint64_t>& values){if(values.empty()) return; out<<"  "<<label<<":"; for(auto id:values) out<<' '<<id; out<<'\n';};
    ids("machines",report.machineIds); ids("portals",report.portalIds); ids("airlocks",report.airlockIds); ids("automation",report.automationRuleIds);
    return out.str();
}

std::string DeveloperObservability::toText(const FramePhaseTelemetryInspection& report) {
    const auto& s=report.snapshot;
    std::ostringstream out;
    out<<"FRAME PHASES frame="<<floatText(static_cast<float>(s.frameMs),2)<<"ms"
       <<" simulation="<<floatText(static_cast<float>(report.simulationMs),2)<<"ms"
       <<" accounted="<<floatText(static_cast<float>(report.accountedMs),2)<<"ms"
       <<" ecsEntities="<<s.ecsEntities<<" persistentCommands="<<s.persistentCommands<<'\n';
    out<<"  snapshot="<<floatText(static_cast<float>(s.snapshotMs),2)<<" sense="<<floatText(static_cast<float>(s.senseMs),2)
       <<" plan="<<floatText(static_cast<float>(s.planMs),2)<<" resolve="<<floatText(static_cast<float>(s.resolveMs),2)
       <<" commit="<<floatText(static_cast<float>(s.commitMs),2)<<" persist="<<floatText(static_cast<float>(s.persistMs),2)
       <<" present="<<floatText(static_cast<float>(s.presentMs),2)<<'\n';
    if(s.frameMs>0.0 && report.accountedMs>s.frameMs+0.01)
        out<<"  warning: accounted phase time exceeds frame time; overlapping/parallel timing or instrumentation mismatch should be inspected\n";
    return out.str();
}

} // namespace elysium
