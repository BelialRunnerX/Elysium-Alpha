// Intended function: Project settlement population, jobs, stocks, power, atmosphere, food, medicine, security, alerts, trade, and history summaries.
#include "FortressDashboard.hpp"
namespace elysium::ui {
std::uint64_t DashboardSectionTable::keyOf(const DashboardSection& v) noexcept { return static_cast<std::uint64_t>(v.sectionId); }
bool DashboardSectionTable::set(DashboardSection v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DashboardSection& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool DashboardSectionTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DashboardSection& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const DashboardSection* DashboardSectionTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DashboardSection& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
