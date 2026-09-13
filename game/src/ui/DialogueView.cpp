// Intended function: Project dialogue speaker, lines, choices, conditions, skill/standing checks, consequences, and conversation history.
#include "DialogueView.hpp"
namespace elysium::ui {
std::uint64_t DialogueViewStateCollection::idOf(const DialogueViewState& v) noexcept { return static_cast<std::uint64_t>(v.viewId); }
bool DialogueViewStateCollection::store(DialogueViewState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DialogueViewState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool DialogueViewStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DialogueViewState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const DialogueViewState* DialogueViewStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DialogueViewState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
