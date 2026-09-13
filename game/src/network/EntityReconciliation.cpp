// Intended function: Track predicted/authoritative stable entity revisions, correction requests, rollback windows, and interpolation state.
#include "EntityReconciliation.hpp"
namespace elysium::network {
std::uint64_t ReconciliationStateCollection::idOf(const ReconciliationState& v) noexcept { return static_cast<std::uint64_t>(v.stableId); }
bool ReconciliationStateCollection::store(ReconciliationState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ReconciliationState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ReconciliationStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ReconciliationState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const ReconciliationState* ReconciliationStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ReconciliationState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
