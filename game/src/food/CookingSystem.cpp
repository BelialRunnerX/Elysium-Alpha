// Intended function: Represent kitchen recipes, ingredient substitutions, cook skill, meal quality, batch size, and contamination handling.
#include "CookingSystem.hpp"
namespace elysium::food {
std::uint64_t CookingJobStore::idOf(const CookingJob& v) noexcept { return static_cast<std::uint64_t>(v.jobId); }
bool CookingJobStore::put(CookingJob v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const CookingJob& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool CookingJobStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const CookingJob& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const CookingJob* CookingJobStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const CookingJob& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::food
