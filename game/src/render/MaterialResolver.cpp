// Intended function: Resolve voxel/item/structure material identity into renderer-neutral surface parameters and texture/material handles.
#include "MaterialResolver.hpp"
namespace elysium::render {
std::uint64_t MaterialRenderStateRegistry::key(const MaterialRenderState& r) noexcept { return static_cast<std::uint64_t>(r.materialId); }
bool MaterialRenderStateRegistry::publish(MaterialRenderState r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MaterialRenderState& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool MaterialRenderStateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MaterialRenderState& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const MaterialRenderState* MaterialRenderStateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MaterialRenderState& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::render
