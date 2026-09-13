#include "faction/ImperialVisibility.hpp"
#include "core/Saturating.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace elysium::faction {
namespace {
std::uint64_t mix64(std::uint64_t x) noexcept { x^=x>>30;x*=0xbf58476d1ce4e5b9ULL;x^=x>>27;x*=0x94d049bb133111ebULL;return x^(x>>31); }
double distance(const VisibilityPosition&a,const VisibilityPosition&b) noexcept {
    const double dx=a.x-b.x,dy=a.y-b.y,dz=a.z-b.z;
    return safe::nonNegative(std::sqrt(dx*dx+dy*dy+dz*dz));
}
bool finitePosition(const VisibilityPosition&p) noexcept {return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);}
}

bool ImperialVisibilityService::publish(VisibilityInfrastructure v){
    if(!v.stableId||!v.systemId||!v.ownerId||!finitePosition(v.position)||v.revision==0)return false;
    v.sensorRange=safe::nonNegative(v.sensorRange);v.inspectionStrength=safe::nonNegative(v.inspectionStrength);
    v.minimumDeclaredVolume=safe::nonNegative(v.minimumDeclaredVolume);
    if(v.kind==VisibilityInfrastructureKind::InspectionBeacon && v.sensorRange<=0.0)return false;
    if(v.kind==VisibilityInfrastructureKind::RegionalArchive && !v.knowledgeProviderRef)return false;
    return infrastructure_.emplace(v.stableId,v).second;
}

const VisibilityInfrastructure* ImperialVisibilityService::find(std::uint64_t id) const {auto it=infrastructure_.find(id);return it==infrastructure_.end()?nullptr:&it->second;}

std::uint64_t ImperialVisibilityService::receiptKey(std::uint64_t node,std::uint64_t signal) noexcept{return mix64(node^mix64(signal));}
std::uint64_t ImperialVisibilityService::reportId(std::uint64_t node,std::uint64_t signal) noexcept{auto v=mix64(0x4956495352455054ULL^node^mix64(signal));return v?v:1;}

std::vector<VisibilityReport> ImperialVisibilityService::observe(std::span<const DeclaredActivitySignal> signals){
    std::vector<VisibilityReport> out;
    for(const auto&[_,node]:infrastructure_){
        if(node.kind!=VisibilityInfrastructureKind::InspectionBeacon||node.state!=VisibilityInfrastructureState::Active)continue;
        for(const auto&s:signals){
            if(!s.signalId||!s.subjectId||s.systemId!=node.systemId||!finitePosition(s.position)||!std::isfinite(s.declaredVolume)||!std::isfinite(s.signature))continue;
            const double volume=safe::nonNegative(s.declaredVolume);if(volume<node.minimumDeclaredVolume)continue;
            const double d=distance(node.position,s.position);if(d>node.sensorRange)continue;
            const auto key=receiptKey(node.stableId,s.signalId);if(reportedReceipts_.contains(key))continue;
            const double falloff=node.sensorRange>0.0?1.0-safe::finiteClamp(d/node.sensorRange,0.0,1.0):0.0;
            const double visibility=safe::nonNegative(safe::nonNegative(s.signature)*falloff*std::max(1.0,volume));
            VisibilityReport r{};r.reportId=reportId(node.stableId,s.signalId);r.nodeId=node.stableId;r.subjectId=s.subjectId;r.signalId=s.signalId;r.systemId=s.systemId;r.visibility=visibility;r.inspectionPressure=safe::nonNegative(visibility*node.inspectionStrength);r.tick=s.tick;
            out.push_back(r);reportedReceipts_[key]=true;
        }
    }
    std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){if(a.nodeId!=b.nodeId)return a.nodeId<b.nodeId;return a.signalId<b.signalId;});
    return out;
}

InfrastructureResult ImperialVisibilityService::validateTransition(const InfrastructureCommand& c,bool enabling) const{
    auto it=infrastructure_.find(c.stableId);if(it==infrastructure_.end())return{false,InfrastructureFailure::Missing,0};
    const auto&v=it->second;if(!c.commandId||!c.actorId||!std::isfinite(c.interactionDistance)||!std::isfinite(c.maximumInteractionDistance))return{false,InfrastructureFailure::InvalidInput,v.revision};
    if(c.expectedRevision!=v.revision)return{false,InfrastructureFailure::RevisionConflict,v.revision};
    if(v.state==VisibilityInfrastructureState::Destroyed)return{false,InfrastructureFailure::Destroyed,v.revision};
    if(safe::nonNegative(c.interactionDistance)>safe::nonNegative(c.maximumInteractionDistance))return{false,InfrastructureFailure::OutOfRange,v.revision};
    if(!c.dependencyOnline)return{false,InfrastructureFailure::MissingDependency,v.revision};
    if(!c.authorized)return{false,v.protectedInfrastructure?InfrastructureFailure::Protected:InfrastructureFailure::Unauthorized,v.revision};
    if(enabling&&v.state==VisibilityInfrastructureState::Active)return{false,InfrastructureFailure::AlreadyActive,v.revision};
    if(!enabling&&v.state==VisibilityInfrastructureState::Disabled)return{false,InfrastructureFailure::AlreadyDisabled,v.revision};
    return{true,InfrastructureFailure::None,v.revision};
}

InfrastructureResult ImperialVisibilityService::disable(const InfrastructureCommand&c){auto r=validateTransition(c,false);if(!r.accepted)return r;auto&v=infrastructure_.at(c.stableId);v.state=VisibilityInfrastructureState::Disabled;v.revision=safe::saturatingIncrement(v.revision);return{true,InfrastructureFailure::None,v.revision};}
InfrastructureResult ImperialVisibilityService::restore(const InfrastructureCommand&c){auto r=validateTransition(c,true);if(!r.accepted)return r;auto&v=infrastructure_.at(c.stableId);v.state=VisibilityInfrastructureState::Active;v.revision=safe::saturatingIncrement(v.revision);return{true,InfrastructureFailure::None,v.revision};}
InfrastructureResult ImperialVisibilityService::destroy(const InfrastructureCommand&c){auto r=validateTransition(c,false);if(!r.accepted&&r.failure!=InfrastructureFailure::AlreadyDisabled)return r;auto&v=infrastructure_.at(c.stableId);if(!c.authorized)return{false,v.protectedInfrastructure?InfrastructureFailure::Protected:InfrastructureFailure::Unauthorized,v.revision};v.state=VisibilityInfrastructureState::Destroyed;v.revision=safe::saturatingIncrement(v.revision);return{true,InfrastructureFailure::None,v.revision};}

RegionalArchiveView ImperialVisibilityService::archiveView(std::uint64_t id,std::span<const KnowledgeFactView> facts,std::size_t maxFacts) const{
    RegionalArchiveView out{};const auto*archive=find(id);if(!archive||archive->kind!=VisibilityInfrastructureKind::RegionalArchive||archive->state!=VisibilityInfrastructureState::Active)return out;
    out.archiveId=id;out.systemId=archive->systemId;out.infrastructureRevision=archive->revision;
    std::vector<KnowledgeFactView> filtered;filtered.reserve(facts.size());
    for(auto f:facts){if(!f.factId||!std::isfinite(f.confidence))continue;f.confidence=safe::finiteClamp(f.confidence,0.0,1.0);filtered.push_back(f);}
    std::sort(filtered.begin(),filtered.end(),[](const auto&a,const auto&b){if(a.observedTick!=b.observedTick)return a.observedTick>b.observedTick;return a.factId<b.factId;});
    const auto n=std::min(maxFacts,filtered.size());out.facts.assign(filtered.begin(),filtered.begin()+static_cast<std::ptrdiff_t>(n));return out;
}

ImperialVisibilitySnapshot ImperialVisibilityService::snapshot() const{ImperialVisibilitySnapshot s;for(auto&[_,v]:infrastructure_)s.infrastructure.push_back(v);for(auto&[k,_]:reportedReceipts_)s.reportedReceiptKeys.push_back(k);return s;}
bool ImperialVisibilityService::restoreSnapshot(const ImperialVisibilitySnapshot&s){ImperialVisibilityService next;for(auto v:s.infrastructure)if(!next.publish(v))return false;std::set<std::uint64_t>seen;for(auto k:s.reportedReceiptKeys){if(!k||!seen.insert(k).second)return false;next.reportedReceipts_[k]=true;}*this=std::move(next);return true;}

} // namespace elysium::faction
