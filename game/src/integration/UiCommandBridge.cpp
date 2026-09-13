// Intended function: Translate validated UI command models into owner-system command requests while keeping UI presentation-only.
#include "UiCommandBridge.hpp"
namespace elysium::integration {
std::uint64_t UiCommandDispatchIndex::keyOf(const UiCommandDispatch& v) noexcept { return static_cast<std::uint64_t>(v.dispatchId); }
bool UiCommandDispatchIndex::upsert(UiCommandDispatch v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const UiCommandDispatch& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool UiCommandDispatchIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const UiCommandDispatch& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const UiCommandDispatch* UiCommandDispatchIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const UiCommandDispatch& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
