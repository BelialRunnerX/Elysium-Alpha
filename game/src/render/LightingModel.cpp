// Intended function: Represent sun, sky, local lights, emissive voxels, shadow budgets, and atmosphere-lighting inputs.
#include "LightingModel.hpp"
namespace elysium::render {
std::uint64_t LightRecordRegistry::key(const LightRecord& r) noexcept { return static_cast<std::uint64_t>(r.lightId); }
bool LightRecordRegistry::publish(LightRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const LightRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool LightRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const LightRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const LightRecord* LightRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const LightRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::render
