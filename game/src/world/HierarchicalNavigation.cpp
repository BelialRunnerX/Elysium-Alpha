// Intended function: execute bounded deterministic A* requests and reuse routes only while their obstacle revision remains current.
#include "world/HierarchicalNavigation.hpp"
#include "core/Determinism.hpp"
#include <algorithm>
#include <limits>
#include <queue>

namespace elysium {
std::size_t HierarchicalNavigation::CacheHash::operator()(const CacheKey& k) const noexcept {
    return std::size_t(mix64(k.a ^ mix64(k.b) ^ mix64(k.revision) ^ k.caps));
}
void HierarchicalNavigation::setEdges(NavSectorKey from,std::vector<NavSectorEdge> edges){
    std::sort(edges.begin(),edges.end(),[](auto&a,auto&b){ return a.to.address<b.to.address; }); graph_[from]=std::move(edges);
}
void HierarchicalNavigation::setObstacle(NavSectorKey s,NavObstacleClass o){ obstacles_[s]=o; ++obstacleRevision_; cache_.clear(); }
void HierarchicalNavigation::clearObstacle(NavSectorKey s){ if(obstacles_.erase(s)){++obstacleRevision_;cache_.clear();} }
void HierarchicalNavigation::submit(NavRouteRequest r){ pending_.push_back(r); }
void HierarchicalNavigation::cancel(std::uint64_t id){ cancelled_.insert(id); }
std::optional<NavRoutePlan> HierarchicalNavigation::cached(NavSectorKey a,NavSectorKey b,std::uint32_t caps) const{
    CacheKey k{a.address,b.address,obstacleRevision_,caps}; auto it=cache_.find(k); if(it==cache_.end()) return std::nullopt; auto r=it->second; r.fromCache=true; return r;
}
NavRoutePlan HierarchicalNavigation::solve(const NavRouteRequest& r){
    NavRoutePlan out{}; out.requestId=r.requestId; out.obstacleRevision=obstacleRevision_;
    if(cancelled_.contains(r.requestId)){out.cancelled=true;return out;}
    if(auto c=cached(r.start,r.goal,r.capabilityMask)){auto v=*c;v.requestId=r.requestId;return v;}
    struct Node{float g;std::uint64_t id;}; struct Cmp{bool operator()(const Node&a,const Node&b)const{return a.g>b.g||(a.g==b.g&&a.id>b.id);}};
    std::priority_queue<Node,std::vector<Node>,Cmp> q; std::unordered_map<std::uint64_t,float> dist; std::unordered_map<std::uint64_t,std::uint64_t> prev;
    dist[r.start.address]=0.f;q.push({0.f,r.start.address});std::uint32_t expanded=0;
    while(!q.empty()&&expanded<r.maxExpanded){ if(cancelled_.contains(r.requestId)){out.cancelled=true;return out;} auto n=q.top();q.pop(); if(n.g!=dist[n.id])continue; ++expanded; if(n.id==r.goal.address)break;
        auto gi=graph_.find(NavSectorKey{n.id}); if(gi==graph_.end())continue;
        for(const auto&e:gi->second){ if((e.capabilityMask&r.capabilityMask)==0)continue; auto oi=obstacles_.find(e.to); if(oi!=obstacles_.end()&&(oi->second==NavObstacleClass::Solid||oi->second==NavObstacleClass::Restricted))continue; float penalty=oi==obstacles_.end()?0.f:(oi->second==NavObstacleClass::Hazard?5.f:1.f); float ng=n.g+std::max(0.001f,e.cost)+penalty; auto di=dist.find(e.to.address); if(di==dist.end()||ng<di->second){dist[e.to.address]=ng;prev[e.to.address]=n.id;q.push({ng,e.to.address});}}
    }
    auto goal=dist.find(r.goal.address); if(goal==dist.end())return out; out.found=true;out.totalCost=goal->second; std::uint64_t cur=r.goal.address; while(true){out.sectors.push_back({cur});if(cur==r.start.address)break;auto p=prev.find(cur);if(p==prev.end()){out.found=false;out.sectors.clear();return out;}cur=p->second;} std::reverse(out.sectors.begin(),out.sectors.end());
    cache_[{r.start.address,r.goal.address,obstacleRevision_,r.capabilityMask}]=out; return out;
}
std::vector<NavRoutePlan> HierarchicalNavigation::process(std::size_t maxRequests){std::vector<NavRoutePlan> out;while(!pending_.empty()&&out.size()<maxRequests){auto r=pending_.front();pending_.pop_front();out.push_back(solve(r));cancelled_.erase(r.requestId);}return out;}
} // namespace elysium
