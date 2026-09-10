// Intended function: imported combat implementation for EnemyCatalog; preserves the agent-authored subsystem contract for later integration/debugging.
#include "combat/EnemyCatalog.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace elysium {
namespace {

constexpr std::array<EnemyProfile,8> kProfiles{{
    // Imperial values preserve the validated v0.20 prototype. Unsworn base
    // tuning is adapted to the same runtime scale while retaining the source
    // family's documented relative role. armorRating preserves the authored
    // bestiary number as data; the Agent-16 damage pipeline does not silently
    // invent a flat-armour formula that the current prototype does not own.
    {EnemyArchetype::Scavenger,EnemyFaction::Unsworn,EnemyRank::Standard,"Scavenger","swarm",30.0f,3.2f,5.0f,2.10f,0.95f,0.0f,0.0f,0.0f,CombatElement::Inert,0.0f,0.0f,TerrainDamageCategory::MarkOnly},
    {EnemyArchetype::Reaver,EnemyFaction::Unsworn,EnemyRank::Elite,"Reaver","pressure",85.0f,2.1f,12.0f,2.65f,1.35f,0.0f,0.0f,4.0f,CombatElement::Inert,0.0f,0.0f,TerrainDamageCategory::FragileMicrodetail},
    {EnemyArchetype::Whisper,EnemyFaction::Unsworn,EnemyRank::Standard,"Whisper","ambush",40.0f,3.4f,14.0f,2.30f,1.15f,0.0f,0.0f,1.0f,CombatElement::Inert,0.0f,0.0f,TerrainDamageCategory::MarkOnly},
    {EnemyArchetype::Drone,EnemyFaction::Empire,EnemyRank::Standard,"Drone","hover",30.0f,3.2f,8.0f,2.35f,1.10f,0.0f,0.0f,6.0f,CombatElement::Inert,0.0f,0.0f,TerrainDamageCategory::MarkOnly},
    {EnemyArchetype::Lictor,EnemyFaction::Empire,EnemyRank::Elite,"Lictor","armor",95.0f,2.1f,14.0f,2.55f,1.35f,0.0f,0.0f,12.0f,CombatElement::Inert,0.0f,0.0f,TerrainDamageCategory::FragileMicrodetail},
    {EnemyArchetype::Adept,EnemyFaction::Empire,EnemyRank::Elite,"Adept","support",48.0f,2.55f,5.0f,2.25f,1.35f,8.0f,7.0f,3.0f,CombatElement::Inert,0.0f,0.0f,TerrainDamageCategory::MarkOnly},
    {EnemyArchetype::Praetor,EnemyFaction::Empire,EnemyRank::Boss,"Praetor of the Sanctioned Answer","command",260.0f,2.35f,20.0f,2.80f,1.55f,0.0f,0.0f,12.0f,CombatElement::Inert,0.0f,0.0f,TerrainDamageCategory::StructuralBreach},
    {EnemyArchetype::Choir,EnemyFaction::Unsworn,EnemyRank::Boss,"Choir of the Uncounted","swarm-command",120.0f,3.2f,9.0f,2.60f,1.40f,0.0f,0.0f,0.0f,CombatElement::Inert,0.0f,0.0f,TerrainDamageCategory::StructuralBreach}
}};

constexpr std::array<EnemyVariantProfile,31> kVariants{{
    {EnemyVariant::Base,EnemyArchetype::Drone,"Base",1.00f,1.00f,1.00f,EnemyAbility::None},
    {EnemyVariant::ScavengerRagpicker,EnemyArchetype::Scavenger,"Ragpicker",1.00f,1.00f,1.00f,EnemyAbility::None},
    {EnemyVariant::ScavengerFeral,EnemyArchetype::Scavenger,"Feral",0.75f,1.45f,0.80f,EnemyAbility::Cornered},
    {EnemyVariant::ScavengerCarrion,EnemyArchetype::Scavenger,"Carrion",1.35f,0.85f,0.80f,EnemyAbility::Knitting},
    {EnemyVariant::ScavengerScuttler,EnemyArchetype::Scavenger,"Scuttler",0.70f,0.95f,1.35f,EnemyAbility::Swift},
    {EnemyVariant::ScavengerBlightfed,EnemyArchetype::Scavenger,"Blightfed",0.95f,1.00f,1.05f,EnemyAbility::Venomous},
    {EnemyVariant::ReaverChainbound,EnemyArchetype::Reaver,"Chainbound",1.00f,1.00f,1.00f,EnemyAbility::Unshaken},
    {EnemyVariant::ReaverSlagfist,EnemyArchetype::Reaver,"Slagfist",1.30f,1.00f,0.70f,EnemyAbility::Hardened},
    {EnemyVariant::ReaverHollowed,EnemyArchetype::Reaver,"Hollowed",0.85f,1.40f,0.75f,EnemyAbility::Cornered},
    {EnemyVariant::ReaverYokebreaker,EnemyArchetype::Reaver,"Yokebreaker",1.20f,0.95f,0.85f,EnemyAbility::Deadfall},
    {EnemyVariant::ReaverGrindmaw,EnemyArchetype::Reaver,"Grindmaw",0.95f,1.20f,0.85f,EnemyAbility::Sundering},
    {EnemyVariant::WhisperAshling,EnemyArchetype::Whisper,"Ashling",1.00f,1.00f,1.00f,EnemyAbility::Swift},
    {EnemyVariant::WhisperNightcut,EnemyArchetype::Whisper,"Nightcut",0.75f,1.45f,0.80f,EnemyAbility::Cornered},
    {EnemyVariant::WhisperVeilwalk,EnemyArchetype::Whisper,"Veilwalk",0.85f,0.95f,1.20f,EnemyAbility::Swift},
    {EnemyVariant::WhisperGutterghost,EnemyArchetype::Whisper,"Gutterghost",0.90f,1.05f,1.05f,EnemyAbility::Blinding},
    {EnemyVariant::WhisperMourner,EnemyArchetype::Whisper,"Mourner",1.15f,0.95f,0.90f,EnemyAbility::Echo},
    {EnemyVariant::DronePatternOne,EnemyArchetype::Drone,"Pattern One",1.00f,1.00f,1.00f,EnemyAbility::None},
    {EnemyVariant::DroneInterdictor,EnemyArchetype::Drone,"Interdictor",1.25f,0.90f,0.85f,EnemyAbility::Bulwark},
    {EnemyVariant::DroneLancer,EnemyArchetype::Drone,"Lancer",0.80f,1.40f,0.80f,EnemyAbility::Sundering},
    {EnemyVariant::DroneRelay,EnemyArchetype::Drone,"Relay",1.05f,0.85f,1.10f,EnemyAbility::StandardSupport},
    {EnemyVariant::DroneKillSwitch,EnemyArchetype::Drone,"Kill Switch",0.90f,1.05f,1.05f,EnemyAbility::Deadfall},
    {EnemyVariant::LictorSanctioned,EnemyArchetype::Lictor,"Sanctioned",1.00f,1.00f,1.00f,EnemyAbility::Unshaken},
    {EnemyVariant::LictorAegis,EnemyArchetype::Lictor,"Aegis",1.30f,0.85f,0.85f,EnemyAbility::Bulwark},
    {EnemyVariant::LictorCensor,EnemyArchetype::Lictor,"Censor",0.90f,1.35f,0.75f,EnemyAbility::Sundering},
    {EnemyVariant::LictorCustodian,EnemyArchetype::Lictor,"Custodian",1.15f,0.95f,0.90f,EnemyAbility::Echo},
    {EnemyVariant::LictorInquisitor,EnemyArchetype::Lictor,"Inquisitor",0.95f,1.15f,0.90f,EnemyAbility::Overcharged},
    {EnemyVariant::AdeptAcolyte,EnemyArchetype::Adept,"Acolyte",1.00f,1.00f,1.00f,EnemyAbility::StandardSupport},
    {EnemyVariant::AdeptVivifier,EnemyArchetype::Adept,"Vivifier",1.20f,0.85f,0.95f,EnemyAbility::Knitting|EnemyAbility::StandardSupport},
    {EnemyVariant::AdeptResonant,EnemyArchetype::Adept,"Resonant",0.85f,1.30f,0.85f,EnemyAbility::Overcharged|EnemyAbility::StandardSupport},
    {EnemyVariant::AdeptMarshal,EnemyArchetype::Adept,"Marshal",1.10f,0.90f,1.00f,EnemyAbility::StandardSupport},
    {EnemyVariant::AdeptNullSpeaker,EnemyArchetype::Adept,"Null Speaker",0.90f,1.05f,1.05f,EnemyAbility::ElementalHardened|EnemyAbility::StandardSupport}
}};

constexpr std::array<EnemyBossProfile,2> kBossProfiles{{
    {EnemyArchetype::Choir,"Choir of the Uncounted",3,true,true,2,0.10f,0.0f,0.0f,1.0f,0.0f},
    {EnemyArchetype::Praetor,"Praetor of the Sanctioned Answer",2,true,true,0,0.0f,0.25f,0.02f,1.25f,0.35f}
}};

} // namespace

const char* enemyArchetypeName(EnemyArchetype archetype) {
    return kProfiles.at(static_cast<std::size_t>(archetype)).name.data();
}

const EnemyProfile& enemyProfile(EnemyArchetype archetype) {
    const auto index=static_cast<std::size_t>(archetype);
    if(index>=kProfiles.size()) throw std::out_of_range("invalid enemy archetype");
    return kProfiles[index];
}

const EnemyVariantProfile& enemyVariantProfile(EnemyVariant variant) {
    const auto index=static_cast<std::size_t>(variant);
    if(index>=kVariants.size()) throw std::out_of_range("invalid enemy variant");
    return kVariants[index];
}

const EnemyBossProfile* enemyBossProfile(EnemyArchetype archetype) {
    for(const auto& boss:kBossProfiles) if(boss.archetype==archetype) return &boss;
    return nullptr;
}

EnemyProfile resolvedEnemyProfile(EnemyArchetype archetype,EnemyVariant variant) {
    EnemyProfile out=enemyProfile(archetype);
    if(variant==EnemyVariant::Base) return out;
    const auto& mod=enemyVariantProfile(variant);
    if(mod.archetype!=archetype) throw std::invalid_argument("enemy variant does not belong to archetype");
    out.name=mod.name;
    out.maxHealth*=mod.healthMultiplier;
    out.attackDamage*=mod.damageMultiplier;
    out.moveSpeed*=mod.speedMultiplier;
    if(hasAbility(mod.abilities,EnemyAbility::StandardSupport)) {
        out.supportRange=out.supportRange>0.0f?out.supportRange:7.5f;
        out.supportHeal=out.supportHeal>0.0f?out.supportHeal:4.5f;
    }
    return out;
}

} // namespace elysium
