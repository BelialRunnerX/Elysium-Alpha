// Intended function: Accumulate bounded counters/timers for chunking, meshing, ECS phases, logistics, AI, saves, and rendering budgets.
#include "PerformanceCounters.hpp"
namespace elysium::tools {
std::uint64_t PerformanceSampleRegistry::key(const PerformanceSample& r) noexcept { return static_cast<std::uint64_t>(r.sampleId); }
bool PerformanceSampleRegistry::publish(PerformanceSample r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const PerformanceSample& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool PerformanceSampleRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const PerformanceSample& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const PerformanceSample* PerformanceSampleRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const PerformanceSample& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::tools
