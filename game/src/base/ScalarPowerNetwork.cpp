#include "base/ScalarPowerNetwork.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
namespace elysium::base {
PowerAllocationPlan ScalarPowerNetwork::allocate(std::span<const PowerNodeSnapshot> input,double dt)const{
 PowerAllocationPlan out{};dt=safe::nonNegative(dt);std::vector<PowerNodeSnapshot> nodes(input.begin(),input.end());
 std::stable_sort(nodes.begin(),nodes.end(),[](const auto&a,const auto&b){if(a.priority!=b.priority)return a.priority<b.priority;return a.nodeId<b.nodeId;});
 double available=0;for(const auto&n:nodes){if(n.isolated||!n.enabled)continue;out.generation=safe::nonNegative(out.generation+safe::nonNegative(n.generation));out.demand=safe::nonNegative(out.demand+safe::nonNegative(n.demand));available=safe::nonNegative(available+safe::nonNegative(n.generation)*dt);}
 // Storage discharge is deterministic by priority/stable id and rate-limited.
 for(const auto&n:nodes){if(n.isolated||!n.enabled)continue;const double possible=std::min({safe::nonNegative(n.storedEnergy),safe::nonNegative(n.maxDischargeRate)*dt});available=safe::nonNegative(available+possible);}
 for(const auto&n:nodes){PowerNodeAllocation a{n.nodeId};if(n.isolated||!n.enabled){out.nodes.push_back(a);continue;}const double need=safe::nonNegative(n.demand)*dt;const double servedEnergy=std::min(available,need);available=safe::nonNegative(available-servedEnergy);a.served=dt>0?servedEnergy/dt:0;a.powered=need==0||servedEnergy>=need;a.shed=need>servedEnergy;out.served=safe::nonNegative(out.served+a.served);out.nodes.push_back(a);}
 out.unserved=safe::nonNegative(out.demand-out.served);
 // Surplus charging is intentionally a plan only; owning machine state applies deltas.
 for(std::size_t i=0;i<nodes.size()&&available>0;++i){const auto&n=nodes[i];if(n.isolated||!n.enabled||n.storageCapacity<=n.storedEnergy)continue;double room=safe::nonNegative(n.storageCapacity-n.storedEnergy);double charge=std::min({available,room,safe::nonNegative(n.maxChargeRate)*dt});available=safe::nonNegative(available-charge);out.nodes[i].storageDelta=charge;out.storageDelta=safe::nonNegative(out.storageDelta+charge);}
 return out;
}
} // namespace elysium::base
