// Intended function: deterministic machine-definition registry with safe numeric metadata.
#include "content/MachineCatalogue.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
namespace elysium::content{
MachineDefinition MachineCatalogue::sanitize(MachineDefinition d){d.idlePower=elysium::safe::nonNegative(d.idlePower);d.activePower=elysium::safe::nonNegative(d.activePower);for(auto&c:d.capabilities)c.rate=elysium::safe::nonNegative(c.rate);std::sort(d.capabilities.begin(),d.capabilities.end(),[](auto&a,auto&b){return a.capabilityId<b.capabilityId;});d.capabilities.erase(std::unique(d.capabilities.begin(),d.capabilities.end(),[](auto&a,auto&b){return a.capabilityId==b.capabilityId;}),d.capabilities.end());return d;}
bool MachineCatalogue::publish(MachineDefinition d){if(!d.machineId)return false;d=sanitize(std::move(d));auto i=std::lower_bound(defs_.begin(),defs_.end(),d.machineId,[](auto&a,std::uint64_t b){return a.machineId<b;});if(i!=defs_.end()&&i->machineId==d.machineId){*i=std::move(d);return true;}defs_.insert(i,std::move(d));return true;}
const MachineDefinition*MachineCatalogue::find(std::uint64_t id)const{auto i=std::lower_bound(defs_.begin(),defs_.end(),id,[](auto&a,std::uint64_t b){return a.machineId<b;});return i!=defs_.end()&&i->machineId==id?&*i:nullptr;}
bool MachineCatalogue::supports(std::uint64_t id,std::uint64_t cap)const{return capabilityRate(id,cap)>0.0;}
double MachineCatalogue::capabilityRate(std::uint64_t id,std::uint64_t cap)const{auto*d=find(id);if(!d)return 0;auto i=std::lower_bound(d->capabilities.begin(),d->capabilities.end(),cap,[](auto&a,std::uint64_t b){return a.capabilityId<b;});return i!=d->capabilities.end()&&i->capabilityId==cap?elysium::safe::nonNegative(i->rate):0;}
}
