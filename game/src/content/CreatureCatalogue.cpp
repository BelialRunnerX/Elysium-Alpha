// Intended function: Provide authored creature definitions layered over procedural body plans, factions, abilities, loot, and habitat tags.
#include "CreatureCatalogue.hpp"
namespace elysium::content {
std::uint64_t CreatureRecordRegistry::key(const CreatureRecord& r) noexcept { return static_cast<std::uint64_t>(r.creatureId); }
bool CreatureRecordRegistry::publish(CreatureRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const CreatureRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool CreatureRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const CreatureRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const CreatureRecord* CreatureRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const CreatureRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::content
