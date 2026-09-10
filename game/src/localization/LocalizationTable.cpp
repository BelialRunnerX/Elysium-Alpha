// Intended function: Store stable localization keys, locale variants, fallbacks, formatting metadata, and mod namespace ownership.
#include "LocalizationTable.hpp"
namespace elysium::localization {
std::uint64_t LocalizedStringCollection::idOf(const LocalizedString& v) noexcept { return static_cast<std::uint64_t>(v.keyId); }
bool LocalizedStringCollection::store(LocalizedString v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const LocalizedString& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool LocalizedStringCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const LocalizedString& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const LocalizedString* LocalizedStringCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const LocalizedString& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
