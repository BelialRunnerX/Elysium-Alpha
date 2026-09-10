// Intended function: Format generated/authored person/site/ship/artifact/faction names with titles, ranks, culture rules, and grammatical context.
#include "NameFormatting.hpp"
namespace elysium::localization {
std::uint64_t NameFormatStateCollection::idOf(const NameFormatState& v) noexcept { return static_cast<std::uint64_t>(v.formatId); }
bool NameFormatStateCollection::store(NameFormatState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const NameFormatState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool NameFormatStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const NameFormatState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const NameFormatState* NameFormatStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const NameFormatState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
