// Intended function: Track stable account balances, reservations, transfers, fees, rewards, debts, and exactly-once settlement references.
#include "CreditLedger.hpp"
namespace elysium::economy {
std::uint64_t AccountStateTable::keyOf(const AccountState& v) noexcept { return static_cast<std::uint64_t>(v.accountId); }
bool AccountStateTable::set(AccountState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AccountState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool AccountStateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AccountState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const AccountState* AccountStateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const AccountState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
