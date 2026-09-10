// Intended function: Compose deterministic POI kits from entrance/core/support/loot/hazard/story modules with stable generated object IDs.
#include "PoiGrammar.hpp"
namespace elysium::procedural {
std::uint64_t PoiGrammarStateIndex::keyOf(const PoiGrammarState& v) noexcept { return static_cast<std::uint64_t>(v.grammarId); }
bool PoiGrammarStateIndex::upsert(PoiGrammarState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PoiGrammarState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool PoiGrammarStateIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PoiGrammarState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const PoiGrammarState* PoiGrammarStateIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PoiGrammarState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
