// Intended function: Project stable character appearance, armor layers, damage, equipment, species/body plan, cosmetics, and animation inputs.
#include "CharacterPresentation.hpp"
namespace elysium::render {
std::uint64_t CharacterRenderStateCollection::idOf(const CharacterRenderState& v) noexcept { return static_cast<std::uint64_t>(v.stableId); }
bool CharacterRenderStateCollection::store(CharacterRenderState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CharacterRenderState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool CharacterRenderStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CharacterRenderState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const CharacterRenderState* CharacterRenderStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CharacterRenderState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
