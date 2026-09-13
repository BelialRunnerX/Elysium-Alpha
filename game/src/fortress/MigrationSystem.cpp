// Intended function: Evaluate migration waves from safety, wealth, housing, jobs, culture, faction relations, and historical events.
#include "MigrationSystem.hpp"
namespace elysium::fortress {
std::uint64_t MigrationWaveStore::idOf(const MigrationWave& v) noexcept { return static_cast<std::uint64_t>(v.waveId); }
bool MigrationWaveStore::put(MigrationWave v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const MigrationWave& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool MigrationWaveStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const MigrationWave& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const MigrationWave* MigrationWaveStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const MigrationWave& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::fortress
