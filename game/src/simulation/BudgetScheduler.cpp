// Intended function: Allocate bounded per-tick budgets across AI, jobs, logistics, environment, history, saves, and presentation staging.
#include "BudgetScheduler.hpp"
namespace elysium::simulation {
std::uint64_t BudgetSliceTable::keyOf(const BudgetSlice& v) noexcept { return static_cast<std::uint64_t>(v.sliceId); }
bool BudgetSliceTable::set(BudgetSlice v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const BudgetSlice& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool BudgetSliceTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const BudgetSlice& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const BudgetSlice* BudgetSliceTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const BudgetSlice& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
