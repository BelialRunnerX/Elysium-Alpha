#include "combat/EncounterContracts.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
namespace elysium::combat {
namespace {
std::uint64_t mix(std::uint64_t x) { x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31); }
bool validLink(const SupportLink& s) { return s.id&&s.source&&s.target&&s.effect&&s.source!=s.target&&
    static_cast<unsigned>(s.verb)<=2&&std::isfinite(s.strength)&&s.strength>=0&&s.strength<=1e6; }
bool references(const std::vector<std::uint64_t>& ids) {
    std::set<std::uint64_t> seen; for(auto id:ids) if(!id||!seen.insert(id).second) return false; return true;
}
}
bool EncounterSupport::add(SupportLink link) {
    if(!validLink(link)||links_.count(link.id)) return false;
    auto next=notices_;next.push_back({link,true}); links_.emplace(link.id,link);notices_.swap(next);return true;
}
void EncounterSupport::removeActor(std::uint64_t id) {
    auto next=notices_;
    for(auto& [key,link]:links_) if(link.source==id||link.target==id) next.push_back({link,false});
    for(auto it=links_.begin();it!=links_.end();) {
        if(it->second.source==id||it->second.target==id) it=links_.erase(it);else ++it;
    }
    notices_.swap(next);
}
std::vector<SupportLink> EncounterSupport::forTarget(std::uint64_t id) const {
    std::vector<SupportLink> out;for(auto& [key,link]:links_) if(link.target==id) out.push_back(link);return out;
}
std::vector<SupportLink> EncounterSupport::snapshot() const {
    std::vector<SupportLink> out;for(auto& [key,link]:links_) out.push_back(link);return out;
}
bool EncounterSupport::restore(const std::vector<SupportLink>& links,const std::set<std::uint64_t>& live) {
    EncounterSupport staged;
    for(auto link:links) if(!live.count(link.source)||!live.count(link.target)||!staged.add(link)) return false;
    auto notices=notices_;for(auto& [id,link]:links_) notices.push_back({link,false});
    notices.insert(notices.end(),staged.notices_.begin(),staged.notices_.end());
    links_.swap(staged.links_);notices_.swap(notices);return true;
}
std::vector<SupportNotice> EncounterSupport::drain() {std::vector<SupportNotice> out;out.swap(notices_);return out;}
bool BossTransformation::validate() const {
    if(!actor||rules.empty()||phase>=rules.size()) return false;
    float previous=1;std::set<std::uint64_t> ids;
    for(auto& rule:rules) {
        if(!rule.id||!rule.silhouette||!ids.insert(rule.id).second||!std::isfinite(rule.enterBelowHealth)||
           rule.enterBelowHealth<0||rule.enterBelowHealth>previous||
           (rule.requiredFacts&rule.forbiddenFacts)||!references(rule.abilities)) return false;
        previous=rule.enterBelowHealth;
    }
    return true;
}
bool BossTransformation::advance(float health,std::uint64_t facts) {
    if(!validate()||!std::isfinite(health)||health<0||health>1||phase+1>=rules.size()) return false;
    auto& next=rules[phase+1];
    if(health>next.enterBelowHealth||(facts&next.requiredFacts)!=next.requiredFacts||(facts&next.forbiddenFacts)) return false;
    ++phase;return true;
}
const BossPhaseRule* BossTransformation::current() const {return validate()?&rules[phase]:nullptr;}
bool EncounterCatalog::publish(ArchetypeContract d) {
    if(!d.id||!d.silhouette||!d.encounterCost||d.reads>63||static_cast<unsigned>(d.profile)>7||
       static_cast<unsigned>(d.faction)>1||!references(d.movementVerbs)||!references(d.defenseVerbs)||
       !references(d.supportVerbs)||!references(d.abilities)) return false;
    // Authored health/weapon values remain in the existing enemy registry.
    if(enemyProfile(d.profile).faction!=d.faction) return false;
    return archetypes_.emplace(d.id,std::move(d)).second;
}
const ArchetypeContract* EncounterCatalog::find(std::uint64_t id) const {
    auto it=archetypes_.find(id);return it==archetypes_.end()?nullptr:&it->second;
}
std::vector<std::uint64_t> EncounterCatalog::compose(std::uint64_t seed,std::uint32_t budget,
    std::uint32_t desired,EnemyFaction faction,std::size_t limit) const {
    std::vector<std::uint64_t> out;std::uint32_t covered=0;
    for(std::size_t n=0;n<std::min<std::size_t>(limit,256);++n) {
        const ArchetypeContract* best=nullptr;int bestCoverage=-1;std::uint64_t tie=0;
        for(auto& [id,d]:archetypes_) if(d.faction==faction&&d.encounterCost<=budget) {
            int coverage=std::popcount(d.reads&desired&~covered);auto score=mix(seed^mix(id)^mix(n));
            if(coverage>bestCoverage||(coverage==bestCoverage&&score>tie)) {best=&d;bestCoverage=coverage;tie=score;}
        }
        if(!best) break;
        out.push_back(best->id);budget-=best->encounterCost;covered|=best->reads;
    }
    return out;
}
}
