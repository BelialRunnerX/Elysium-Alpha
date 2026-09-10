// Intended function: Coordinate stable entity promotion/demotion between strategic summaries and active ECS shards without identity loss.
#include "ShardPromotion.hpp"
namespace elysium::simulation {
std::uint64_t ShardTransferTable::keyOf(const ShardTransfer& v) noexcept { return static_cast<std::uint64_t>(v.transferId); }
bool ShardTransferTable::set(ShardTransfer v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ShardTransfer& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ShardTransferTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ShardTransfer& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const ShardTransfer* ShardTransferTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ShardTransfer& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
