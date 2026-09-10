// Intended function: imported tests implementation for rpg_rules_tests; preserves the agent-authored subsystem contract for later integration/debugging.
#include "rules/CharacterRpg.hpp"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>

using namespace elysium::rpg;

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

bool near(double a, double b, double eps=1.0e-9) { return std::abs(a-b) <= eps; }

CharacterBuild baseBuild() {
    CharacterBuild b;
    b.raceId=std::string(ids::raceImperial);
    b.classId=std::string(ids::classMarksman);
    return b;
}

void testProgressionAndStatCurves() {
    require(experienceToNextLevel(1)==100,"level-1 XP requirement changed");
    ProgressionState p;
    auto award=awardExperience(p,100);
    require(award.levelsGained==1 && p.level==2 && p.spendableStatPoints==2 && p.experience==0,
            "XP award did not grant exactly one level/two points");
    require(spendStatPoints(p,Stat::Vitality,2) && p.spendableStatPoints==0 && near(p.spentStats.get(Stat::Vitality),2),
            "manual stat spending failed");
    require(!spendStatPoints(p,Stat::Strength,1),"stat spending allowed an unavailable point");

    const double a=boundedProportion(120,120,1.0);
    const double b=boundedProportion(1.0e12,120,1.0);
    require(near(a,.5) && b>a && b<1.0,"curved stat no longer gives diminishing returns below ceiling");
    require(near(combineIndependentShares(.6,.6),.84),"independent share combination changed");

    StatBlock high;
    for(std::size_t i=0;i<kStatCount;++i) high.values[i]=1.0e12;
    const auto d=deriveStats(high);
    require(std::isfinite(d.regenerationPerTick) && std::isfinite(d.psionicScale) &&
            d.resilienceReduction<1.0 && d.movementSpeedShare<.60 && d.criticalChance<.75 &&
            d.reflexDodgeChance<.50 && d.retributionShare<.80 && d.luckExtraDropChance<.90,
            "high-stat arithmetic became non-finite or hit a dead-end cap");
}

void testCatalogBalanceAndExtensibility() {
    RpgCatalog c;
    registerCanonicalRpgContent(c);
    c.registerRace({"example:race/tester","Tester",{},{{}},
        [](const PassiveContext&,int,double){ PassiveContribution p; p.attackScale=1.1; return p; }});
    // The extra race intentionally means canonical balance validator should fail,
    // proving validators distinguish extensions from the locked six-race corpus.
    require(c.race("example:race/tester").displayName=="Tester","registered extension race was not queryable");
    require(c.frozen(),"first read did not freeze RPG registry");
    bool lateThrew=false;
    try { c.registerClass({"example:class/late","Late",{},nullptr}); } catch(const std::logic_error&) { lateThrew=true; }
    require(lateThrew,"late registration after freeze did not fail loudly");

    auto canonical=makeCanonicalRpgCatalog();
    require(canonical.races().size()==6 && canonical.classes().size()==9 && canonical.runes().size()==9 &&
            canonical.trinkets().size()==40 && canonical.gearFamilies().size()==6,
            "canonical RPG content counts changed");
    require(canonical.validateCanonicalBalance(),"race/class 44/3/2 balance invariant failed");
}

void testElementRingAndTiers() {
    auto c=makeCanonicalRpgCatalog();
    require(c.elements().size()==5 && c.validateClosedElementGraph(),"five-element graph is not closed 2-out/2-in");
    require(c.elementBeats(ids::elementVoid,ids::elementKinetic) && c.elementBeats(ids::elementVoid,ids::elementDimensional),
            "Void relationship changed");
    require(c.element(ids::elementPlasma).environmentalAffinity==HazardAffinity::Thermal &&
            c.element(ids::elementVoid).environmentalAffinity==HazardAffinity::Radiological &&
            c.element(ids::elementNeural).environmentalAffinity==HazardAffinity::Corrosive &&
            c.element(ids::elementDimensional).environmentalAffinity==HazardAffinity::Pressure &&
            c.element(ids::elementKinetic).environmentalAffinity==HazardAffinity::Cryogenic,
            "element/environment mapping changed");
    require(requiredLevelForTier(0)==0 && requiredLevelForTier(5)==25 && requiredLevelForTier(12)==60,
            "tier required-level formula changed");
    require(statWeight(0)==1 && statWeight(1)==2 && statWeight(2)==3 && statWeight(5)==9 && statWeight(12)==55,
            "tier stat-weight anchors changed");
    require(near(elementalAdvantageForTier(0),.05) && near(elementalAdvantageForTier(5),.40) &&
            elementalAdvantageForTier(100)>.40 && elementalAdvantageForTier(100)<1.0,
            "elemental advantage table/asymptote changed");
    require(tierScale(1000000)==1.0e6 && std::isfinite(tierAdded(1.0,1000000)),
            "unbounded tier safety ceiling failed");
}

void testRaceClassAndPassiveAggregation() {
    auto c=makeCanonicalRpgCatalog();
    CharacterBuild b=baseBuild();
    b.progression.level=50;
    PassiveContext ctx; ctx.level=50;
    auto s=evaluatePassives(c,b,ctx);
    require(near(s.reflectShare,50.0/150.0) && near(s.critMultiplier,2.25),
            "race/class passive hooks did not compose");

    b.raceId=std::string(ids::raceDruun);
    b.classId=std::string(ids::classEnforcer);
    ctx.healthFraction=.25; ctx.targetIsUnsworn=true;
    s=evaluatePassives(c,b,ctx);
    require(near(s.attackScale,(1.0+.60*.75)*1.25),"multiplicative attack hooks combined incorrectly");

    b.raceId=std::string(ids::raceKorrath);
    b.classId=std::string(ids::classVoidrunner);
    ctx={}; ctx.level=10; ctx.secondsSinceLastHit=6; ctx.hurtRecovery=false;
    s=evaluatePassives(c,b,ctx);
    require(near(s.regenScale,3.0) && near(s.fallDamageScale,.5),"Korrath/Voidrunner condition hooks changed");
    ctx.hurtRecovery=true;
    require(near(evaluatePassives(c,b,ctx).regenScale,1.0),"Korrath incorrectly counted hurt-recovery as untouched");
}

void testGearStatsSocketingAndRunes() {
    auto c=makeCanonicalRpgCatalog();
    GearItemDefinition def{"example:gear/plasma_plate","Plasma Plate",std::string(ids::elementPlasma),GearSlot::Chest,2,true,true,1.0,100.0,std::nullopt};
    auto item=makeGearItem(def);
    require(item.tier==2 && socketCapacityForTier(item.tier)==2,"initial gear tier/socket capacity wrong");
    std::string why;
    require(socketRune(item,def,c,ids::runePlasmaforge,&why),"failed to socket first rune: "+why);
    require(!socketRune(item,def,c,ids::runePlasmaforge,&why) && why.find("duplicate")!=std::string::npos,
            "duplicate rune restriction failed");
    require(socketRune(item,def,c,ids::runeReflex,&why),"failed to socket utility rune");
    require(!socketRune(item,def,c,ids::runeBarrier,&why),"socket capacity was ignored");
    require(near(runeAlignmentMultiplier(def,c.rune(ids::runePlasmaforge)),1.75) &&
            near(runeAlignmentMultiplier(def,c.rune(ids::runeReflex)),1.0),
            "rune alignment multiplier changed");

    CharacterBuild b=baseBuild();
    b.gearDefinitions.push_back(def);
    b.gear.push_back(item);
    PassiveContext ctx; ctx.healthFraction=.8;
    const auto passive=evaluatePassives(c,b,ctx);
    require(near(passive.attackDamageAdd,1.5*1.75) && passive.strengthEffect && passive.dodgeChance>0.0,
            "socketed rune behavior/alignment not exposed through passive hooks");

    const auto total=evaluateCharacterStats(c,b);
    // Imperial L1 base Plasma grants STR+ACC by weight=3, armour adds FOR+1.
    require(near(total.get(Stat::Strength),4+3) && near(total.get(Stat::Accuracy),3+3) && near(total.get(Stat::Fortitude),4+1),
            "gear->character stat mapping changed or reforge double-counted");
}

void testReforgeAndAscension() {
    GearItemDefinition def{"example:gear/ascender","Ascender",std::nullopt,GearSlot::Ring,5,false,true,1.25,120.0,std::nullopt};
    auto a=makeGearItem(def);
    auto b=makeGearItem(def);
    const auto r1=reforgeGear(a,def,1.5,0x12345678ULL);
    auto a2=makeGearItem(def);
    const auto r2=reforgeGear(a2,def,1.5,0x12345678ULL);
    require(r1.success && r2.success && r1.rolls==r2.rolls && a.reforgeCharges==2,
            "reforge is not deterministic or did not spend exactly one charge");
    require(r1.rolls.armour>=1 && r1.rolls.health>=1 && r1.rolls.speed>=1,"reforge produced invalid positive roll");
    require(reforgeBasePoints(5)==25 && reforgeBasePoints(100)==100000 &&
            reforgeFinalPoints(1000000,1.0,1.0)>0,
            "high-tier reforge arithmetic regressed");

    a.condition=1.0;
    a.socketedRunes.push_back(std::string(ids::runeReflex));
    const auto previousRolls=a.reforgeRolls;
    std::optional<GearItemState> counterpart=b;
    const auto result=ascendGear(a,counterpart,def);
    require(result.success && !counterpart && a.tier==6 && near(a.condition,120.0) && a.reforgeCharges==3 &&
            a.socketedRunes.size()==1 && a.reforgeRolls==previousRolls,
            "ascension did not consume equal-tier counterpart/preserve state/repair/refill");

    auto bad=makeGearItem(def);
    bad.tier=7;
    counterpart=bad;
    require(!ascendGear(a,counterpart,def).success && counterpart.has_value(),
            "unequal-tier ascension consumed invalid counterpart");
}

void testTrinketBehaviorsAndCraftedScaling() {
    auto c=makeCanonicalRpgCatalog();
    CharacterBuild b=baseBuild();
    b.raceId=std::string(ids::raceUnsworn); // Presence 0 keeps standing scales easy to inspect.
    b.classId=std::string(ids::classVoidrunner);
    b.trinkets={{"elysium:trinket/cracked_reliquary",0},{"elysium:trinket/quiet_hours",0},{"elysium:trinket/aetherium_band",3}};
    PassiveContext ctx; ctx.healthFraction=.2;
    auto s=evaluatePassives(c,b,ctx);
    require(near(s.regenScale,3.0) && near(s.favorScale,.25) && near(s.suspicionScale,.25) &&
            s.attackScale>1.15,
            "found/crafted trinket hooks did not compose or crafted ascension failed to scale");

    b.trinkets={{"elysium:trinket/prospectors_lens",0},{"elysium:trinket/miners_rig",8}};
    s=evaluatePassives(c,b,ctx);
    require(s.doublesOreAlways && near(s.xpScale,.6) && s.savesDurabilityChance>0.20 && s.doublesOreChance>0.12,
            "ore/durability trinket behavior or unbounded crafted scaling changed");

    const auto& found=c.trinket("elysium:trinket/long_memory");
    const auto& crafted=c.trinket("elysium:trinket/bloodlet_ring");
    require(found.found && !found.canAscend && found.baseTier==0 && !crafted.found && crafted.canAscend && crafted.baseTier==2,
            "found-vs-crafted trinket identity rules changed");
}

void testEventsAndRuneUtilities() {
    auto c=makeCanonicalRpgCatalog();
    CharacterBuild b=baseBuild();
    b.trinkets={{"elysium:trinket/carrion_signet",0},{"elysium:trinket/tithe_bracelet",0},{"elysium:trinket/pale_tourniquet",0},{"elysium:trinket/long_memory",0}};
    PassiveContext ctx; ctx.event=PassiveEvent::Kill; ctx.victimMaxHealth=100;
    auto s=evaluatePassives(c,b,ctx);
    require(near(s.healOnKill,6.0) && near(s.bonusFavorOnKill,1.0) && s.blockStandingDecay,
            "kill/standing event outputs changed");
    ctx.event=PassiveEvent::Damaged; ctx.incomingDamage=6;
    s=evaluatePassives(c,b,ctx);
    require(near(s.resistanceSecondsOnDamaged,3.0),"onDamaged trinket event output changed");

    GearItemDefinition def{"example:gear/util_rune_host","Utility Host",std::nullopt,GearSlot::Weapon,20,false,true,1,100,std::nullopt};
    auto item=makeGearItem(def);
    require(socketRune(item,def,c,ids::runeStabilizer) && socketRune(item,def,c,ids::runeReflex) &&
            socketRune(item,def,c,ids::runeBarrier) && socketRune(item,def,c,ids::runePlasmaCore),
            "high-tier utility rune socket setup failed");
    b.gearDefinitions={def}; b.gear={item}; b.trinkets.clear();
    ctx={}; ctx.healthFraction=.5;
    s=evaluatePassives(c,b,ctx);
    require(near(s.periodicHealAdd,.5) && s.dodgeChance>0 && near(s.barrierCapacityAdd,4) && near(s.barrierRefreshAdd,1) &&
            near(s.heatReductionAdd,.12) && near(s.cappedHeatReduction(),.12),
            "utility rune effects changed");
}

void testGearFamiliesAndHighScaleFinite() {
    auto c=makeCanonicalRpgCatalog();
    require(c.gearFamily("elysium:gear_family/hazard_suit_modules").purposes.size()==5 &&
            c.gearFamily("elysium:gear_family/utility_harness").slots.size()==2 &&
            c.gearFamily("elysium:gear_family/industrial_exoskeleton").slots.size()==1 &&
            c.gearFamily("elysium:gear_family/survey_kit").purposes.size()==4 &&
            c.gearFamily("elysium:gear_family/siege_kit").purposes.size()==3 &&
            c.gearFamily("elysium:gear_family/explorer_kit").purposes.size()==3,
            "expanded gear-family catalogue incomplete");

    CharacterBuild b=baseBuild();
    b.progression.level=1000000;
    GearItemDefinition def{"example:gear/high_tier","High Tier",std::string(ids::elementVoid),GearSlot::Chest,1000000,true,true,2.0,100,std::nullopt};
    auto item=makeGearItem(def);
    item.reforgeRolls={1000000,1000000,1000000};
    b.gearDefinitions={def}; b.gear={item};
    b.trinkets={{"elysium:trinket/aetherium_band",1000000},{"elysium:trinket/bloodlet_ring",1000000}};
    const auto total=evaluateCharacterStats(c,b);
    const auto d=deriveStats(total);
    PassiveContext ctx; ctx.level=b.progression.level;
    const auto p=evaluatePassives(c,b,ctx,&total);
    require(std::isfinite(total.sum()) && std::isfinite(d.regenerationPerTick) && std::isfinite(p.attackScale) &&
            p.lifestealShare<1.0 && d.resilienceReduction<1.0 && p.extraDropChance<1.0,
            "high level/tier arithmetic became non-finite or saturated to certainty");
}

void testDeterministicChance() {
    for(std::uint64_t seed=0;seed<100;++seed) {
        require(deterministicChance(seed,.37)==deterministicChance(seed,.37),"chance query changed for same entropy");
    }
    require(!deterministicChance(1,0.0) && deterministicChance(1,1.0),"chance boundary behavior changed");
}

void testCanonicalTrinketLoadoutValidation() {
    auto c=makeCanonicalRpgCatalog();
    require(trinketSlotCapacity(GearSlot::Ring)==2 && trinketSlotCapacity(GearSlot::Charm)==1 &&
            trinketSlotCapacity(GearSlot::Weapon)==0 && trinketSlotCapacity(GearSlot::Frame)==0,
            "canonical eight-slot accessory capacities changed");

    CharacterBuild b=baseBuild();
    b.progression.level=25;
    b.trinkets={
        {"elysium:trinket/widows_thimble",0},
        {"elysium:trinket/duellists_cuff",0},
        {"elysium:trinket/ashen_mantle",0},
        {"elysium:trinket/iron_discipline",0},
        {"elysium:trinket/nine_tenths_charm",0},
        {"elysium:trinket/ratchet_gauntlet",0},
        {"elysium:trinket/prospectors_lens",0},
        {"elysium:trinket/cracked_reliquary",0}
    };
    require(validateTrinketLoadout(c,b).valid,"legal eight-slot trinket loadout was rejected");

    b.trinkets.push_back({"elysium:trinket/tithe_bracelet",0});
    const auto overflow=validateTrinketLoadout(c,b);
    require(!overflow.valid && !overflow.issues.empty(),"third ring was not rejected");

    b=baseBuild();
    b.progression.level=1;
    b.trinkets={{"elysium:trinket/long_memory",0}};
    require(!validateTrinketLoadout(c,b).valid,"trinket level requirement was not enforced by validation");

    b.progression.level=25;
    b.trinkets={{"elysium:trinket/long_memory",1}};
    require(!validateTrinketLoadout(c,b).valid,"found trinket ascension was not rejected");

    b.trinkets={{"elysium:trinket/aetherium_band",-1}};
    require(!validateTrinketLoadout(c,b).valid,"negative crafted ascension count was not rejected");
}

void testPassiveBreakdownExplainability() {
    auto c=makeCanonicalRpgCatalog();
    CharacterBuild b=baseBuild();
    b.progression.level=25;
    b.trinkets={{"elysium:trinket/duellists_cuff",0}};
    GearItemDefinition def{"example:gear/explainable","Explainable",std::string(ids::elementPlasma),GearSlot::Chest,4,true,true,1,100,std::nullopt};
    auto item=makeGearItem(def);
    require(socketRune(item,def,c,ids::runePlasmaforge),"passive breakdown rune setup failed");
    b.gearDefinitions={def};
    b.gear={item};

    PassiveContext ctx;
    ctx.level=25;
    ctx.targetHealthFraction=1.0;
    ctx.healthFraction=.8;
    const auto ordinary=evaluatePassives(c,b,ctx);
    const auto breakdown=evaluatePassiveBreakdown(c,b,ctx);
    require(breakdown.sources.size()==4,"passive breakdown did not expose race/class/trinket/rune sources");
    require(breakdown.sources[0].kind==PassiveSourceKind::Race && breakdown.sources[0].sourceId==ids::raceImperial,
            "passive breakdown race provenance changed");
    require(breakdown.sources[1].kind==PassiveSourceKind::Class && breakdown.sources[1].sourceId==ids::classMarksman,
            "passive breakdown class provenance changed");
    require(breakdown.sources[2].kind==PassiveSourceKind::Trinket &&
            breakdown.sources[2].sourceId=="elysium:trinket/duellists_cuff",
            "passive breakdown trinket provenance changed");
    require(breakdown.sources[3].kind==PassiveSourceKind::Rune && breakdown.sources[3].sourceId==ids::runePlasmaforge &&
            near(breakdown.sources[3].amplifier,1.75),
            "passive breakdown rune alignment provenance changed");
    require(near(ordinary.attackScale,breakdown.summary.attackScale) &&
            near(ordinary.critMultiplier,breakdown.summary.critMultiplier) &&
            near(ordinary.attackDamageAdd,breakdown.summary.attackDamageAdd) &&
            near(ordinary.favorScale,breakdown.summary.favorScale),
            "passive breakdown aggregate diverged from ordinary evaluation");
}

} // namespace

int main() {
    try {
        testProgressionAndStatCurves();
        testCatalogBalanceAndExtensibility();
        testElementRingAndTiers();
        testRaceClassAndPassiveAggregation();
        testGearStatsSocketingAndRunes();
        testReforgeAndAscension();
        testTrinketBehaviorsAndCraftedScaling();
        testEventsAndRuneUtilities();
        testGearFamiliesAndHighScaleFinite();
        testDeterministicChance();
        testCanonicalTrinketLoadoutValidation();
        testPassiveBreakdownExplainability();
        std::cout << "Agent 15 RPG/gear/passive headless tests passed\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "Agent 15 RPG/gear/passive test failure: " << e.what() << '\n';
        return 1;
    }
}
