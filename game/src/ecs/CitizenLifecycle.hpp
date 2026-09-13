// Intended function: imported ecs implementation for CitizenLifecycle; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "ecs/CitizenComponents.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium {

struct CitizenLifeStageRules {
    std::array<CitizenCapabilityValues, 5> stageMultiplier{};

    static CitizenLifeStageRules conservativeDefaults();
    const CitizenCapabilityValues& forStage(CitizenLifeStage stage) const;
};

enum class CitizenLifecycleEventKind : std::uint8_t {
    LifeStageChanged = 0,
    RemainsCreationRequested,
    PossessionDispositionRequested,
    OfficeVacancyRequested,
    RelationshipDeathNoticeRequested,
    MemoryDeathNoticeRequested,
    MemorialRequested,
    ChronicleDeathRequested
};

struct CitizenLifecycleEvent {
    CitizenLifecycleEventKind kind{CitizenLifecycleEventKind::LifeStageChanged};
    CitizenStableId subjectId{};
    CitizenStableId targetId{};
    CitizenLifeStage priorStage{CitizenLifeStage::Adult};
    CitizenLifeStage newStage{CitizenLifeStage::Adult};
};

struct CitizenDeathResult {
    bool transitioned{};
    CitizenStableId remainsStableId{};
    std::vector<CitizenLifecycleEvent> events;
};

enum class CitizenEntryKind : std::uint8_t {
    Arrival = 0,
    Birth,
    Creation
};

struct CitizenCreationSpec {
    CitizenStableId stableId{};
    CitizenEntryKind entryKind{CitizenEntryKind::Arrival};
    CitizenIdentityComponent identity;
    CitizenMembershipComponent membership;
    AgeLifeStageComponent lifeStage;
    CitizenBodyReferenceComponent body;
    CitizenProfessionComponent profession;
    LaborCapabilitiesComponent laborCapabilities;
    CitizenStrategicLocationComponent strategicLocation;
    CitizenShardStateComponent shardState;
    CitizenPersistencePolicyComponent persistence;
};

struct FounderRoleDefinition {
    std::string professionRef;
    CitizenCapabilityValues baseCapabilities{};
};

struct FoundingExpeditionSpec {
    std::uint64_t campaignSeed{};
    CitizenStableId settlementId{};
    CitizenStableId factionId{};
    std::string speciesRef;
    std::string raceRef;
    std::string bodyPlanRef;
    std::size_t founderCount{7};
    std::vector<std::string> founderNames;
    std::vector<FounderRoleDefinition> roles;
    std::vector<CitizenStableId> culturalOrganizationIds;
    std::vector<std::string> survivalSupplyContentIds;
    std::vector<std::string> toolContentIds;
};

struct FoundingExpeditionResult {
    std::vector<CitizenRecord> founders;
    std::vector<std::string> survivalSupplyContentIds;
    std::vector<std::string> toolContentIds;
};

class CitizenLifecycleSystem {
public:
    static CitizenCapabilityValues fullCapabilityValues(float value = 1.0f);
    static std::vector<FounderRoleDefinition> defaultFounderRoles();

    static CitizenRecord createCitizen(const CitizenCreationSpec& spec);
    static FoundingExpeditionResult createFoundingExpedition(const FoundingExpeditionSpec& spec);

    static CitizenCapabilitySummary capabilities(const CitizenRecord& citizen,
                                                  const CitizenLifeStageRules& rules);
    static CitizenCapabilitySummary capabilities(const RemoteCitizenRecord& citizen,
                                                  const CitizenLifeStageRules& rules);

    static bool transitionLifeStage(CitizenRecord& citizen,
                                    CitizenLifeStage newStage,
                                    std::vector<CitizenLifecycleEvent>* events = nullptr);

    // Does not calculate metabolism. It only accepts already-resolved facts from
    // the survival/atmosphere layer so citizen/job systems can inspect them.
    static void synchronizeBiologicalFacts(CitizenRecord& citizen,
                                           float nutrition01,
                                           float rest01,
                                           float respiration01,
                                           bool conscious);

    static RemoteCitizenRecord compactForRemote(
        const CitizenRecord& citizen,
        std::int64_t strategicTime,
        CitizenSimulationShard targetShard = CitizenSimulationShard::ActivePlanetRemote);
    static CitizenRecord promoteFromRemote(
        const RemoteCitizenRecord& remote,
        CitizenSimulationShard targetShard = CitizenSimulationShard::ActiveFortress);

    static CitizenDeathResult transitionToDeath(CitizenRecord& citizen,
                                                std::uint64_t eventSeed);

    static CitizenRepresentation remoteRepresentationFor(const CitizenRecord& citizen);
    static bool shouldPersistIndividually(const CitizenSubjectPersistenceTraits& traits);

    static bool validate(const CitizenRecord& citizen, std::string* error = nullptr);
    static bool validate(const RemoteCitizenRecord& citizen, std::string* error = nullptr);

private:
    static CitizenStableId founderStableId(const FoundingExpeditionSpec& spec, std::size_t ordinal);
    static CitizenStableId deathRemainsStableId(CitizenStableId citizenId, std::uint64_t eventSeed);
};

bool durableCitizenStateEqual(const CitizenRecord& a, const CitizenRecord& b);
bool durableCitizenStateEqual(const CitizenRecord& a, const RemoteCitizenRecord& b);

} // namespace elysium
