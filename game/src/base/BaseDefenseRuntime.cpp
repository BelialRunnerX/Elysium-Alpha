#include "base/BaseDefenseRuntime.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
#include <unordered_map>
namespace elysium::base {
std::vector<FusedContact>BaseDefenseRuntime::fuse(std::span<const SensorReport>rs)const{std::unordered_map<std::uint64_t,FusedContact>m;for(const auto&r:rs){if(!r.contactId)continue;auto&c=m[r.contactId];c.contactId=r.contactId;c.confidence=std::max(c.confidence,safe::finiteClamp(r.confidence,0.0,1.0));c.strength=std::max(c.strength,safe::nonNegative(r.strength));const double d=safe::nonNegative(r.distance);c.nearestDistance=c.nearestDistance==0?d:std::min(c.nearestDistance,d);c.hostile=c.hostile||r.hostile;}std::vector<FusedContact>o;for(auto&[_,c]:m)o.push_back(c);std::sort(o.begin(),o.end(),[](auto&a,auto&b){return a.contactId<b.contactId;});return o;}
std::vector<DefenseFireRequest>BaseDefenseRuntime::planFire(std::span<const DefenseEmplacement>es,std::span<const FusedContact>cs)const{std::vector<DefenseFireRequest>o;std::vector<DefenseEmplacement>sorted(es.begin(),es.end());std::sort(sorted.begin(),sorted.end(),[](auto&a,auto&b){return a.stableId<b.stableId;});for(const auto&e:sorted){if(!e.enabled||!e.powered||e.ammo==0)continue;const FusedContact*best=nullptr;for(const auto&c:cs)if(c.hostile&&c.nearestDistance<=safe::nonNegative(e.range)&&(!best||c.confidence>best->confidence||(c.confidence==best->confidence&&c.contactId<best->contactId)))best=&c;if(best)o.push_back({e.stableId,best->contactId,1,best->confidence});}return o;}
} // namespace elysium::base
