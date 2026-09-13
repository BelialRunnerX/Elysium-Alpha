// Intended function: Plan atomic source/destination/ownership container transfers with capacity, filter, reservation, hazard, and rollback checks.
#include "TransferPlanner.hpp"
namespace elysium::inventory {
std::uint64_t TransferPlanIndex::keyOf(const TransferPlan& v) noexcept { return static_cast<std::uint64_t>(v.planId); }
bool TransferPlanIndex::upsert(TransferPlan v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TransferPlan& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool TransferPlanIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TransferPlan& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const TransferPlan* TransferPlanIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TransferPlan& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
