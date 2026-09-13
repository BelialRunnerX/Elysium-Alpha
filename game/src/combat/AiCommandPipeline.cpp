#include "combat/AiCommandPipeline.hpp"
#include <algorithm>
#include <cmath>
#include <set>
#include <tuple>
namespace elysium::combat {
namespace {
bool position(const std::array<double,3>& p) {for(auto v:p) if(!std::isfinite(v)) return false;return true;}
const AiEntityView* find(const AiWorldSnapshot& s,AiId id) {
    auto i=std::lower_bound(s.entities.begin(),s.entities.end(),id,[](auto& e,AiId key){return e.id<key;});
    return i==s.entities.end()||i->id!=id?nullptr:&*i;
}
AiRejection validate(const AiWorldSnapshot& s,const AiCandidate& c) {
    if(c.tick!=s.tick) return AiRejection::StaleTick;
    if(!c.actor||!c.sequence||static_cast<unsigned>(c.action)>3||!position(c.destination)) return AiRejection::Invalid;
    auto a=find(s,c.actor);if(!a||!a->alive) return AiRejection::MissingActor;
    if(a->generation!=c.actorGeneration||a->revision!=c.actorRevision) return AiRejection::StaleEntity;
    if(c.target) {
        auto t=find(s,c.target);if(!t||!t->alive) return AiRejection::MissingTarget;
        if(t->generation!=c.targetGeneration||t->revision!=c.targetRevision) return AiRejection::StaleEntity;
    } else if(c.action==AiAction::Attack) return AiRejection::MissingTarget;
    if(c.action==AiAction::Ability&&!c.ability) return AiRejection::Invalid;
    return AiRejection::None;
}
}
bool AiCommandPipeline::canonicalize(AiWorldSnapshot& s) {
    auto next=s.entities;
    std::sort(next.begin(),next.end(),[](auto& a,auto& b){return a.id<b.id;});
    for(std::size_t i=0;i<next.size();++i)
        if(!next[i].id||!next[i].generation||!position(next[i].position)||(i&&next[i-1].id==next[i].id)) return false;
    s.entities.swap(next);return true;
}
std::vector<AiPathRequest> AiCommandPipeline::pathRequests(const AiWorldSnapshot& input,const std::vector<AiCandidate>& candidates) {
    auto s=input;std::vector<AiPathRequest> out;if(!canonicalize(s)) return out;
    std::set<AiId> requests;
    for(auto& c:candidates) if(c.action==AiAction::Move&&c.pathRequest&&validate(s,c)==AiRejection::None) {
        if(!requests.insert(c.pathRequest).second) continue;
        auto a=find(s,c.actor);out.push_back({c.pathRequest,s.tick,c.actor,a->generation,s.navigationRevision,a->position,c.destination});
    }
    std::sort(out.begin(),out.end(),[](auto& a,auto& b){return a.id<b.id;});return out;
}
AiCommitBatch AiCommandPipeline::commit(const AiWorldSnapshot& input,std::vector<AiCandidate> candidates,std::vector<AiPathResult> paths) {
    auto s=input;AiCommitBatch batch;batch.tick=s.tick;
    const bool worldValid=canonicalize(s);
    std::sort(candidates.begin(),candidates.end(),[](auto& a,auto& b){
        if(a.actor!=b.actor)return a.actor<b.actor;if(a.priority!=b.priority)return a.priority>b.priority;
        return std::tie(a.sequence,a.target,a.action,a.ability,a.pathRequest)<std::tie(b.sequence,b.target,b.action,b.ability,b.pathRequest);
    });
    std::map<AiId,std::size_t> pathById;std::set<AiId> ambiguousPaths;
    for(std::size_t i=0;i<paths.size();++i) if(!pathById.emplace(paths[i].request.id,i).second) ambiguousPaths.insert(paths[i].request.id);
    std::set<AiId> claimed;std::map<std::pair<AiId,AiId>,unsigned> sequences;
    for(auto& c:candidates) ++sequences[{c.actor,c.sequence}];
    for(auto& c:candidates) {
        auto reason=worldValid?validate(s,c):AiRejection::Invalid;
        if(sequences[{c.actor,c.sequence}]!=1) reason=AiRejection::Conflict;
        const AiPathResult* path=nullptr;
        if(reason==AiRejection::None&&c.action==AiAction::Move) {
            auto it=pathById.find(c.pathRequest);
            if(it==pathById.end()||ambiguousPaths.count(c.pathRequest)) reason=AiRejection::StalePath;
            else {
                path=&paths[it->second];auto& r=path->request;
                if(!path->reachable||path->waypoints.empty()||r.tick!=s.tick||r.actor!=c.actor||
                   r.generation!=c.actorGeneration||r.navigationRevision!=s.navigationRevision||
                   r.to!=c.destination||r.from!=find(s,c.actor)->position||
                   !std::all_of(path->waypoints.begin(),path->waypoints.end(),position)) reason=AiRejection::StalePath;
            }
        }
        if(reason==AiRejection::None&&!claimed.insert(c.actor).second) reason=AiRejection::Conflict;
        batch.decisions.push_back({c,reason});if(reason==AiRejection::None&&path) batch.routes.push_back(*path);
    }
    return batch;
}
bool AiCommandPipeline::alreadyActed(const AiCandidate& c) const {
    auto it=lastActed_.find(c.actor);return it!=lastActed_.end()&&it->second.first>=c.tick;
}
bool AiCommandPipeline::acknowledge(const AiDecision& d) {
    if(d.rejection!=AiRejection::None||!d.candidate.actor||alreadyActed(d.candidate)) return false;
    lastActed_[d.candidate.actor]={d.candidate.tick,d.candidate.sequence};return true;
}
std::vector<std::array<AiId,3>> AiCommandPipeline::snapshot() const {
    std::vector<std::array<AiId,3>> out;for(auto& [actor,state]:lastActed_) out.push_back({actor,state.first,state.second});return out;
}
bool AiCommandPipeline::restore(const std::vector<std::array<AiId,3>>& records) {
    std::map<AiId,std::pair<AiId,AiId>> next;
    for(auto& r:records) if(!r[0]||!r[2]||!next.emplace(r[0],std::make_pair(r[1],r[2])).second) return false;
    lastActed_.swap(next);return true;
}
}
