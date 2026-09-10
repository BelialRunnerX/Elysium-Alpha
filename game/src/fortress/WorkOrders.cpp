// Intended function: Track repeating/conditional work orders, quotas, input availability, target stock, and production priority.
#include "WorkOrders.hpp"
namespace elysium::fortress {
std::uint64_t WorkOrderStateStore::idOf(const WorkOrderState& v) noexcept { return static_cast<std::uint64_t>(v.orderId); }
bool WorkOrderStateStore::put(WorkOrderState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const WorkOrderState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool WorkOrderStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const WorkOrderState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const WorkOrderState* WorkOrderStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const WorkOrderState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::fortress
