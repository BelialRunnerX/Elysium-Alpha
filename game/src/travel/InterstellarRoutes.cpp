// Intended function: find deterministic feasible jump chains while making danger/Imperial exposure explicit route costs instead of hidden randomness.
#include "travel/InterstellarRoutes.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
namespace elysium{namespace{double dist(const StarRouteNode&a,const StarRouteNode&b){auto x=a.x-b.x,y=a.y-b.y,z=a.z-b.z;return std::sqrt(x*x+y*y+z*z);}}
void InterstellarRoutePlanner::upsert(StarRouteNode n){nodes_[n.systemId]=n;}
InterstellarRoute InterstellarRoutePlanner::find(const InterstellarRouteRequest&r)const{InterstellarRoute o;auto s=nodes_.find(r.from),g=nodes_.find(r.to);if(s==nodes_.end()||g==nodes_.end()||r.jumpRangeLy<=0)return o;struct Q{double c;std::uint32_t id;};struct C{bool operator()(Q a,Q b)const{return a.c>b.c||(a.c==b.c&&a.id>b.id);}};std::priority_queue<Q,std::vector<Q>,C>q;std::unordered_map<std::uint32_t,double>d,raw;std::unordered_map<std::uint32_t,std::uint32_t>prev;d[r.from]=0;raw[r.from]=0;q.push({0,r.from});std::uint32_t ex=0;while(!q.empty()&&ex++<r.maxExpanded){auto n=q.top();q.pop();if(n.c!=d[n.id])continue;if(n.id==r.to)break;auto&a=nodes_.at(n.id);std::vector<std::uint32_t>ids;ids.reserve(nodes_.size());for(auto&[id,_]:nodes_)ids.push_back(id);std::sort(ids.begin(),ids.end());for(auto id:ids){if(id==n.id)continue;auto&b=nodes_.at(id);double dd=dist(a,b);if(dd>r.jumpRangeLy)continue;double nc=n.c+dd+double(b.hazard*r.hazardWeight+b.imperialPressure*r.imperialWeight)*r.jumpRangeLy;double nr=raw[n.id]+dd;if(nr*r.fuelPerLy>r.fuelAvailable)continue;if(!d.contains(id)||nc<d[id]){d[id]=nc;raw[id]=nr;prev[id]=n.id;q.push({nc,id});}}}if(!d.contains(r.to))return o;o.found=true;o.distanceLy=raw[r.to];o.fuelCost=o.distanceLy*r.fuelPerLy;o.weightedCost=d[r.to];for(auto cur=r.to;;cur=prev[cur]){o.systems.push_back(cur);if(cur==r.from)break;}std::reverse(o.systems.begin(),o.systems.end());return o;}
}
