// Intended function: bound dormant-factory CPU cost while keeping material production deterministic and budgeted on wake/catch-up.
#include "world/FactorySleepController.hpp"
#include <algorithm>
namespace elysium{
void FactorySleepController::upsert(std::uint64_t id,std::uint32_t m,std::uint64_t t){auto&c=clusters_[id];c.clusterId=id;c.machineCount=m;if(!c.lastFineTick)c.lastFineTick=t;}
void FactorySleepController::noteActivity(std::uint64_t id,FactoryWakeReason r,std::uint64_t t){auto&c=clusters_[id];c.clusterId=id;c.sleeping=false;c.wakeReason=r;c.lastFineTick=t;}
void FactorySleepController::evaluateSleep(std::uint64_t t,std::uint64_t idle){for(auto&[_,c]:clusters_)if(!c.sleeping&&t>=c.lastFineTick+idle){c.sleeping=true;c.wakeReason=FactoryWakeReason::None;c.lastCoarseTick=t;}}
std::vector<FactoryCatchupSlice> FactorySleepController::collectCatchup(double dt,std::uint32_t budget){std::vector<FactoryCatchupSlice> out;if(dt<=0||!budget)return out;std::vector<std::uint64_t>ids;for(auto&[id,_]:clusters_)ids.push_back(id);std::sort(ids.begin(),ids.end());for(auto id:ids){auto&c=clusters_[id];if(!c.sleeping)continue;c.accumulatedSeconds+=dt;const auto want=std::uint32_t(std::min<double>(c.accumulatedSeconds,64.0));if(!want)continue;auto take=std::min(want,budget);out.push_back({id,double(take),take});c.accumulatedSeconds-=take;budget-=take;if(!budget)break;}return out;}
}
