// Intended function: assign available drones to explicit transfer orders by priority and stable IDs, keeping assignment deterministic and bounded.
#include "world/DroneLogistics.hpp"
#include <algorithm>
namespace elysium{
void DroneLogisticsScheduler::upsertPort(DronePort p){ports_[p.portId]=p;}void DroneLogisticsScheduler::upsertDrone(DroneUnit d){drones_[d.droneId]=d;}bool DroneLogisticsScheduler::submit(DroneTransferOrder o){if(!o.orderId||!o.quantity||!ports_.contains(o.sourcePort)||!ports_.contains(o.targetPort))return false;orders_.push_back(o);return true;}
std::vector<DroneMission>DroneLogisticsScheduler::plan(std::size_t max){std::sort(orders_.begin(),orders_.end(),[](auto&a,auto&b){if(a.priority!=b.priority)return a.priority>b.priority;return a.orderId<b.orderId;});std::vector<std::uint64_t>ids;for(auto&[id,d]:drones_)if(!d.busy)ids.push_back(id);std::sort(ids.begin(),ids.end());std::vector<DroneMission>out;std::size_t oi=0;for(auto id:ids){if(out.size()>=max||oi>=orders_.size())break;auto&d=drones_[id];auto&o=orders_[oi++];auto q=std::min(d.capacity,o.quantity);out.push_back({o.orderId,id,o.sourcePort,o.targetPort,o.itemId,q});d.busy=true;if(q<o.quantity)o.quantity-=q;else o.quantity=0;}orders_.erase(std::remove_if(orders_.begin(),orders_.end(),[](auto&o){return o.quantity==0;}),orders_.end());return out;}
void DroneLogisticsScheduler::complete(std::uint64_t id){auto i=drones_.find(id);if(i!=drones_.end())i->second.busy=false;}
}
