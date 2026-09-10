// Intended function: authored Praetor boss phase/ability planner with telegraphed attacks and deterministic stable-ID outputs.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium{
enum class PraetorPhase:std::uint8_t{Arrival,Shielded,Assault,Overload,FinalStand,Defeated};
enum class PraetorAbility:std::uint8_t{None,LanceSweep,GravityWell,AegisPulse,DroneCall,RiftStep,ExecutionArc,OverloadNova};
struct PraetorState{std::uint64_t stableId{};PraetorPhase phase{PraetorPhase::Arrival};float health{1.f},shield{1.f};std::uint64_t nextAbilityTick{},lastAbilityTick{};std::uint32_t sequence{};};
struct PraetorIntent{std::uint64_t stableKey{},bossId{},targetId{};PraetorAbility ability{PraetorAbility::None};std::uint32_t telegraphTicks{};float radius{},magnitude{};bool summon{};};
class PraetorBossPlanner{public:std::vector<PraetorIntent> update(PraetorState&state,std::uint64_t tick,std::uint64_t targetId);void applyDamage(PraetorState&state,float shieldDamage,float healthDamage);};
}
