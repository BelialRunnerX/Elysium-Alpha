#pragma once
#include "combat/CombatRules.hpp"
#include <map>
#include <set>
#include <vector>
namespace elysium::combat {
inline constexpr std::uint32_t DamageOrderVersion=1;
struct CombatHealth { std::uint64_t id{}, revision{}; float health{}, maximum{}; };
struct MitigationSource { std::uint64_t content{}, item{}; DamagePipelineStage stage{}; float share{}; };
enum class DamageCommitError { None, DuplicateEvent, UnknownActor, InvalidInput, StaleActor, UnsupportedVersion };
struct FinalDamageRecord {
    std::uint32_t pipelineVersion{DamageOrderVersion};
    DamageResolution resolution;
    float defenderBefore{}, defenderAfter{}, attackerBefore{}, attackerAfter{};
    float actualLanded{}, actualReflected{}, actualHealing{};
    CombatElement attackElement{}, defenseElement{};
    bool elementalAdvantage{};
    std::vector<MitigationSource> sources;
};
struct DamageCommitResult { DamageCommitError error{}; FinalDamageRecord record; };
struct DamageLedgerSnapshot {
    std::uint32_t pipelineVersion{DamageOrderVersion};
    std::vector<CombatHealth> actors;
    std::vector<std::uint64_t> committedEvents;
    std::vector<FinalDamageRecord> unpublished;
};
// A narrow owner-side health store for isolated combat generation. ECS binding
// will replace ownership at integration; damage arithmetic stays in CombatRules.
class DamageCommit {
public:
    bool registerActor(CombatHealth);
    DamageCommitResult apply(const DamageContext&, std::uint64_t attackerRevision,
                            std::uint64_t defenderRevision, std::vector<MitigationSource> sources={},
                            std::uint32_t pipelineVersion=DamageOrderVersion);
    const CombatHealth* find(std::uint64_t id) const;
    DamageLedgerSnapshot snapshot() const;
    bool restore(const DamageLedgerSnapshot&);
    std::vector<FinalDamageRecord> drainEvents();
private:
    std::map<std::uint64_t,CombatHealth> actors_;
    std::set<std::uint64_t> committed_;
    std::vector<FinalDamageRecord> events_;
};
}
