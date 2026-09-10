#include "combat/DamageCommit.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>
namespace elysium::combat {
namespace {
bool finite(float f) { return std::isfinite(f) && f>=0 && f<=1e9f; }
bool validHealth(const CombatHealth& h) {
    return h.id && finite(h.health) && finite(h.maximum) && h.maximum>0 && h.health<=h.maximum;
}
}
bool DamageCommit::registerActor(CombatHealth actor) {
    return validHealth(actor) && actors_.emplace(actor.id,actor).second;
}
const CombatHealth* DamageCommit::find(std::uint64_t id) const {
    auto it=actors_.find(id); return it==actors_.end()?nullptr:&it->second;
}
DamageCommitResult DamageCommit::apply(const DamageContext& c,std::uint64_t ar,std::uint64_t dr,
                                     std::vector<MitigationSource> sources,std::uint32_t version) {
    DamageCommitResult out;
    auto fail=[&](DamageCommitError e) { out.error=e; return out; };
    if (version!=DamageOrderVersion) return fail(DamageCommitError::UnsupportedVersion);
    if (committed_.count(c.eventId)) return fail(DamageCommitError::DuplicateEvent);
    if (!c.eventId || !c.attackerStableId || !c.defenderStableId || !finite(c.baseAmount))
        return fail(DamageCommitError::InvalidInput);
    auto a=actors_.find(c.attackerStableId), d=actors_.find(c.defenderStableId);
    if (a==actors_.end() || d==actors_.end()) return fail(DamageCommitError::UnknownActor);
    if (a->second.revision!=ar || d->second.revision!=dr || ar==UINT64_MAX || dr==UINT64_MAX)
        return fail(DamageCommitError::StaleActor);
    for (const auto& s:sources)
        if (!s.content || !std::isfinite(s.share) || s.share<0 || s.share>1 ||
            static_cast<unsigned>(s.stage)>8) return fail(DamageCommitError::InvalidInput);
    const float scalars[]={c.attack.psionicScale,c.attack.meleeBaseDamage,c.attack.weaponMultiplier,
        c.attack.attackScale,c.attack.criticalChance,c.attack.criticalMultiplier,c.attack.lifestealShare,
        c.defender.dodgeRune,c.defender.dodgeReflex,c.defender.dodgePassive,c.defender.heatReduction,
        c.defender.psionicScale,c.defender.damageReduction,c.defender.defenceScale,c.defender.reflectShare};
    for (float v:scalars) if (!finite(v)) return fail(DamageCommitError::InvalidInput);
    if (static_cast<unsigned>(c.attack.kind)>6 || static_cast<unsigned>(c.attack.element)>5 ||
        static_cast<unsigned>(c.defender.element)>5 || static_cast<unsigned>(c.attack.terrainCategory)>4 ||
        c.attack.weaponTier<0 || c.defender.armorTier<0) return fail(DamageCommitError::InvalidInput);
    auto& r=out.record;
    r.resolution=resolveDamage(c);
    if (!finite(r.resolution.landed) || !finite(r.resolution.reflected) || !finite(r.resolution.lifesteal))
        return fail(DamageCommitError::InvalidInput);
    r.defenderBefore=d->second.health; r.attackerBefore=a->second.health;
    r.actualLanded=std::min(r.defenderBefore,r.resolution.landed);
    r.defenderAfter=r.defenderBefore-r.actualLanded;
    const bool same=c.attackerStableId==c.defenderStableId;
    const float attackerAfterHit=same?r.defenderAfter:r.attackerBefore;
    r.actualReflected=c.reflecting||same?0:std::min(attackerAfterHit,r.resolution.reflected);
    const float afterReflection=attackerAfterHit-r.actualReflected;
    const float healing=c.reflecting||same||afterReflection<=0?0:
        r.actualLanded*std::clamp(c.attack.lifestealShare,0.0f,.95f);
    r.actualHealing=std::min(a->second.maximum-afterReflection,healing);
    r.attackerAfter=afterReflection+r.actualHealing;
    if (same) r.defenderAfter=r.attackerAfter;
    r.attackElement=c.attack.element; r.defenseElement=c.defender.element;
    r.elementalAdvantage=elementBeats(r.attackElement,r.defenseElement);
    std::sort(sources.begin(),sources.end(),[](auto& x,auto& y){return std::tie(x.stage,x.content,x.item)<std::tie(y.stage,y.content,y.item);});
    r.sources=std::move(sources);
    r.resolution.event.landed=r.actualLanded; r.resolution.event.reflected=r.actualReflected;
    r.resolution.event.lifesteal=r.actualHealing;
    auto stagedEvents=events_; stagedEvents.push_back(r);
    auto stagedIds=committed_; stagedIds.insert(c.eventId);
    a->second.health=r.attackerAfter; ++a->second.revision;
    if (!same) { d->second.health=r.defenderAfter; ++d->second.revision; }
    events_.swap(stagedEvents); committed_.swap(stagedIds);
    return out;
}
DamageLedgerSnapshot DamageCommit::snapshot() const {
    DamageLedgerSnapshot s;
    for (const auto& [id,actor]:actors_) s.actors.push_back(actor);
    s.committedEvents.assign(committed_.begin(),committed_.end()); s.unpublished=events_; return s;
}
bool DamageCommit::restore(const DamageLedgerSnapshot& s) {
    if (s.pipelineVersion!=DamageOrderVersion) return false;
    DamageCommit staged;
    for (auto actor:s.actors) if (!staged.registerActor(actor)) return false;
    for (auto id:s.committedEvents) if (!id || !staged.committed_.insert(id).second) return false;
    std::set<std::uint64_t> queued;
    for (const auto& e:s.unpublished)
        if (e.pipelineVersion!=DamageOrderVersion || !staged.committed_.count(e.resolution.event.eventId) ||
            !queued.insert(e.resolution.event.eventId).second || !finite(e.actualLanded) ||
            !finite(e.actualHealing) || !finite(e.actualReflected)) return false;
    staged.events_=s.unpublished;
    actors_.swap(staged.actors_); committed_.swap(staged.committed_); events_.swap(staged.events_); return true;
}
std::vector<FinalDamageRecord> DamageCommit::drainEvents() {
    std::vector<FinalDamageRecord> out; out.swap(events_); return out;
}
}
