// Intended function: imported tests implementation for citizen_tests; preserves the agent-authored subsystem contract for later integration/debugging.
#include "ecs/CitizenLifecycle.hpp"
#include "ecs/CitizenPersistence.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

using namespace elysium;

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

FoundingExpeditionSpec fixtureSpec() {
    FoundingExpeditionSpec spec{};
    spec.campaignSeed = 0xE1717A17ULL;
    spec.settlementId = 0x510E0001ULL;
    spec.factionId = 0xFAC71001ULL;
    spec.speciesRef = "elysium:species/founding_people";
    spec.raceRef = "elysium:race/unsworn";
    spec.bodyPlanRef = "elysium:body/humanoid";
    spec.culturalOrganizationIds = {0xC0170001ULL, 0xC0170002ULL};
    spec.survivalSupplyContentIds = {"elysium:item/ration", "elysium:item/oxygen_canister"};
    spec.toolContentIds = {"elysium:item/mining_tool", "elysium:item/construction_tool"};
    return spec;
}


void testArrivalBirthCreationAndStageTransitions() {
    CitizenCreationSpec birth{};
    birth.stableId = 0xB1700001ULL;
    birth.entryKind = CitizenEntryKind::Birth;
    birth.identity = {"Nova", "elysium:species/founding_people", "elysium:race/unsworn", true};
    birth.membership.homeSettlementId = 0x510E0001ULL;
    birth.membership.currentSettlementId = 0x510E0001ULL;
    birth.membership.citizenship = CitizenshipStatus::Citizen;
    birth.lifeStage.stage = CitizenLifeStage::Dependent;
    birth.body.bodyPlanRef = "elysium:body/humanoid";
    birth.profession.professionRef = "elysium:profession/dependent";
    birth.laborCapabilities.base = CitizenLifecycleSystem::fullCapabilityValues();
    auto citizen = CitizenLifecycleSystem::createCitizen(birth);
    require(citizen.lifeStage.stage == CitizenLifeStage::Dependent && citizen.persistentId.value == birth.stableId,
            "birth creation did not preserve explicit stable identity/life stage");

    std::vector<CitizenLifecycleEvent> events;
    require(CitizenLifecycleSystem::transitionLifeStage(citizen, CitizenLifeStage::Trainee, &events),
            "dependent could not transition to trainee");
    require(CitizenLifecycleSystem::transitionLifeStage(citizen, CitizenLifeStage::Adult, &events),
            "trainee could not transition to adult");
    require(!CitizenLifecycleSystem::transitionLifeStage(citizen, CitizenLifeStage::Dependent, &events),
            "life-cycle system allowed an ordinary reverse age-stage transition");
    require(CitizenLifecycleSystem::transitionLifeStage(citizen, CitizenLifeStage::Elder, &events),
            "adult could not transition to elder/veteran");
    require(events.size() == 3, "life-stage transition event count is not deterministic");

    auto created = birth;
    created.stableId = 0xC2EA7101ULL;
    created.entryKind = CitizenEntryKind::Creation;
    created.identity.name = "Synth-1";
    created.lifeStage.stage = CitizenLifeStage::Adult;
    require(CitizenLifecycleSystem::createCitizen(created).lifeStage.stage == CitizenLifeStage::Adult,
            "artificial creation incorrectly forced human birth semantics");
}

void testSevenFounderFixture() {
    const auto spec = fixtureSpec();
    const auto result = CitizenLifecycleSystem::createFoundingExpedition(spec);
    require(result.founders.size() == 7, "default founding expedition did not create seven pioneers");
    require(result.survivalSupplyContentIds == spec.survivalSupplyContentIds,
            "founding expedition lost explicit survival manifest");
    require(result.toolContentIds == spec.toolContentIds,
            "founding expedition lost explicit tool manifest");

    std::set<CitizenStableId> ids;
    const auto rules = CitizenLifeStageRules::conservativeDefaults();
    for (const auto& founder : result.founders) {
        std::string error;
        require(CitizenLifecycleSystem::validate(founder, &error), "invalid founder fixture: " + error);
        require(ids.insert(founder.persistentId.value).second, "founding expedition generated duplicate StableId");
        require(founder.membership.homeSettlementId == spec.settlementId &&
                founder.membership.currentSettlementId == spec.settlementId,
                "founder settlement membership is invalid");
        require(founder.membership.citizenship == CitizenshipStatus::Citizen,
                "founder did not receive citizen legal status");
        require(founder.lifeStage.stage == CitizenLifeStage::Adult,
                "founder did not begin as an adult pioneer");
        require(founder.persistence.historicalSignificance && !founder.persistence.allowRemoteCohort,
                "founder was not pinned as a historical individual");
        const auto summary = CitizenLifecycleSystem::capabilities(founder, rules);
        require(summary.alive && summary.available(CitizenCapability::Walk) &&
                summary.available(CitizenCapability::Breathe),
                "founder lacks valid body capabilities");
        require(!founder.profession.professionRef.empty(), "founder lacks explicit data-driven role");
    }

    const auto mirror = CitizenLifecycleSystem::createFoundingExpedition(spec);
    for (std::size_t i = 0; i < result.founders.size(); ++i)
        require(result.founders[i].persistentId.value == mirror.founders[i].persistentId.value,
                "same founding seed/configuration produced different stable identities");

    auto expanded = spec;
    expanded.founderCount = 11;
    require(CitizenLifecycleSystem::createFoundingExpedition(expanded).founders.size() == 11,
            "founder count is hard-coded to seven instead of data-driven");
}

void testCapabilityBridgeIsLifeStageAndInjuryDriven() {
    auto citizen = CitizenLifecycleSystem::createFoundingExpedition(fixtureSpec()).founders.front();
    citizen.profession.professionRef = "elysium:profession/test_profession_a";
    citizen.laborCapabilities.base = CitizenLifecycleSystem::fullCapabilityValues();
    citizen.impairment.multiplier = CitizenLifecycleSystem::fullCapabilityValues();

    CitizenLifeStageRules rules = CitizenLifeStageRules::conservativeDefaults();
    rules.stageMultiplier[static_cast<std::size_t>(CitizenLifeStage::Dependent)] =
        CitizenLifecycleSystem::fullCapabilityValues(0.5f);
    rules.stageMultiplier[static_cast<std::size_t>(CitizenLifeStage::Adult)] =
        CitizenLifecycleSystem::fullCapabilityValues(1.0f);

    citizen.lifeStage.stage = CitizenLifeStage::Adult;
    citizen.impairment.multiplier[static_cast<std::size_t>(CitizenCapability::Walk)] = 0.40f;
    citizen.impairment.multiplier[static_cast<std::size_t>(CitizenCapability::ExtractionConstruction)] = 0.55f;
    auto summary = CitizenLifecycleSystem::capabilities(citizen, rules);
    require(std::abs(summary.value(CitizenCapability::Walk) - 0.40f) < 1e-6f,
            "injury/function multiplier did not affect walking capability");
    require(std::abs(summary.value(CitizenCapability::ExtractionConstruction) - 0.55f) < 1e-6f,
            "injury/function multiplier did not affect work capability");

    citizen.lifeStage.stage = CitizenLifeStage::Dependent;
    summary = CitizenLifecycleSystem::capabilities(citizen, rules);
    require(std::abs(summary.value(CitizenCapability::ExtractionConstruction) - 0.275f) < 1e-6f,
            "life-stage policy did not compose with functional impairment");

    const auto beforeProfessionChange = summary.effective;
    citizen.profession.professionRef = "elysium:profession/completely_different_label";
    const auto afterProfessionChange = CitizenLifecycleSystem::capabilities(citizen, rules).effective;
    require(beforeProfessionChange == afterProfessionChange,
            "capability query contains a hard-coded profession-name switch");

    CitizenLifecycleSystem::synchronizeBiologicalFacts(citizen, 0.25f, 0.5f, 0.75f, false);
    require(std::abs(citizen.biological.nutrition01 - 0.25f) < 1e-6f &&
            std::abs(citizen.biological.rest01 - 0.5f) < 1e-6f &&
            std::abs(citizen.biological.respiration01 - 0.75f) < 1e-6f &&
            !citizen.biological.conscious,
            "survival fact bridge did not accept resolved nutrition/sleep/respiration state");
}

void testRemoteCompactionPromotionAndPersistence() {
    auto citizen = CitizenLifecycleSystem::createFoundingExpedition(fixtureSpec()).founders[1];
    citizen.lifeStage.ageYears = 41.25;
    citizen.possessions.ownedUniqueItemIds = {0x1002, 0x1001};
    citizen.possessions.equippedItemIds = {0x1101};
    citizen.possessions.quartersId = 0x1201;
    citizen.offices.officeIds = {0x2001};
    citizen.military.squadId = 0x3001;
    citizen.military.uniformProfileId = 0x3002;
    citizen.biography.relationshipIds = {0x4002, 0x4001};
    citizen.biography.importantMemoryIds = {0x5001};
    citizen.biography.historyEventIds = {0x6001};
    citizen.strategicLocation.systemId = 0x7001;
    citizen.strategicLocation.planetId = 0x7002;
    citizen.strategicLocation.siteId = citizen.membership.currentSettlementId;
    citizen.navigationIntent = {0xDEADBEEF, 42, true};

    const auto remote = CitizenLifecycleSystem::compactForRemote(
        citizen, 1234567, CitizenSimulationShard::StarSystemStrategic);
    require(remote.persistence.representation == CitizenRepresentation::RemoteIndividual,
            "named founder compacted into anonymous cohort representation");
    require(remote.persistence.lastStrategicUpdate == 1234567,
            "remote compaction lost strategic timestamp");
    require(remote.shardState.shard == CitizenSimulationShard::StarSystemStrategic &&
            remote.shardState.transitionSerial == citizen.shardState.transitionSerial + 1,
            "remote compaction did not record behavioral-LOD shard transition");
    require(remote.strategicLocation.systemId == 0x7001 && remote.strategicLocation.planetId == 0x7002 &&
            remote.strategicLocation.siteId == citizen.membership.currentSettlementId,
            "remote compaction lost durable strategic location");
    require(durableCitizenStateEqual(citizen, remote),
            "remote compaction changed durable biography or possessions");

    const auto text = serializeRemoteCitizenRecord(remote);
    std::string error;
    const auto parsed = deserializeRemoteCitizenRecord(text, &error);
    require(parsed.has_value(), "citizen persistence codec failed round trip: " + error);
    const auto promoted = CitizenLifecycleSystem::promoteFromRemote(*parsed, CitizenSimulationShard::DirectOperativeBubble);
    require(durableCitizenStateEqual(citizen, promoted),
            "promotion after persistence round trip changed durable citizen state");
    require(promoted.persistentId.value == citizen.persistentId.value,
            "promotion changed StableId");
    require(promoted.shardState.shard == CitizenSimulationShard::DirectOperativeBubble &&
            promoted.shardState.transitionSerial == remote.shardState.transitionSerial + 1,
            "promotion did not retain/advance behavioral-LOD transition metadata");
    require(!promoted.navigationIntent.hasIntent && promoted.navigationIntent.targetStableId == 0,
            "promotion incorrectly restored transient navigation cache/intent");
}

void testCitizenPersistenceSchemaV1Migration() {
    auto citizen = CitizenLifecycleSystem::createFoundingExpedition(fixtureSpec()).founders.front();
    auto remote = CitizenLifecycleSystem::compactForRemote(citizen, 99);
    std::string legacy = serializeRemoteCitizenRecord(remote);
    const auto firstNewline = legacy.find('\n');
    require(firstNewline != std::string::npos, "serialized citizen record missing header newline");
    legacy.replace(0, firstNewline, "ELYSIUM_CITIZEN_ENTITY 1");
    const auto locationPos = legacy.find("location ");
    const auto persistencePos = legacy.find("persistence ");
    require(locationPos != std::string::npos && persistencePos != std::string::npos && locationPos < persistencePos,
            "serialized v2 record missing migration rows");
    legacy.erase(locationPos, persistencePos - locationPos);

    std::string error;
    const auto parsed = deserializeRemoteCitizenRecord(legacy, &error);
    require(parsed.has_value(), "schema-v1 citizen record did not migrate: " + error);
    require(parsed->schemaVersion == 1, "legacy reader lost source schema version");
    require(parsed->strategicLocation.siteId == parsed->membership.currentSettlementId,
            "schema-v1 migration did not recover strategic site from current settlement");
    require(parsed->shardState.shard == CitizenSimulationShard::ActivePlanetRemote,
            "schema-v1 migration did not derive a remote simulation shard");
}

void testDeathEmitsStableDownstreamWorkWithoutDeletingBiography() {
    auto citizen = CitizenLifecycleSystem::createFoundingExpedition(fixtureSpec()).founders[2];
    citizen.possessions.ownedUniqueItemIds = {0x9002, 0x9001};
    citizen.offices.officeIds = {0xA002, 0xA001};
    citizen.biography.relationshipIds = {0xB001};
    citizen.biography.importantMemoryIds = {0xC001};
    const auto priorId = citizen.persistentId.value;

    const auto result = CitizenLifecycleSystem::transitionToDeath(citizen, 0xD34DULL);
    require(result.transitioned && result.remainsStableId != 0,
            "death transition did not create deterministic remains request");
    require(citizen.persistentId.value == priorId && citizen.lifeStage.stage == CitizenLifeStage::Deceased &&
            !citizen.biological.alive,
            "death deleted or replaced citizen biography identity");
    require(citizen.possessions.ownedUniqueItemIds.size() == 2 && citizen.offices.officeIds.size() == 2,
            "death orphaned durable item/office references by deleting them before downstream systems could resolve them");

    auto countKind = [&](CitizenLifecycleEventKind kind) {
        return static_cast<int>(std::count_if(result.events.begin(), result.events.end(),
            [&](const CitizenLifecycleEvent& event) { return event.kind == kind; }));
    };
    require(countKind(CitizenLifecycleEventKind::PossessionDispositionRequested) == 2,
            "death did not emit one disposition request per owned unique item");
    require(countKind(CitizenLifecycleEventKind::OfficeVacancyRequested) == 2,
            "death did not emit one vacancy request per office reference");
    require(countKind(CitizenLifecycleEventKind::RelationshipDeathNoticeRequested) == 1,
            "death did not emit relationship notification request");
    require(countKind(CitizenLifecycleEventKind::MemoryDeathNoticeRequested) == 1,
            "death did not emit memory notification request");
    require(countKind(CitizenLifecycleEventKind::MemorialRequested) == 1 &&
            countKind(CitizenLifecycleEventKind::ChronicleDeathRequested) == 1,
            "death did not emit memorial/Chronicle requests");

    const auto repeated = CitizenLifecycleSystem::transitionToDeath(citizen, 0xD34DULL);
    require(!repeated.transitioned && repeated.events.empty(),
            "death transition is not idempotent");

    const auto deadCapabilities = CitizenLifecycleSystem::capabilities(citizen, CitizenLifeStageRules::conservativeDefaults());
    require(std::all_of(deadCapabilities.effective.begin(), deadCapabilities.effective.end(),
                        [](float value) { return value == 0.0f; }),
            "deceased citizen retained effective work/body capabilities");
}

void testRemotePersistenceRules() {
    require(CitizenLifecycleSystem::shouldPersistIndividually({true, true, false, false, false, false}),
            "named citizen was allowed to disappear into cohort state");
    require(CitizenLifecycleSystem::shouldPersistIndividually({false, true, true, false, false, false}),
            "named domestic animal was not pinned as persistent individual");
    require(CitizenLifecycleSystem::shouldPersistIndividually({true, false, false, true, false, false}),
            "historical citizen was not pinned as persistent individual");
    require(CitizenLifecycleSystem::shouldPersistIndividually({true, false, false, false, true, false}),
            "unique-item owner was allowed to cohort-compact");
    require(!CitizenLifecycleSystem::shouldPersistIndividually({true, false, false, false, false, false}),
            "ordinary remote citizen with no durable anchors was incorrectly forced to full individual state");
}

} // namespace

int main() {
    try {
        testArrivalBirthCreationAndStageTransitions();
        testSevenFounderFixture();
        testCapabilityBridgeIsLifeStageAndInjuryDriven();
        testRemoteCompactionPromotionAndPersistence();
        testCitizenPersistenceSchemaV1Migration();
        testDeathEmitsStableDownstreamWorkWithoutDeletingBiography();
        testRemotePersistenceRules();
        std::cout << "Elysium citizen/lifecycle tests: PASS\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Elysium citizen/lifecycle tests: FAIL: " << e.what() << '\n';
        return 1;
    }
}
