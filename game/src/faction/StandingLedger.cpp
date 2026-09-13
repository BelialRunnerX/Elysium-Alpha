#include "faction/StandingLedger.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
namespace elysium::faction {
std::uint64_t StandingLedger::key(std::uint64_t p,std::uint64_t s){std::uint64_t x=p^(s+0x9e3779b97f4a7c15ULL+(p<<6)+(p>>2));return x?x:1;}
StandingLedger::StandingLedger(StandingTuning t):tuning_(t){if(tuning_.maximum<tuning_.minimum)std::swap(tuning_.maximum,tuning_.minimum);tuning_.favorDecayPerSecond=safe::nonNegative(tuning_.favorDecayPerSecond);tuning_.suspicionDecayPerSecond=safe::nonNegative(tuning_.suspicionDecayPerSecond);}
bool StandingLedger::apply(const StandingSignal&s,double scale){if(!s.transactionId||!s.playerId||transactions_.contains(s.transactionId))return false;auto k=key(s.playerId,s.systemId);auto&v=rows_[k];v.playerId=s.playerId;v.systemId=s.systemId;double d=safe::finiteClamp(s.delta*safe::nonNegative(scale),-safe::PublishedScalarCeiling,safe::PublishedScalarCeiling);if(s.meter==StandingMeter::Favor)v.favor=safe::finiteClamp(v.favor+d,tuning_.minimum,tuning_.maximum);else v.suspicion=safe::finiteClamp(v.suspicion+d,std::max(tuning_.minimum,v.suspicionFloor),tuning_.maximum);v.revision=safe::saturatingIncrement(v.revision);transactions_.insert(s.transactionId);return true;}
void StandingLedger::setClaimFloor(std::uint64_t p,std::uint64_t s,double f){auto&v=rows_[key(p,s)];v.playerId=p;v.systemId=s;v.suspicionFloor=safe::finiteClamp(f,tuning_.minimum,tuning_.maximum);v.suspicion=std::max(v.suspicion,v.suspicionFloor);v.revision=safe::saturatingIncrement(v.revision);}
void StandingLedger::decay(double dt){dt=safe::nonNegative(dt);for(auto&[_,v]:rows_){v.favor=safe::finiteClamp(v.favor-tuning_.favorDecayPerSecond*dt,tuning_.minimum,tuning_.maximum);v.suspicion=safe::finiteClamp(v.suspicion-tuning_.suspicionDecayPerSecond*dt,std::max(tuning_.minimum,v.suspicionFloor),tuning_.maximum);}}
const StandingSnapshot*StandingLedger::find(std::uint64_t p,std::uint64_t s)const{auto it=rows_.find(key(p,s));return it==rows_.end()?nullptr:&it->second;}
std::vector<StandingSnapshot>StandingLedger::snapshot()const{std::vector<StandingSnapshot>o;for(auto&[_,v]:rows_)o.push_back(v);std::sort(o.begin(),o.end(),[](auto&a,auto&b){if(a.playerId!=b.playerId)return a.playerId<b.playerId;return a.systemId<b.systemId;});return o;}
} // namespace elysium::faction
