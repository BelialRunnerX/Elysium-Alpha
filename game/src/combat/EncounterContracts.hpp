#pragma once
#include "combat/EnemyCatalog.hpp"
#include <map>
#include <set>
#include <vector>
namespace elysium::combat {
enum EncounterRead : std::uint32_t { Swarm=1, Pressure=2, Ambush=4, Hover=8, Armor=16, Support=32 };
enum class SupportVerb : std::uint8_t { Buff, Repair, Enable };
struct ArchetypeContract {
    std::uint64_t id{}, silhouette{}; EnemyArchetype profile{};
    EnemyFaction faction{}; std::uint32_t reads{};
    std::vector<std::uint64_t> movementVerbs, defenseVerbs, supportVerbs, abilities;
    std::uint32_t encounterCost{1};
};
struct SupportLink {
    std::uint64_t id{}, source{}, target{}, effect{};
    SupportVerb verb{}; float strength{};
};
struct SupportNotice { SupportLink link; bool active{}; };
class EncounterSupport {
public:
    bool add(SupportLink);
    void removeActor(std::uint64_t);
    std::vector<SupportLink> forTarget(std::uint64_t) const;
    std::vector<SupportLink> snapshot() const;
    bool restore(const std::vector<SupportLink>&, const std::set<std::uint64_t>& live);
    std::vector<SupportNotice> drain();
private: std::map<std::uint64_t,SupportLink> links_; std::vector<SupportNotice> notices_;
};
struct BossPhaseRule {
    std::uint64_t id{}, silhouette{};
    float enterBelowHealth{1}; std::uint64_t requiredFacts{}, forbiddenFacts{};
    std::uint32_t preferredTargetTags{};
    std::vector<std::uint64_t> abilities;
};
struct BossTransformation {
    std::uint64_t actor{}; std::uint32_t phase{}; std::vector<BossPhaseRule> rules;
    bool validate() const;
    bool advance(float healthFraction, std::uint64_t facts);
    const BossPhaseRule* current() const;
};
class EncounterCatalog {
public:
    bool publish(ArchetypeContract);
    const ArchetypeContract* find(std::uint64_t) const;
    std::vector<std::uint64_t> compose(std::uint64_t seed,std::uint32_t budget,
        std::uint32_t desiredReads,EnemyFaction faction,std::size_t limit=32) const;
private: std::map<std::uint64_t,ArchetypeContract> archetypes_;
};
}
