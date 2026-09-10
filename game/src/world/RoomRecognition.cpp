#include "world/RoomRecognition.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
namespace elysium::world {
namespace { constexpr std::uint64_t ProviderRoom=0x524f4f4dULL; }
bool RoomFunctionRegistry::publish(RoomFunctionDefinition d, reason::ReasonStack* reasons){
    reason::ReasonStack local;if(d.functionId==0)local.add({reason::ReasonCode::InvalidRequest,ProviderRoom,d.functionId,0});
    d.minimumFloorArea=safe::nonNegative(d.minimumFloorArea);d.minimumVolume=safe::nonNegative(d.minimumVolume);
    std::sort(d.requiredCapabilities.begin(),d.requiredCapabilities.end());d.requiredCapabilities.erase(std::unique(d.requiredCapabilities.begin(),d.requiredCapabilities.end()),d.requiredCapabilities.end());
    if(reasons)reasons->append(local);if(local.blocked())return false;return definitions_.emplace(d.functionId,std::move(d)).second;
}
const RoomFunctionDefinition* RoomFunctionRegistry::find(ContentId id)const{auto it=definitions_.find(id);return it==definitions_.end()?nullptr:&it->second;}
RoomRecognitionResult RoomFunctionRegistry::recognize(ContentId id,const BoundedVolumeSnapshot& v)const{
    RoomRecognitionResult out{};out.functionId=id;out.sealed=v.terminatedBounded&&!v.reachedSky&&!v.budgetExhausted;
    const auto*d=find(id);if(!d){out.reasons.add({reason::ReasonCode::UnknownContent,ProviderRoom,id,0});return out;}
    if(d->requiresSealed&&!out.sealed)out.reasons.add({reason::ReasonCode::InvalidPlacement,ProviderRoom,v.volumeId,10});
    if(safe::nonNegative(v.floorArea)<d->minimumFloorArea||safe::nonNegative(v.volume)<d->minimumVolume)out.reasons.add({reason::ReasonCode::BudgetExceeded,ProviderRoom,v.volumeId,20});
    for(auto required:d->requiredCapabilities)if(!std::binary_search(v.installedCapabilities.begin(),v.installedCapabilities.end(),required))out.reasons.add({reason::ReasonCode::MissingCapability,ProviderRoom,required,30});
    out.recognized=!out.reasons.blocked();return out;
}
} // namespace elysium::world
