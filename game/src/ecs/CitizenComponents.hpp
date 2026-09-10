// Intended function: imported ecs implementation for CitizenComponents; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace elysium {

// Merge note: the standalone v0.20 tree predates the shared EntityStableId type
// owned by the foundation/persistence stream. Citizen-facing APIs therefore use
// a dedicated durable integer alias. It is never an entt::entity or container
// index and is intended to map 1:1 to the running build's canonical stable ID.
using CitizenStableId = std::uint64_t;

enum class CitizenshipStatus : std::uint8_t {
    Visitor = 0,
    GuestWorker,
    LongTermResident,
    Citizen,
    ProtectedRefugee,
    Restricted,
    Exiled
};

enum class CitizenLifeStage : std::uint8_t {
    Dependent = 0,
    Trainee,
    Adult,
    Elder,
    Deceased
};

enum class CitizenRepresentation : std::uint8_t {
    Active = 0,
    RemoteIndividual,
    RemoteCohortEligible
};

// Fourth Edition Part 3 simulation domains. This is durable behavioral-LOD
// metadata, not a renderer/streaming residency enum. A citizen may change shard
// without changing StableId.
enum class CitizenSimulationShard : std::uint8_t {
    DirectOperativeBubble = 0,
    ActiveFortress,
    ActivePlanetRemote,
    StarSystemStrategic,
    GalaxyHistory
};

enum class CitizenCapability : std::uint8_t {
    Walk = 0,
    Grasp,
    See,
    Breathe,
    Speak,
    ExtractionConstruction,
    MetallurgyFabrication,
    UtilitiesMaintenance,
    AgricultureFood,
    Medicine,
    Knowledge,
    AdministrationSocial,
    Combat,
    ArtCulture,
    Count
};

constexpr std::size_t citizenCapabilityCount = static_cast<std::size_t>(CitizenCapability::Count);
using CitizenCapabilityValues = std::array<float, citizenCapabilityCount>;

struct PersistentIdComponent {
    CitizenStableId value{};
};

struct CitizenIdentityComponent {
    std::string name;
    std::string speciesRef;
    std::string raceRef;
    bool named{true};
};

struct CitizenMembershipComponent {
    CitizenStableId homeSettlementId{};
    CitizenStableId currentSettlementId{};
    CitizenStableId primaryFactionId{};
    CitizenshipStatus citizenship{CitizenshipStatus::Citizen};
    std::vector<CitizenStableId> organizationIds;
};

struct AgeLifeStageComponent {
    double ageYears{};
    std::int64_t birthTime{};
    CitizenLifeStage stage{CitizenLifeStage::Adult};
};

// These are bridge facts populated by the survival/environment systems. This
// subsystem deliberately owns no drain rates, hazard formulas, or metabolism.
struct BiologicalStateComponent {
    float nutrition01{1.0f};
    float rest01{1.0f};
    float respiration01{1.0f};
    bool conscious{true};
    bool alive{true};
};

struct CitizenBodyReferenceComponent {
    std::string bodyPlanRef;
    CitizenStableId bodyStateRef{};
};

struct CitizenProfessionComponent {
    std::string professionRef;
};

// Base capability is semantic eligibility/function, not learned skill rank.
// Agent 11/19 systems can combine this result with skills/work-detail policy.
struct LaborCapabilitiesComponent {
    CitizenCapabilityValues base{};
};

// Written by medicine/body systems as a bounded functional bridge. A value of
// 1 means unimpaired; 0 means the function is unavailable. This avoids making
// citizen life-cycle code understand wound names or prosthetic implementations.
struct FunctionalImpairmentComponent {
    CitizenCapabilityValues multiplier{};
};

struct CitizenPossessionLinksComponent {
    std::vector<CitizenStableId> ownedUniqueItemIds;
    std::vector<CitizenStableId> equippedItemIds;
    CitizenStableId quartersId{};
};

struct CitizenOfficeLinksComponent {
    std::vector<CitizenStableId> officeIds;
};

struct CitizenMilitaryAssignmentComponent {
    CitizenStableId squadId{};
    CitizenStableId uniformProfileId{};
};

// Opaque durable endpoints only. Relationship scoring/memory content belongs to
// Agents 18+; Agent 17 needs the references so life-cycle transitions can emit
// deterministic downstream notifications without destroying biography.
struct CitizenBiographyLinksComponent {
    std::vector<CitizenStableId> relationshipIds;
    std::vector<CitizenStableId> importantMemoryIds;
    std::vector<CitizenStableId> historyEventIds;
};

struct CitizenStrategicLocationComponent {
    CitizenStableId systemId{};
    CitizenStableId planetId{};
    CitizenStableId siteId{};
};

struct CitizenShardStateComponent {
    CitizenSimulationShard shard{CitizenSimulationShard::ActiveFortress};
    std::uint64_t transitionSerial{};
};

struct CitizenNavigationIntentComponent {
    CitizenStableId targetStableId{};
    std::uint32_t intentCode{};
    bool hasIntent{};
};

struct CitizenPersistencePolicyComponent {
    CitizenRepresentation representation{CitizenRepresentation::Active};
    bool historicalSignificance{};
    bool namedDomestic{};
    bool allowRemoteCohort{true};
    std::int64_t lastStrategicUpdate{};
};

struct CitizenRecord {
    PersistentIdComponent persistentId;
    CitizenIdentityComponent identity;
    CitizenMembershipComponent membership;
    AgeLifeStageComponent lifeStage;
    BiologicalStateComponent biological;
    CitizenBodyReferenceComponent body;
    CitizenProfessionComponent profession;
    LaborCapabilitiesComponent laborCapabilities;
    FunctionalImpairmentComponent impairment;
    CitizenPossessionLinksComponent possessions;
    CitizenOfficeLinksComponent offices;
    CitizenMilitaryAssignmentComponent military;
    CitizenBiographyLinksComponent biography;
    CitizenStrategicLocationComponent strategicLocation;
    CitizenShardStateComponent shardState;
    CitizenPersistencePolicyComponent persistence;

    // Active-only/cache state. It is intentionally omitted from remote records.
    CitizenNavigationIntentComponent navigationIntent;
};

struct RemoteCitizenRecord {
    std::uint32_t schemaVersion{2};
    PersistentIdComponent persistentId;
    CitizenIdentityComponent identity;
    CitizenMembershipComponent membership;
    AgeLifeStageComponent lifeStage;
    BiologicalStateComponent biological;
    CitizenBodyReferenceComponent body;
    CitizenProfessionComponent profession;
    LaborCapabilitiesComponent laborCapabilities;
    FunctionalImpairmentComponent impairment;
    CitizenPossessionLinksComponent possessions;
    CitizenOfficeLinksComponent offices;
    CitizenMilitaryAssignmentComponent military;
    CitizenBiographyLinksComponent biography;
    CitizenStrategicLocationComponent strategicLocation;
    CitizenShardStateComponent shardState;
    CitizenPersistencePolicyComponent persistence;
};

struct CitizenCapabilitySummary {
    CitizenCapabilityValues effective{};
    bool alive{};
    bool conscious{};

    float value(CitizenCapability capability) const {
        return effective[static_cast<std::size_t>(capability)];
    }

    bool available(CitizenCapability capability, float minimum = 0.001f) const {
        return value(capability) >= minimum;
    }
};

struct CitizenSubjectPersistenceTraits {
    bool citizen{};
    bool named{};
    bool domestic{};
    bool historicalSignificance{};
    bool ownsUniqueItem{};
    bool holdsOffice{};
};

} // namespace elysium
