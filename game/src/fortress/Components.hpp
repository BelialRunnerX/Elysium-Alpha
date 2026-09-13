#pragma once

#include "fortress/Common.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace elysium::fortress {

struct PersistentIdentity {
    StableId stableId{};
    std::string name;
    SimulationShard shard{SimulationShard::ActiveFortress};
    std::uint32_t schemaVersion{1};
};

struct SettlementMembership {
    SiteId site{};
    CivilizationId civilization{};
    OrganizationId faction{};
    bool citizen{true};
    bool resident{true};
};

struct AgeLifeStage {
    float biologicalYears{};
    LifeStage stage{LifeStage::Adult};
    float speciesExpectedYears{80.0f};
};

struct BiologicalState {
    float hydration{1.0f};
    float nutrition{1.0f};
    float sleepDebt{};
    float oxygenation{1.0f};
    float bodyTemperature{1.0f};
    float pain{};
    float bloodFraction{1.0f};
    float immunity{1.0f};
};

struct Personality {
    float riskTolerance{0.5f};
    float sociability{0.5f};
    float patience{0.5f};
    float discipline{0.5f};
    float anxiety{0.5f};
    float anger{0.5f};
    float altruism{0.5f};
    float curiosity{0.5f};
    float orderliness{0.5f};
};

struct Values {
    float family{0.5f};
    float law{0.5f};
    float independence{0.5f};
    float empire{0.5f};
    float tradition{0.5f};
    float craft{0.5f};
    float knowledge{0.5f};
    float nature{0.5f};
    float wealth{0.5f};
    float martialHonor{0.5f};
};

struct PreferenceEntry {
    ContentId subject;
    float affinity{};
};

struct Preferences {
    std::vector<PreferenceEntry> entries;
};

enum class NeedKind : std::uint8_t {
    Sleep,
    Food,
    Safety,
    Social,
    Purpose,
    Worship,
    Creativity,
    Learning,
    Family,
    Solitude,
    Excitement
};

struct NeedState {
    NeedKind kind{NeedKind::Sleep};
    float satisfaction{1.0f};
    float urgency{};
    float decayPerDay{0.05f};
};

struct Needs {
    std::vector<NeedState> states;
};

struct Thought {
    std::string key;
    float valence{};
    float intensity{};
    TimeStamp created{};
    StableId subject{};
};

struct CurrentThoughts {
    std::vector<Thought> thoughts;
};

struct Memory {
    std::string key;
    float valence{};
    float strength{};
    float trauma{};
    TimeStamp occurred{};
    StableId subject{};
    HistoricalEventId event{};
};

struct Memories {
    std::vector<Memory> entries;
};

struct FocusState {
    float current{1.0f};
    float workMultiplier{1.0f};
    float combatMultiplier{1.0f};
};

struct StressState {
    float load{};
    float recoveryReserve{0.5f};
    StressBand band{StressBand::Stable};
    std::vector<std::string> topReasons;
};

enum class RelationshipType : std::uint16_t {
    Parent = 1 << 0,
    Child = 1 << 1,
    Sibling = 1 << 2,
    Partner = 1 << 3,
    Friend = 1 << 4,
    Rival = 1 << 5,
    Mentor = 1 << 6,
    Commander = 1 << 7,
    Coworker = 1 << 8,
    Creditor = 1 << 9,
    FaithPeer = 1 << 10,
    Guardian = 1 << 11
};

struct RelationshipEdge {
    StableId source{};
    StableId target{};
    std::uint16_t typeMask{};
    float affinity{};
    float trust{};
    float respect{};
    float fear{};
    float grievance{};
    float familiarity{};
    TimeStamp lastInteraction{};
    std::vector<HistoricalEventId> eventRefs;
};

struct SkillEntry {
    ContentId skill;
    float experience{};
    float aptitude{1.0f};
    float effectiveRank{};
};

struct Skills {
    std::vector<SkillEntry> entries;
};

struct Profession {
    ContentId profession;
    std::string displayName;
};

enum class LaborPolicy : std::uint8_t {
    Unrestricted,
    Preferred,
    Required,
    Forbidden
};

struct WorkDetailEntry {
    ContentId labor;
    LaborPolicy policy{LaborPolicy::Unrestricted};
};

struct WorkDetails {
    std::vector<WorkDetailEntry> entries;
};

struct ScheduleBlock {
    std::uint8_t startHour{};
    std::uint8_t endHour{24};
    ContentId activity;
    ZoneId zone{};
};

struct Schedule {
    std::vector<ScheduleBlock> blocks;
};

struct CurrentJob {
    JobId job{};
};

struct JobHistoryEntry {
    JobId job{};
    ContentId type;
    JobState result{JobState::Completed};
    TimeStamp ended{};
};

struct JobHistory {
    std::vector<JobHistoryEntry> entries;
};

struct JobRequirements {
    ContentId labor;
    float minimumSkill{};
    ContentId toolTag;
    AccessPolicy access{AccessPolicy::Open};
    bool requiresBreathableAtmosphere{};
    bool requiresMobility{true};
};

struct ReservationClaim {
    ReservationKind kind{ReservationKind::Item};
    StableId resource{};
    StableId owner{};
    JobId job{};
    float quantity{1.0f};
    std::int64_t expiresTick{};
    std::string cancellationReason;
};

struct JobComponent {
    JobId id{};
    ContentId type;
    PriorityBand priority{PriorityBand::Normal};
    StableId origin{};
    SpatialAnchor target{};
    JobRequirements requirements{};
    std::vector<ContentId> materialFilters;
    std::vector<ReservationClaim> reservations;
    StableId worker{};
    std::vector<JobId> dependencies;
    float progress{};
    float workRequired{1.0f};
    JobState state{JobState::Pending};
    bool interruptible{true};
    std::string failureReason;
    bool recordHistory{};
};

struct WorkOrderCondition {
    ContentId stockTag;
    float minimumStock{};
    float maximumStock{std::numeric_limits<float>::max()};
    float minimumPowerReserve{};
    float maximumHazard{1.0f};
    float maximumSuspicion{100.0f};
    std::vector<WorkOrderId> dependencies;
};

struct WorkOrderComponent {
    WorkOrderId id{};
    ContentId recipeOrJob;
    WorkOrderMode mode{WorkOrderMode::ProduceQuantity};
    WorkOrderFrequency frequency{WorkOrderFrequency::Once};
    float targetQuantity{1.0f};
    float lowerBound{};
    float upperBound{};
    StableId workshopScope{};
    SiteId siteScope{};
    WorkOrderCondition condition{};
    bool enabled{true};
    bool conditionWasTrue{};
    float repeatIntervalSeconds{};
    double cooldown{};
    std::string diagnostic;
};

struct DesignationComponent {
    DesignationId id{};
    DesignationKind kind{DesignationKind::Mine};
    SpatialExtent extent{};
    StableId owner{};
    PriorityBand priority{PriorityBand::Normal};
    ContentId materialPolicy;
    AccessPolicy access{AccessPolicy::Open};
    bool generatesJobs{true};
    bool suspended{};
};

struct RoomQuality {
    float area{};
    float materialValue{};
    float craftsmanship{};
    float cleanliness{1.0f};
    float decor{};
    float privacy{};
    float noise{};
    float comfort{0.5f};
    float safety{0.5f};
    float view{};
    float culturalMatch{0.5f};
    float composite{};
};

struct RoomComponent {
    RoomId id{};
    RoomKind kind{RoomKind::Bedroom};
    ContentId spatialMask;
    StableId owner{};
    InstitutionId institution{};
    AccessPolicy access{AccessPolicy::Open};
    RoomQuality quality{};
    bool recognized{};
    bool sealed{};
};

struct ZoneComponent {
    ZoneId id{};
    ZoneKind kind{ZoneKind::General};
    ContentId spatialMask;
    AccessPolicy access{AccessPolicy::Open};
    float trafficCost{1.0f};
    bool enabled{true};
};

enum class ItemIdentityTier : std::uint8_t {
    BulkAggregate,
    Stack,
    Individual,
    NamedOwned,
    Artifact
};

struct ItemState {
    StableId id{};
    ContentId item;
    ContentId material;
    ItemIdentityTier identityTier{ItemIdentityTier::Stack};
    float quantity{1.0f};
    float quality{};
    float condition{1.0f};
    float contamination{};
    float massPerUnit{1.0f};
    StableId owner{};
    StableId container{};
    SpatialAnchor location{};
    bool forbidden{};
    bool reserved{};
    bool tradeLocked{};
};

struct OwnershipClaim {
    StableId subject{};
    StableId owner{};
    ContentId reason;
    TimeStamp since{};
};

struct ArtifactState {
    ArtifactId artifact{};
    StableId item{};
    std::string name;
    StableId creator{};
    ContentId motif;
    float value{};
    bool indestructibleByOrdinaryMeans{};
    std::vector<HistoricalEventId> provenance;
};

struct ItemFilter {
    std::vector<ContentId> categories;
    std::vector<ContentId> materials;
    float minQuality{};
    float maxContamination{1.0f};
    bool allowOwned{true};
    bool allowArtifacts{};
};

struct StockpileComponent {
    StockpileId id{};
    ContentId spatialMask;
    ItemFilter filter{};
    PriorityBand priority{PriorityBand::Normal};
    float capacity{100.0f};
    float used{};
    std::vector<StockpileId> giveLinks;
    std::vector<StockpileId> takeLinks;
    ContentId containerPolicy;
    ContentId vehiclePolicy;
    ContentId hazardPolicy;
    StockpileId overflow{};
};

struct InventoryPort {
    ContentId name;
    ContentId filter;
    float capacity{};
    bool input{};
    bool output{};
};

struct MachineState {
    StableId id{};
    ContentId type;
    ContentId domain;
    int tier{};
    std::vector<InventoryPort> ports;
    float powerGeneration{};
    float powerDraw{};
    std::uint8_t powerPriority{4};
    float condition{1.0f};
    float contamination{};
    float heat{};
    bool enabled{true};
    bool powered{};
    bool faulted{};
    ContentId activeProcess;
    float processProgress{};
};

struct RecipeProcess {
    ContentId recipe;
    std::vector<std::pair<ContentId, float>> inputs;
    std::vector<std::pair<ContentId, float>> outputs;
    float work{};
    float power{};
    float heat{};
    ContentId requiredLabor;
    float difficulty{};
};

struct MaintenanceState {
    float condition{1.0f};
    float wearRate{0.001f};
    float contaminationPenalty{};
    float environmentPenalty{};
    float warningThreshold{0.4f};
    float failureThreshold{0.1f};
    TimeStamp lastService{};
    bool serviceRequested{};
};

struct CropState {
    StableId id{};
    ContentId species;
    float growth{};
    float water{1.0f};
    float nutrients{1.0f};
    float light{1.0f};
    float temperatureSuitability{1.0f};
    float diseasePressure{};
    float pestPressure{};
    bool mature{};
};

struct LivestockState {
    StableId id{};
    ContentId archetype;
    float hunger{};
    float health{1.0f};
    float productProgress{};
    float tame{1.0f};
    StableId geneticLine{};
    ZoneId pasture{};
};

struct RoomAtmosphere {
    RoomId room{};
    float pressure{1.0f};
    float oxygen{0.21f};
    float smoke{};
    float toxins{};
    float humidity{0.4f};
    float temperature{0.5f};
    bool connectedToSky{};
    bool sealed{};
    bool boundedFillExhausted{};
};

struct FluidVolumeState {
    StableId id{};
    MediumKind medium{MediumKind::Water};
    float amount{};
    float pressure{};
    float temperature{0.5f};
    float contamination{};
    ContentId spatialMask;
};

struct FireState {
    StableId id{};
    float intensity{};
    float fuel{};
    float oxygenDemand{};
    float smokeRate{};
    float heatRate{};
    ContentId spatialMask;
};

struct ContaminationState {
    StableId id{};
    ContentId contaminant;
    float intensity{};
    ContentId carrier;
    ContentId spatialMask;
};

struct BodyPartState {
    ContentId part;
    ContentId parent;
    float integrity{1.0f};
    float function{1.0f};
    float bleeding{};
    float pain{};
    bool missing{};
};

struct WoundState {
    StableId id{};
    ContentId bodyPart;
    ContentId woundType;
    InjurySeverity severity{InjurySeverity::Minor};
    float bleeding{};
    float pain{};
    float contamination{};
    float infectionRisk{};
    float healing{};
    StableId causedBy{};
};

struct BodyState {
    ContentId plan;
    std::vector<BodyPartState> parts;
    std::vector<WoundState> wounds;
    float bloodFraction{1.0f};
    float oxygenation{1.0f};
    float mobility{1.0f};
    float manipulation{1.0f};
    float consciousness{1.0f};
};

struct TreatmentOrder {
    StableId patient{};
    StableId wound{};
    TreatmentKind treatment{TreatmentKind::Diagnose};
    PriorityBand priority{PriorityBand::Normal};
    ContentId supply;
    ContentId skill;
    float work{};
    bool complete{};
};

struct ProstheticState {
    ContentId bodyPart;
    ContentId device;
    float fitQuality{};
    float functionRestored{};
    float maintenance{};
};

struct SquadMember {
    StableId citizen{};
    ContentId role;
    bool mandatory{};
};

struct SquadState {
    SquadId id{};
    std::string name;
    StableId commander{};
    std::vector<SquadMember> members;
    ContentId uniformProfile;
    ZoneId rallyZone{};
    AlertLevel alert{AlertLevel::Green};
    float readiness{};
    float training{};
    float ammoFraction{1.0f};
};

struct SecurityCredential {
    StableId holder{};
    ContentId credential;
    std::uint32_t clearance{};
    TimeStamp expires{};
};

struct CrimeRecord {
    StableId perpetrator{};
    StableId victim{};
    CrimeKind kind{CrimeKind::Theft};
    float severity{};
    SpatialAnchor location{};
    TimeStamp occurred{};
    std::vector<StableId> witnesses;
    std::vector<StableId> evidence;
};

struct JusticeCase {
    CaseId id{};
    CrimeRecord crime{};
    StableId accused{};
    float evidenceStrength{};
    float confidence{};
    ContentId status;
    ContentId sentence;
    StableId investigator{};
    StableId judge{};
};

struct OfficeState {
    ContentId office;
    StableId holder{};
    SiteId jurisdiction{};
    float prestige{};
    std::vector<ContentId> mandates;
    std::vector<ContentId> requirements;
};

struct InstitutionState {
    InstitutionId id{};
    ContentId type;
    std::string name;
    SiteId jurisdiction{};
    StableId owner{};
    std::vector<RoomId> rooms;
    std::vector<StableId> staff;
    std::vector<StableId> members;
    std::vector<ContentId> supplies;
    float quality{};
    float prestige{};
    AccessPolicy access{AccessPolicy::Open};
    std::vector<ScheduleBlock> schedule;
    bool recordHistory{};
};

struct MigrationCandidate {
    StableId person{};
    SiteId origin{};
    SiteId destination{};
    float attraction{};
    float fear{};
    float kinship{};
    float opportunity{};
    float ideologicalFit{};
    bool visitor{};
};

struct MarketGood {
    ContentId good;
    float supply{};
    float demand{};
    float basePrice{1.0f};
    float price{1.0f};
};

struct MarketState {
    SiteId site{};
    std::vector<MarketGood> goods;
    float liquidity{1.0f};
    float riskPremium{};
};

struct ContractState {
    ContractId id{};
    ContractType type{ContractType::Survey};
    ContractScope scope{ContractScope::Local};
    OrganizationId issuer{};
    StableId assignee{};
    SiteId targetSite{};
    ContentId objective;
    ContentId complication;
    float credits{};
    float standingReward{};
    TimeStamp deadline{};
    float progress{};
    bool accepted{};
    bool completed{};
    bool failed{};
};

struct CaravanState {
    StableId id{};
    OrganizationId owner{};
    SiteId origin{};
    SiteId destination{};
    std::vector<std::pair<ContentId, float>> cargo;
    float capacity{};
    float progress{};
    float risk{};
    float escortStrength{};
};

struct DiplomacyState {
    CivilizationId a{};
    CivilizationId b{};
    float trust{};
    float fear{};
    float tradeAffinity{};
    float grievance{};
    bool atWar{};
    bool embargo{};
    bool treaty{};
};

struct StandingState {
    float favor{};
    std::unordered_map<std::uint64_t, float> suspicionBySystem;
};

struct ClaimState {
    SiteId site{};
    std::uint64_t system{};
    StableId beacon{};
    float suspicionFloor{};
    bool active{};
    bool contested{};
};

struct ThreatState {
    StableId id{};
    ThreatKind kind{ThreatKind::Wildlife};
    SiteId target{};
    OrganizationId faction{};
    float strength{};
    float escalation{};
    AlertLevel alert{AlertLevel::Yellow};
    TimeStamp announced{};
    TimeStamp arrival{};
    bool active{};
    bool defeated{};
};

struct RiftHorrorState {
    StableId id{};
    std::string generatedName;
    std::vector<ContentId> bodyTraits;
    std::vector<ContentId> attacks;
    std::vector<ContentId> immunities;
    std::vector<ContentId> syndromes;
    float scale{1.0f};
    float intelligence{0.5f};
    float instability{0.5f};
};

struct SyndromeState {
    StableId id{};
    ContentId syndrome;
    StableId host{};
    float incubation{};
    float progress{};
    float contagiousness{};
    float severity{};
    bool quarantined{};
};

struct KnowledgeState {
    StableId owner{};
    std::vector<std::pair<ContentId, float>> fields;
    std::vector<ContentId> discoveries;
    std::vector<ContentId> authoredWorks;
};

struct ResearchProject {
    StableId id{};
    ContentId field;
    ContentId objective;
    float progress{};
    float difficulty{1.0f};
    std::vector<StableId> researchers;
    std::vector<ContentId> samples;
    bool completed{};
};

struct AethericObsession {
    StableId citizen{};
    ContentId demandedWorkshop;
    std::vector<ContentId> demandedMaterials;
    std::vector<StableId> reservedItems;
    float urgency{};
    float progress{};
    bool resolved{};
    ArtifactId result{};
};

struct ScanRecord {
    StableId scanner{};
    ContentId targetType;
    StableId target{};
    ContentId discovery;
    float completeness{};
    TimeStamp discovered{};
    bool filedEmpire{};
    bool sharedUnsworn{};
};

struct DirectOperativeState {
    StableId citizen{};
    bool playerControlled{};
    bool commandLinkActive{};
    SiteId activeSite{};
    ContentId objective;
    AlertLevel tacticalAlert{AlertLevel::Green};
};

struct VehicleState {
    VehicleId id{};
    ContentId archetype;
    StableId owner{};
    float cargoMass{};
    float cargoCapacity{};
    float power{};
    float fuel{};
    float hull{1.0f};
    float mobility{1.0f};
    ContentId route;
    SiteId destination{};
    bool crewed{};
    bool autonomous{};
};

struct ShipState {
    StableId id{};
    StableId owner{};
    float cargoSlots{12.0f};
    float cargoMass{};
    float jumpRangeLy{100.0f};
    float fuelCells{4.0f};
    float hull{200.0f};
    float atmosphericHandling{1.0f};
    float scannerGrade{1.0f};
    std::vector<ContentId> modules;
    std::uint64_t currentSystem{};
    std::uint64_t destinationSystem{};
    float routeProgress{};
};

struct OrbitalSiteState {
    SiteId id{};
    ContentId type;
    std::uint64_t system{};
    StableId owner{};
    float storage{};
    float production{};
    float defense{};
    float visibility{};
};

struct PowerNode {
    StableId id{};
    float generation{};
    float demand{};
    float storage{};
    float storageCapacity{};
    std::uint8_t priority{4};
    bool isolated{};
    bool powered{};
};

struct AutomationSignal {
    ContentId name;
    float numeric{};
    bool boolean{};
};

struct AutomationRule {
    StableId id{};
    ContentId trigger;
    ContentId condition;
    ContentId action;
    float threshold{};
    StableId target{};
    bool enabled{true};
    bool latched{};
};

struct RailCartState {
    StableId id{};
    ContentId route;
    float mass{};
    float capacity{};
    float speed{};
    float brakingDistance{};
    float progress{};
    bool derailed{};
    std::vector<std::pair<ContentId, float>> manifest;
};

struct BlueprintStep {
    DesignationKind kind{DesignationKind::Build};
    SpatialExtent relativeExtent{};
    ContentId material;
    ContentId prefab;
    float quantity{1.0f};
};

struct ConstructionBlueprint {
    BlueprintId id{};
    std::string name;
    std::vector<BlueprintStep> steps;
    std::vector<ContentId> utilityPorts;
    RoomKind intendedRoom{RoomKind::Workshop};
};

struct SupportIsland {
    StableId id{};
    std::vector<CellAddress> cells;
    float supportCapacity{};
    float load{};
    bool rooted{};
    bool dirty{};
};

struct SiteState {
    SiteId id{};
    std::string name;
    ContentId type;
    CivilizationId civilization{};
    OrganizationId government{};
    std::uint64_t system{};
    std::uint64_t planet{};
    float population{};
    float foodReserve{};
    float industrialCapacity{};
    float militaryStrength{};
    float contamination{};
    float prosperity{};
    float stability{1.0f};
    bool activeFortress{};
    bool abandoned{};
    bool ruined{};
};

struct CivilizationState {
    CivilizationId id{};
    std::string name;
    ContentId culture;
    ContentId government;
    ContentId faith;
    float population{};
    float technology{};
    float wealth{};
    float militaryStrength{};
    std::vector<SiteId> sites;
    std::vector<OrganizationId> organizations;
};

struct OrganizationState {
    OrganizationId id{};
    std::string name;
    ContentId purpose;
    CivilizationId civilization{};
    SiteId headquarters{};
    float wealth{};
    float influence{};
    float militaryStrength{};
    std::vector<StableId> members;
};

struct HistoricalEvent {
    HistoricalEventId id{};
    HistoricalEventKind kind{HistoricalEventKind::Discovery};
    TimeStamp time{};
    StableId primary{};
    StableId secondary{};
    SiteId site{};
    ArtifactId artifact{};
    OrganizationId organization{};
    CivilizationId civilization{};
    std::string summary;
    float significance{};
};

struct HistorySignificance {
    float score{};
    bool historicalFigure{};
    bool forcePersist{};
};

struct PersistencePolicy {
    bool persistIdentity{true};
    bool persistRelationships{true};
    bool persistWounds{true};
    bool persistInventoryUnique{true};
    bool persistMemories{true};
    bool compactWhenRemote{true};
};

struct InspectorFact {
    std::string category;
    std::string reason;
    float weight{};
    StableId subject{};
};

struct InspectorTrace {
    StableId subject{};
    std::string decision;
    std::vector<InspectorFact> facts;
};

} // namespace elysium::fortress
