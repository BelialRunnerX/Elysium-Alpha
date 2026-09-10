// Intended function: Represent atomic ownership transfer intents for items, cargo, structures, vehicles, artifacts, and trade settlements.
#include "OwnershipTransfer.hpp"
namespace elysium::economy {
std::uint64_t OwnershipTransferStateTable::keyOf(const OwnershipTransferState& v) noexcept { return static_cast<std::uint64_t>(v.transferId); }
bool OwnershipTransferStateTable::set(OwnershipTransferState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const OwnershipTransferState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool OwnershipTransferStateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const OwnershipTransferState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const OwnershipTransferState* OwnershipTransferStateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const OwnershipTransferState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
