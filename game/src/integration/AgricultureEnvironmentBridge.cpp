// Intended function: Feed authoritative pressure, oxygen, temperature, water, light, and contamination samples into crop/livestock simulation adapters.
#include "AgricultureEnvironmentBridge.hpp"
namespace elysium::integration {
std::uint64_t AgricultureEnvironmentSampleIndex::keyOf(const AgricultureEnvironmentSample& v) noexcept { return static_cast<std::uint64_t>(v.sampleId); }
bool AgricultureEnvironmentSampleIndex::upsert(AgricultureEnvironmentSample v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AgricultureEnvironmentSample& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool AgricultureEnvironmentSampleIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AgricultureEnvironmentSample& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const AgricultureEnvironmentSample* AgricultureEnvironmentSampleIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AgricultureEnvironmentSample& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
