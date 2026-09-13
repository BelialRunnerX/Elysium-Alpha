// Intended function: Track social/psychological needs, satisfaction, stress contribution, institutions, relationships, and recovery.
#include "SocialNeeds.hpp"
namespace elysium::fortress {
std::uint64_t SocialNeedStateStore::idOf(const SocialNeedState& v) noexcept { return static_cast<std::uint64_t>(v.citizenId); }
bool SocialNeedStateStore::put(SocialNeedState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SocialNeedState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool SocialNeedStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SocialNeedState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const SocialNeedState* SocialNeedStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SocialNeedState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::fortress
