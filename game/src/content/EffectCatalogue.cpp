// Intended function: Provide stable gameplay-effect definitions for buffs, debuffs, hazards, medical states, abilities, and equipment passives.
#include "EffectCatalogue.hpp"
namespace elysium::content {
std::uint64_t EffectRecordRegistry::key(const EffectRecord& r) noexcept { return static_cast<std::uint64_t>(r.effectId); }
bool EffectRecordRegistry::publish(EffectRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const EffectRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool EffectRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const EffectRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const EffectRecord* EffectRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const EffectRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::content
