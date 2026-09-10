#include "construction/BlueprintStaging.hpp"
#include "core/Saturating.hpp"
#include <algorithm>

namespace elysium::construction {
namespace { constexpr std::uint64_t ProviderBlueprint = 0x424c55455052ULL; }

bool BlueprintStagingService::publish(BlueprintDefinition d, reason::ReasonStack* reasons) {
    reason::ReasonStack local;
    if (d.blueprintId == 0 || d.components.empty()) local.add({reason::ReasonCode::InvalidRequest,ProviderBlueprint,d.blueprintId,0});
    for (const auto& c : d.components) if (c.pieceFamilyId == 0) local.add({reason::ReasonCode::UnknownContent,ProviderBlueprint,d.blueprintId,10});
    for (const auto& c : d.costs) if (c.materialId == 0 || c.amount == 0) local.add({reason::ReasonCode::UnknownContent,ProviderBlueprint,d.blueprintId,10});
    std::sort(d.components.begin(),d.components.end(),[](const auto&a,const auto&b){return a.buildOrder<b.buildOrder;});
    if (reasons) reasons->append(local); if(local.blocked()) return false;
    return definitions_.emplace(d.blueprintId,std::move(d)).second;
}
const BlueprintDefinition* BlueprintStagingService::find(ContentId id) const { auto it=definitions_.find(id);return it==definitions_.end()?nullptr:&it->second; }
BlueprintCreatePlan BlueprintStagingService::create(std::uint64_t tx, StableId iid, ContentId bid, StableId owner, std::uint64_t rid) const {
    BlueprintCreatePlan out{};
    if(tx==0||iid==0||owner==0||rid==0){out.reasons.add({reason::ReasonCode::InvalidRequest,ProviderBlueprint,iid,0});return out;}
    if(transactions_.contains(tx)||instances_.contains(iid)){out.reasons.add({reason::ReasonCode::DuplicateTransaction,ProviderBlueprint,iid,5});return out;}
    const auto*d=find(bid);if(!d){out.reasons.add({reason::ReasonCode::UnknownContent,ProviderBlueprint,bid,10});return out;}
    out.instance={iid,bid,owner,rid,1,{}};out.instance.components.reserve(d->components.size());
    for(std::uint32_t i=0;i<d->components.size();++i)out.instance.components.push_back({i,BuildStage::Ghost,0.0});
    out.reservation={rid,owner,d->costs};out.accepted=true;return out;
}
bool BlueprintStagingService::commitCreate(std::uint64_t tx, BlueprintInstance value){if(tx==0||value.instanceId==0||transactions_.contains(tx)||instances_.contains(value.instanceId))return false;instances_.emplace(value.instanceId,std::move(value));transactions_.insert(tx);return true;}
bool BlueprintStagingService::applyProgress(std::uint64_t tx, StableId iid, std::uint32_t ci, BuildStage stage, double progress){
    if(tx==0||transactions_.contains(tx))return false;auto it=instances_.find(iid);if(it==instances_.end()||ci>=it->second.components.size())return false;
    auto& c=it->second.components[ci];if(static_cast<unsigned>(stage)<static_cast<unsigned>(c.stage))return false;
    c.stage=stage;c.progress01=safe::finiteClamp(progress,0.0,1.0);it->second.revision=safe::saturatingIncrement(it->second.revision);transactions_.insert(tx);return true;
}
const BlueprintInstance* BlueprintStagingService::instance(StableId id) const {auto it=instances_.find(id);return it==instances_.end()?nullptr:&it->second;}

} // namespace elysium::construction
