// Intended function: imported ecs implementation for CitizenLifecycle; preserves the agent-authored subsystem contract for later integration/debugging.
#include "ecs/CitizenLifecycle.hpp"
#include "ecs/CitizenPersistence.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <stdexcept>
#include <tuple>

namespace elysium {
namespace {

constexpr std::uint64_t kFounderStableIdLabel = 0x4349545A464E4452ULL; // CITZFNDR
constexpr std::uint64_t kRemainsStableIdLabel = 0x4349545A52454D4EULL; // CITZREMN

std::size_t stageIndex(CitizenLifeStage stage) {
    return static_cast<std::size_t>(stage);
}

float clamp01(float v) {
    if (!std::isfinite(v)) return 0.0f;
    return std::clamp(v, 0.0f, 1.0f);
}

CitizenCapabilityValues filled(float value) {
    CitizenCapabilityValues out{};
    out.fill(value);
    return out;
}

void setWorkDomains(CitizenCapabilityValues& values, float value) {
    for (std::size_t i = static_cast<std::size_t>(CitizenCapability::ExtractionConstruction);
         i < citizenCapabilityCount; ++i) {
        values[i] = value;
    }
}

void sortUniqueNonZero(std::vector<CitizenStableId>& values) {
    values.erase(std::remove(values.begin(), values.end(), CitizenStableId{}), values.end());
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

bool vectorEqual(const std::vector<CitizenStableId>& a, const std::vector<CitizenStableId>& b) {
    return a == b;
}

bool capabilityValuesEqual(const CitizenCapabilityValues& a, const CitizenCapabilityValues& b) {
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::abs(a[i] - b[i]) > 1e-6f) return false;
    }
    return true;
}

bool durableFieldsEqual(const CitizenRecord& a, const RemoteCitizenRecord& b) {
    return a.persistentId.value == b.persistentId.value &&
           a.identity.name == b.identity.name &&
           a.identity.speciesRef == b.identity.speciesRef &&
           a.identity.raceRef == b.identity.raceRef &&
           a.identity.named == b.identity.named &&
           a.membership.homeSettlementId == b.membership.homeSettlementId &&
           a.membership.currentSettlementId == b.membership.currentSettlementId &&
           a.membership.primaryFactionId == b.membership.primaryFactionId &&
           a.membership.citizenship == b.membership.citizenship &&
           vectorEqual(a.membership.organizationIds, b.membership.organizationIds) &&
           std::abs(a.lifeStage.ageYears - b.lifeStage.ageYears) < 1e-9 &&
           a.lifeStage.birthTime == b.lifeStage.birthTime &&
           a.lifeStage.stage == b.lifeStage.stage &&
           std::abs(a.biological.nutrition01 - b.biological.nutrition01) < 1e-6f &&
           std::abs(a.biological.rest01 - b.biological.rest01) < 1e-6f &&
           std::abs(a.biological.respiration01 - b.biological.respiration01) < 1e-6f &&
           a.biological.conscious == b.biological.conscious &&
           a.biological.alive == b.biological.alive &&
           a.body.bodyPlanRef == b.body.bodyPlanRef &&
           a.body.bodyStateRef == b.body.bodyStateRef &&
           a.profession.professionRef == b.profession.professionRef &&
           capabilityValuesEqual(a.laborCapabilities.base, b.laborCapabilities.base) &&
           capabilityValuesEqual(a.impairment.multiplier, b.impairment.multiplier) &&
           vectorEqual(a.possessions.ownedUniqueItemIds, b.possessions.ownedUniqueItemIds) &&
           vectorEqual(a.possessions.equippedItemIds, b.possessions.equippedItemIds) &&
           a.possessions.quartersId == b.possessions.quartersId &&
           vectorEqual(a.offices.officeIds, b.offices.officeIds) &&
           a.military.squadId == b.military.squadId &&
           a.military.uniformProfileId == b.military.uniformProfileId &&
           vectorEqual(a.biography.relationshipIds, b.biography.relationshipIds) &&
           vectorEqual(a.biography.importantMemoryIds, b.biography.importantMemoryIds) &&
           vectorEqual(a.biography.historyEventIds, b.biography.historyEventIds) &&
           a.strategicLocation.systemId == b.strategicLocation.systemId &&
           a.strategicLocation.planetId == b.strategicLocation.planetId &&
           a.strategicLocation.siteId == b.strategicLocation.siteId &&
           a.persistence.historicalSignificance == b.persistence.historicalSignificance &&
           a.persistence.namedDomestic == b.persistence.namedDomestic &&
           a.persistence.allowRemoteCohort == b.persistence.allowRemoteCohort;
}

bool validateCapabilityValues(const CitizenCapabilityValues& values, const char* field, std::string* error) {
    for (const float value : values) {
        if (!std::isfinite(value) || value < 0.0f || value > 1.0f) {
            if (error) *error = std::string(field) + " contains value outside [0,1]";
            return false;
        }
    }
    return true;
}

bool validateStableRefs(const std::vector<CitizenStableId>& refs, const char* field, std::string* error) {
    std::set<CitizenStableId> seen;
    for (const auto id : refs) {
        if (id == 0) {
            if (error) *error = std::string(field) + " contains zero stable ID";
            return false;
        }
        if (!seen.insert(id).second) {
            if (error) *error = std::string(field) + " contains duplicate stable ID";
            return false;
        }
    }
    return true;
}

bool validateCommon(const RemoteCitizenRecord& citizen, std::string* error) {
    if (citizen.persistentId.value == 0) {
        if (error) *error = "citizen stable ID is zero";
        return false;
    }
    if (citizen.identity.name.empty()) {
        if (error) *error = "citizen name is empty";
        return false;
    }
    if (citizen.identity.speciesRef.empty() || citizen.body.bodyPlanRef.empty()) {
        if (error) *error = "citizen species/body-plan reference is empty";
        return false;
    }
    if (citizen.membership.homeSettlementId == 0 || citizen.membership.currentSettlementId == 0) {
        if (error) *error = "citizen settlement membership is missing";
        return false;
    }
    if (citizen.strategicLocation.siteId == 0) {
        if (error) *error = "citizen strategic site location is missing";
        return false;
    }
    const auto shard = static_cast<std::uint8_t>(citizen.shardState.shard);
    if (shard > static_cast<std::uint8_t>(CitizenSimulationShard::GalaxyHistory)) {
        if (error) *error = "citizen simulation shard is invalid";
        return false;
    }
    if (citizen.lifeStage.stage == CitizenLifeStage::Deceased && citizen.biological.alive) {
        if (error) *error = "deceased citizen is marked alive";
        return false;
    }
    if (citizen.lifeStage.stage != CitizenLifeStage::Deceased && !citizen.biological.alive) {
        if (error) *error = "living life stage is marked dead";
        return false;
    }
    if (!std::isfinite(citizen.lifeStage.ageYears) || citizen.lifeStage.ageYears < 0.0) {
        if (error) *error = "citizen age is invalid";
        return false;
    }
    if (!validateCapabilityValues(citizen.laborCapabilities.base, "base capabilities", error) ||
        !validateCapabilityValues(citizen.impairment.multiplier, "functional impairment", error)) {
        return false;
    }
    if (!validateStableRefs(citizen.membership.organizationIds, "organization links", error) ||
        !validateStableRefs(citizen.possessions.ownedUniqueItemIds, "owned unique item links", error) ||
        !validateStableRefs(citizen.possessions.equippedItemIds, "equipped item links", error) ||
        !validateStableRefs(citizen.offices.officeIds, "office links", error) ||
        !validateStableRefs(citizen.biography.relationshipIds, "relationship links", error) ||
        !validateStableRefs(citizen.biography.importantMemoryIds, "memory links", error) ||
        !validateStableRefs(citizen.biography.historyEventIds, "history links", error)) {
        return false;
    }
    return true;
}

RemoteCitizenRecord toRemote(const CitizenRecord& citizen) {
    RemoteCitizenRecord out{};
    out.persistentId = citizen.persistentId;
    out.identity = citizen.identity;
    out.membership = citizen.membership;
    out.lifeStage = citizen.lifeStage;
    out.biological = citizen.biological;
    out.body = citizen.body;
    out.profession = citizen.profession;
    out.laborCapabilities = citizen.laborCapabilities;
    out.impairment = citizen.impairment;
    out.possessions = citizen.possessions;
    out.offices = citizen.offices;
    out.military = citizen.military;
    out.biography = citizen.biography;
    out.strategicLocation = citizen.strategicLocation;
    out.shardState = citizen.shardState;
    out.persistence = citizen.persistence;
    return out;
}

} // namespace

CitizenLifeStageRules CitizenLifeStageRules::conservativeDefaults() {
    CitizenLifeStageRules rules{};
    for (auto& stage : rules.stageMultiplier) stage = filled(1.0f);

    // TUNING: the specification defines qualitative stage restrictions, not
    // exact multipliers. These conservative defaults are replaceable data and
    // are intentionally not keyed to profession names.
    auto& dependent = rules.stageMultiplier[stageIndex(CitizenLifeStage::Dependent)];
    setWorkDomains(dependent, 0.25f);
    dependent[static_cast<std::size_t>(CitizenCapability::Combat)] = 0.0f;

    auto& trainee = rules.stageMultiplier[stageIndex(CitizenLifeStage::Trainee)];
    setWorkDomains(trainee, 0.70f);
    trainee[static_cast<std::size_t>(CitizenCapability::Combat)] = 0.55f;

    auto& elder = rules.stageMultiplier[stageIndex(CitizenLifeStage::Elder)];
    elder[static_cast<std::size_t>(CitizenCapability::Walk)] = 0.85f;
    elder[static_cast<std::size_t>(CitizenCapability::Grasp)] = 0.90f;
    elder[static_cast<std::size_t>(CitizenCapability::ExtractionConstruction)] = 0.80f;
    elder[static_cast<std::size_t>(CitizenCapability::MetallurgyFabrication)] = 0.90f;
    elder[static_cast<std::size_t>(CitizenCapability::UtilitiesMaintenance)] = 0.90f;
    elder[static_cast<std::size_t>(CitizenCapability::AgricultureFood)] = 0.90f;
    elder[static_cast<std::size_t>(CitizenCapability::Combat)] = 0.75f;

    rules.stageMultiplier[stageIndex(CitizenLifeStage::Deceased)] = filled(0.0f);
    return rules;
}

const CitizenCapabilityValues& CitizenLifeStageRules::forStage(CitizenLifeStage stage) const {
    const auto index = stageIndex(stage);
    if (index >= stageMultiplier.size()) throw std::out_of_range("invalid citizen life stage");
    return stageMultiplier[index];
}

CitizenCapabilityValues CitizenLifecycleSystem::fullCapabilityValues(float value) {
    return filled(clamp01(value));
}

std::vector<FounderRoleDefinition> CitizenLifecycleSystem::defaultFounderRoles() {
    auto role = [](const char* id, CitizenCapability primary) {
        FounderRoleDefinition r{};
        r.professionRef = id;
        r.baseCapabilities = filled(0.75f);
        for (std::size_t i = 0; i < static_cast<std::size_t>(CitizenCapability::ExtractionConstruction); ++i)
            r.baseCapabilities[i] = 1.0f;
        r.baseCapabilities[static_cast<std::size_t>(primary)] = 1.0f;
        return r;
    };
    return {
        role("elysium:profession/expedition_lead", CitizenCapability::AdministrationSocial),
        role("elysium:profession/extractor", CitizenCapability::ExtractionConstruction),
        role("elysium:profession/builder", CitizenCapability::ExtractionConstruction),
        role("elysium:profession/systems_engineer", CitizenCapability::UtilitiesMaintenance),
        role("elysium:profession/medicae", CitizenCapability::Medicine),
        role("elysium:profession/agronomist", CitizenCapability::AgricultureFood),
        role("elysium:profession/scout", CitizenCapability::Combat)
    };
}

CitizenStableId CitizenLifecycleSystem::founderStableId(const FoundingExpeditionSpec& spec, std::size_t ordinal) {
    std::uint64_t id = mix64(spec.campaignSeed ^ kFounderStableIdLabel ^ mix64(spec.settlementId) ^ mix64(static_cast<std::uint64_t>(ordinal) + 1ULL));
    if (id == 0) id = mix64(kFounderStableIdLabel ^ static_cast<std::uint64_t>(ordinal) ^ 1ULL);
    return id == 0 ? static_cast<CitizenStableId>(ordinal + 1ULL) : id;
}

CitizenStableId CitizenLifecycleSystem::deathRemainsStableId(CitizenStableId citizenId, std::uint64_t eventSeed) {
    std::uint64_t id = mix64(citizenId ^ kRemainsStableIdLabel ^ mix64(eventSeed));
    if (id == 0) id = mix64(citizenId ^ kRemainsStableIdLabel ^ 1ULL);
    return id == 0 ? 1ULL : id;
}

CitizenRecord CitizenLifecycleSystem::createCitizen(const CitizenCreationSpec& spec) {
    if (spec.stableId == 0) throw std::invalid_argument("citizen creation requires nonzero StableId");
    if (spec.identity.name.empty()) throw std::invalid_argument("citizen creation requires a name");
    if (spec.identity.speciesRef.empty() || spec.body.bodyPlanRef.empty())
        throw std::invalid_argument("citizen creation requires species/body-plan references");
    if (spec.membership.homeSettlementId == 0 || spec.membership.currentSettlementId == 0)
        throw std::invalid_argument("citizen creation requires home/current settlement membership");

    CitizenRecord citizen{};
    citizen.persistentId.value = spec.stableId;
    citizen.identity = spec.identity;
    citizen.membership = spec.membership;
    sortUniqueNonZero(citizen.membership.organizationIds);
    citizen.lifeStage = spec.lifeStage;
    citizen.body = spec.body;
    citizen.profession = spec.profession;
    citizen.laborCapabilities = spec.laborCapabilities;
    citizen.impairment.multiplier = filled(1.0f);
    citizen.strategicLocation = spec.strategicLocation;
    if (citizen.strategicLocation.siteId == 0) citizen.strategicLocation.siteId = spec.membership.currentSettlementId;
    citizen.shardState = spec.shardState;
    citizen.shardState.shard = CitizenSimulationShard::ActiveFortress;
    citizen.persistence = spec.persistence;
    citizen.persistence.representation = CitizenRepresentation::Active;

    // Entry kind is intentionally not used to infer a human life stage. A
    // species/content layer chooses the initial stage explicitly; this keeps
    // birth/arrival/artificial creation compatible with non-human species.
    (void)spec.entryKind;

    if (citizen.lifeStage.stage == CitizenLifeStage::Deceased) {
        citizen.biological.alive = false;
        citizen.biological.conscious = false;
    }

    std::string error;
    if (!validate(citizen, &error)) throw std::invalid_argument("invalid citizen creation state: " + error);
    return citizen;
}

FoundingExpeditionResult CitizenLifecycleSystem::createFoundingExpedition(const FoundingExpeditionSpec& spec) {
    if (spec.founderCount == 0) throw std::invalid_argument("founding expedition must contain at least one founder");
    if (spec.settlementId == 0) throw std::invalid_argument("founding expedition requires nonzero settlement stable ID");
    if (spec.speciesRef.empty() || spec.bodyPlanRef.empty()) throw std::invalid_argument("founding expedition requires species/body-plan references");

    const auto roles = spec.roles.empty() ? defaultFounderRoles() : spec.roles;
    if (roles.empty()) throw std::invalid_argument("founding expedition role table is empty");
    for (const auto& role : roles) {
        if (role.professionRef.empty()) throw std::invalid_argument("founder role has empty profession reference");
        std::string error;
        if (!validateCapabilityValues(role.baseCapabilities, "founder role capabilities", &error))
            throw std::invalid_argument(error);
    }

    FoundingExpeditionResult out{};
    out.survivalSupplyContentIds = spec.survivalSupplyContentIds;
    out.toolContentIds = spec.toolContentIds;
    out.founders.reserve(spec.founderCount);

    std::set<CitizenStableId> allocated;
    for (std::size_t i = 0; i < spec.founderCount; ++i) {
        CitizenStableId id = founderStableId(spec, i);
        std::uint64_t collisionOrdinal = 0;
        while (id == 0 || !allocated.insert(id).second) {
            ++collisionOrdinal;
            id = mix64(id ^ mix64(collisionOrdinal));
        }

        const auto& roleDef = roles[i % roles.size()];
        CitizenCreationSpec creation{};
        creation.stableId = id;
        creation.entryKind = CitizenEntryKind::Arrival;
        creation.identity.name = i < spec.founderNames.size() && !spec.founderNames[i].empty()
            ? spec.founderNames[i]
            : "Pioneer-" + std::to_string(i + 1);
        creation.identity.speciesRef = spec.speciesRef;
        creation.identity.raceRef = spec.raceRef;
        creation.identity.named = true;
        creation.membership.homeSettlementId = spec.settlementId;
        creation.membership.currentSettlementId = spec.settlementId;
        creation.membership.primaryFactionId = spec.factionId;
        creation.membership.citizenship = CitizenshipStatus::Citizen;
        creation.membership.organizationIds = spec.culturalOrganizationIds;
        creation.lifeStage.stage = CitizenLifeStage::Adult;
        creation.body.bodyPlanRef = spec.bodyPlanRef;
        creation.profession.professionRef = roleDef.professionRef;
        creation.laborCapabilities.base = roleDef.baseCapabilities;
        creation.strategicLocation.siteId = spec.settlementId;
        creation.shardState.shard = CitizenSimulationShard::ActiveFortress;
        creation.persistence.historicalSignificance = true;
        creation.persistence.allowRemoteCohort = false;
        out.founders.push_back(createCitizen(creation));
    }
    return out;
}

CitizenCapabilitySummary CitizenLifecycleSystem::capabilities(const CitizenRecord& citizen,
                                                              const CitizenLifeStageRules& rules) {
    CitizenCapabilitySummary summary{};
    summary.alive = citizen.biological.alive && citizen.lifeStage.stage != CitizenLifeStage::Deceased;
    summary.conscious = summary.alive && citizen.biological.conscious;
    const auto& stage = rules.forStage(citizen.lifeStage.stage);
    for (std::size_t i = 0; i < citizenCapabilityCount; ++i) {
        const float life = clamp01(stage[i]);
        const float base = clamp01(citizen.laborCapabilities.base[i]);
        const float injury = clamp01(citizen.impairment.multiplier[i]);
        summary.effective[i] = summary.alive ? base * life * injury : 0.0f;
    }
    return summary;
}

CitizenCapabilitySummary CitizenLifecycleSystem::capabilities(const RemoteCitizenRecord& citizen,
                                                              const CitizenLifeStageRules& rules) {
    CitizenRecord active = promoteFromRemote(citizen);
    return capabilities(active, rules);
}

bool CitizenLifecycleSystem::transitionLifeStage(CitizenRecord& citizen,
                                                 CitizenLifeStage newStage,
                                                 std::vector<CitizenLifecycleEvent>* events) {
    if (citizen.lifeStage.stage == CitizenLifeStage::Deceased) return false;
    if (citizen.lifeStage.stage == newStage) return false;
    if (newStage != CitizenLifeStage::Deceased &&
        stageIndex(newStage) < stageIndex(citizen.lifeStage.stage)) return false;
    const auto oldStage = citizen.lifeStage.stage;
    citizen.lifeStage.stage = newStage;
    if (newStage == CitizenLifeStage::Deceased) {
        citizen.biological.alive = false;
        citizen.biological.conscious = false;
    }
    if (events) events->push_back({CitizenLifecycleEventKind::LifeStageChanged,
                                   citizen.persistentId.value, 0, oldStage, newStage});
    return true;
}

void CitizenLifecycleSystem::synchronizeBiologicalFacts(CitizenRecord& citizen,
                                                        float nutrition01,
                                                        float rest01,
                                                        float respiration01,
                                                        bool conscious) {
    citizen.biological.nutrition01 = clamp01(nutrition01);
    citizen.biological.rest01 = clamp01(rest01);
    citizen.biological.respiration01 = clamp01(respiration01);
    citizen.biological.conscious = citizen.biological.alive && conscious;
}

CitizenRepresentation CitizenLifecycleSystem::remoteRepresentationFor(const CitizenRecord& citizen) {
    CitizenSubjectPersistenceTraits traits{};
    traits.citizen = true;
    traits.named = citizen.identity.named;
    traits.domestic = citizen.persistence.namedDomestic;
    traits.historicalSignificance = citizen.persistence.historicalSignificance;
    traits.ownsUniqueItem = !citizen.possessions.ownedUniqueItemIds.empty();
    traits.holdsOffice = !citizen.offices.officeIds.empty();
    if (!citizen.persistence.allowRemoteCohort || shouldPersistIndividually(traits))
        return CitizenRepresentation::RemoteIndividual;
    return CitizenRepresentation::RemoteCohortEligible;
}

RemoteCitizenRecord CitizenLifecycleSystem::compactForRemote(
    const CitizenRecord& citizen,
    std::int64_t strategicTime,
    CitizenSimulationShard targetShard) {
    if (targetShard == CitizenSimulationShard::DirectOperativeBubble ||
        targetShard == CitizenSimulationShard::ActiveFortress) {
        throw std::invalid_argument("remote compaction target must be a remote/strategic shard");
    }
    RemoteCitizenRecord remote = toRemote(citizen);
    remote.schemaVersion = kCitizenEntitySchemaVersion;
    remote.persistence.representation = remoteRepresentationFor(citizen);
    remote.persistence.lastStrategicUpdate = strategicTime;
    remote.shardState.shard = targetShard;
    ++remote.shardState.transitionSerial;
    return remote;
}

CitizenRecord CitizenLifecycleSystem::promoteFromRemote(
    const RemoteCitizenRecord& remote,
    CitizenSimulationShard targetShard) {
    if (remote.schemaVersion < kCitizenEntityOldestReadableSchemaVersion ||
        remote.schemaVersion > kCitizenEntitySchemaVersion)
        throw std::invalid_argument("unsupported remote citizen schema version");
    if (targetShard != CitizenSimulationShard::DirectOperativeBubble &&
        targetShard != CitizenSimulationShard::ActiveFortress) {
        throw std::invalid_argument("promotion target must be an active citizen shard");
    }
    CitizenRecord out{};
    out.persistentId = remote.persistentId;
    out.identity = remote.identity;
    out.membership = remote.membership;
    out.lifeStage = remote.lifeStage;
    out.biological = remote.biological;
    out.body = remote.body;
    out.profession = remote.profession;
    out.laborCapabilities = remote.laborCapabilities;
    out.impairment = remote.impairment;
    out.possessions = remote.possessions;
    out.offices = remote.offices;
    out.military = remote.military;
    out.biography = remote.biography;
    out.strategicLocation = remote.strategicLocation;
    out.shardState = remote.shardState;
    out.shardState.shard = targetShard;
    ++out.shardState.transitionSerial;
    out.persistence = remote.persistence;
    out.persistence.representation = CitizenRepresentation::Active;
    out.navigationIntent = {};
    return out;
}

CitizenDeathResult CitizenLifecycleSystem::transitionToDeath(CitizenRecord& citizen,
                                                             std::uint64_t eventSeed) {
    CitizenDeathResult result{};
    if (citizen.lifeStage.stage == CitizenLifeStage::Deceased || !citizen.biological.alive) return result;

    const auto prior = citizen.lifeStage.stage;
    citizen.lifeStage.stage = CitizenLifeStage::Deceased;
    citizen.biological.alive = false;
    citizen.biological.conscious = false;
    citizen.navigationIntent = {};
    citizen.persistence.historicalSignificance = true;
    citizen.persistence.allowRemoteCohort = false;

    result.transitioned = true;
    result.remainsStableId = deathRemainsStableId(citizen.persistentId.value, eventSeed);
    result.events.push_back({CitizenLifecycleEventKind::LifeStageChanged,
                             citizen.persistentId.value, 0, prior, CitizenLifeStage::Deceased});
    result.events.push_back({CitizenLifecycleEventKind::RemainsCreationRequested,
                             citizen.persistentId.value, result.remainsStableId, prior, CitizenLifeStage::Deceased});

    auto appendRefs = [&](CitizenLifecycleEventKind kind, std::vector<CitizenStableId> refs) {
        sortUniqueNonZero(refs);
        for (const auto target : refs)
            result.events.push_back({kind, citizen.persistentId.value, target, prior, CitizenLifeStage::Deceased});
    };
    appendRefs(CitizenLifecycleEventKind::PossessionDispositionRequested, citizen.possessions.ownedUniqueItemIds);
    appendRefs(CitizenLifecycleEventKind::OfficeVacancyRequested, citizen.offices.officeIds);
    appendRefs(CitizenLifecycleEventKind::RelationshipDeathNoticeRequested, citizen.biography.relationshipIds);
    appendRefs(CitizenLifecycleEventKind::MemoryDeathNoticeRequested, citizen.biography.importantMemoryIds);

    result.events.push_back({CitizenLifecycleEventKind::MemorialRequested,
                             citizen.persistentId.value, 0, prior, CitizenLifeStage::Deceased});
    result.events.push_back({CitizenLifecycleEventKind::ChronicleDeathRequested,
                             citizen.persistentId.value, 0, prior, CitizenLifeStage::Deceased});
    return result;
}

bool CitizenLifecycleSystem::shouldPersistIndividually(const CitizenSubjectPersistenceTraits& traits) {
    if (traits.named || traits.domestic || traits.historicalSignificance || traits.ownsUniqueItem || traits.holdsOffice) return true;
    // Active fortress citizens have durable identity locally, but remote ordinary
    // population may be cohort-compacted when no durable anchor requires a full
    // biography, matching Fourth Edition retirement/behavioral LOD rules.
    return false;
}

bool CitizenLifecycleSystem::validate(const CitizenRecord& citizen, std::string* error) {
    RemoteCitizenRecord remote = toRemote(citizen);
    return validateCommon(remote, error);
}

bool CitizenLifecycleSystem::validate(const RemoteCitizenRecord& citizen, std::string* error) {
    if (citizen.schemaVersion < kCitizenEntityOldestReadableSchemaVersion ||
        citizen.schemaVersion > kCitizenEntitySchemaVersion) {
        if (error) *error = "unsupported remote citizen schema version";
        return false;
    }
    return validateCommon(citizen, error);
}

bool durableCitizenStateEqual(const CitizenRecord& a, const CitizenRecord& b) {
    return durableFieldsEqual(a, toRemote(b));
}

bool durableCitizenStateEqual(const CitizenRecord& a, const RemoteCitizenRecord& b) {
    return durableFieldsEqual(a, b);
}

} // namespace elysium
