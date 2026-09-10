// Intended function: Generate procedural Rift Horror body plans, abilities, vulnerabilities, phase rules, tells, and stable encounter identity.
#include "RiftHorrorGrammar.hpp"
namespace elysium::rift {
std::uint64_t RiftHorrorDescriptorStore::idOf(const RiftHorrorDescriptor& v) noexcept { return static_cast<std::uint64_t>(v.horrorId); }
bool RiftHorrorDescriptorStore::put(RiftHorrorDescriptor v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const RiftHorrorDescriptor& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool RiftHorrorDescriptorStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const RiftHorrorDescriptor& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const RiftHorrorDescriptor* RiftHorrorDescriptorStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const RiftHorrorDescriptor& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::rift
