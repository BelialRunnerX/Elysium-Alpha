// Intended function: Track recoverable objects, claims, hazards, extraction, and delivery requirements.
#include "mission/RecoveryMission.hpp"
#include <algorithm>
#include <cmath>
#include <utility>
namespace elysium::mission {
bool RecoveryMissionSystem::submit(const RecoveryMissionRequest& q) { if(q.id==0||!std::isfinite(q.value)||!std::isfinite(q.rate)) return false; auto* r=mutableFind(q.id); if(!r){records_.push_back({});r=&records_.back();r->id=q.id;} r->revision=revision_++;r->owner=q.owner;r->target=q.target;r->tick=q.tick;r->value=q.value;r->total+=q.value;r->pressure=std::clamp(r->pressure*.8+std::abs(q.value)*.2,0.0,1.0e15);r->kind=q.kind;r->flags=q.flags;r->active=(q.flags&0x80000000u)==0u;if(q.flags&1u)notice(*r,q.value,q.kind);return true; }
bool RecoveryMissionSystem::remove(std::uint64_t id){auto i=std::find_if(records_.begin(),records_.end(),[&](auto&v){return v.id==id;});if(i==records_.end())return false;records_.erase(i);++revision_;return true;}
void RecoveryMissionSystem::step(std::uint64_t tick,double dt){if(!(dt>0.0)||!std::isfinite(dt))return;for(auto&r:records_){if(!r.active)continue;auto before=r.total;r.total+=r.value*dt;r.pressure=std::max(0.0,r.pressure*std::exp(-.03*dt));r.tick=tick;r.revision=revision_++;if(std::floor(std::abs(before)/5000.0)!=std::floor(std::abs(r.total)/5000.0))notice(r,r.total,r.kind);}}
const RecoveryMissionRecord* RecoveryMissionSystem::find(std::uint64_t id)const{auto i=std::find_if(records_.begin(),records_.end(),[&](auto&v){return v.id==id;});return i==records_.end()?nullptr:&*i;}
RecoveryMissionRecord* RecoveryMissionSystem::mutableFind(std::uint64_t id){auto i=std::find_if(records_.begin(),records_.end(),[&](auto&v){return v.id==id;});return i==records_.end()?nullptr:&*i;}
std::vector<RecoveryMissionRecord> RecoveryMissionSystem::snapshot()const{auto out=records_;std::sort(out.begin(),out.end(),[](auto&a,auto&b){return a.owner==b.owner?a.id<b.id:a.owner<b.owner;});return out;}
std::vector<RecoveryMissionNotice> RecoveryMissionSystem::drainNotices(){std::sort(notices_.begin(),notices_.end(),[](auto&a,auto&b){return a.tick==b.tick?a.sequence<b.sequence:a.tick<b.tick;});auto out=std::move(notices_);notices_.clear();return out;}
void RecoveryMissionSystem::clear(){records_.clear();notices_.clear();++revision_;}
void RecoveryMissionSystem::notice(const RecoveryMissionRecord&r,double v,std::uint32_t k){notices_.push_back({sequence_++,r.id,r.target,r.tick,v,k});}
}
