// Intended function: Queue bounded script-originated commands through the same validated stable command path as UI/AI rather than direct world mutation.
#include "ScriptCommandQueue.hpp"
namespace elysium::scripting {
std::uint64_t ScriptCommandCollection::idOf(const ScriptCommand& v) noexcept { return static_cast<std::uint64_t>(v.commandId); }
bool ScriptCommandCollection::store(ScriptCommand v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ScriptCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ScriptCommandCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ScriptCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const ScriptCommand* ScriptCommandCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ScriptCommand& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
