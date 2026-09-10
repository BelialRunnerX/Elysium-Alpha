// Intended function: Represent renderer-neutral player command intents for construction, logistics, machines, squads, travel, trade, and governance.
#include "CommandModels.hpp"
namespace elysium::ui {
std::uint64_t CommandModelRegistry::key(const CommandModel& r) noexcept { return static_cast<std::uint64_t>(r.commandId); }
bool CommandModelRegistry::publish(CommandModel r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const CommandModel& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool CommandModelRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const CommandModel& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const CommandModel* CommandModelRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const CommandModel& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::ui
