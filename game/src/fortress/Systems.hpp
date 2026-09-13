#pragma once

#include "fortress/Commands.hpp"
#include "fortress/ContentCatalog.hpp"

#include <functional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace elysium::fortress {

struct JobScoreInput {
    const PersistentIdentity* identity{};
    const BiologicalState* biology{};
    const StressState* stress{};
    const Skills* skills{};
    const WorkDetails* workDetails{};
    const Schedule* schedule{};
    SpatialAnchor workerLocation{};
    float estimatedDistance{};
    float hazard{};
    std::uint8_t hour{};
};

struct JobScoreResult {
    bool eligible{};
    float score{};
    std::string reason;
};

struct FortressStockSnapshot {
    std::unordered_map<std::string, float> quantities;
    float powerReserve{};
    float hazard{};
    float suspicion{};
};

struct PowerAllocationResult {
    float generated{};
    float demand{};
    float served{};
    float storedDelta{};
    std::vector<StableId> shedNodes;
};

struct HaulCandidate {
    StableId item{};
    StockpileId stockpile{};
    float score{};
};

struct TreatmentPlan {
    StableId patient{};
    std::vector<TreatmentOrder> orders;
    PriorityBand triage{PriorityBand::Normal};
};

struct MigrationDecision {
    bool migrate{};
    float score{};
    std::string reason;
};

struct ContractGenerationContext {
    std::uint64_t seed{};
    std::uint64_t epoch{};
    OrganizationId issuer{};
    SiteId target{};
    float hazard{};
    float suspicion{};
    float wealth{};
};

struct RemoteSiteDelta {
    SiteId site{};
    float populationDelta{};
    float foodDelta{};
    float industryDelta{};
    float wealthDelta{};
    float stabilityDelta{};
};

struct AlertRecord {
    AlertLevel level{AlertLevel::Green};
    std::string category;
    std::string title;
    std::string reason;
    StableId subject{};
    SiteId site{};
    float urgency{};
};

// Citizen / mind / life-cycle.
void advanceBiology(BiologicalState& state, float days, bool sleeping, bool eating, float atmosphereOxygen);
void advanceNeeds(Needs& needs, float days, const Personality& personality, const Values& values);
StressBand classifyStress(float load);
void updateStress(StressState& stress, const Needs& needs, const Memories& memories,
                  const Personality& personality, float safety, float treatment, float days);
void updateFocus(FocusState& focus, const Needs& needs, const StressState& stress,
                 const BiologicalState& biology);
LifeStage classifyLifeStage(float years, float expectedYears);
float learningGain(float difficulty, float currentRank, float aptitude, float focus, float mentorship);

// Relationships / generations.
float relationshipInteractionDelta(const RelationshipEdge& edge, const Thought& interaction,
                                   const Personality& personality);
void decayRelationship(RelationshipEdge& edge, float days);

// Jobs, work details, work orders and reservations.
JobScoreResult scoreJob(const JobComponent& job, const JobScoreInput& worker);
bool reservationCompatible(const ReservationClaim& existing, const ReservationClaim& incoming);
bool workOrderConditionMet(const WorkOrderComponent& order, const FortressStockSnapshot& stock);
std::vector<CreateJobCommand> evaluateWorkOrder(const WorkOrderComponent& order,
                                                const FortressStockSnapshot& stock,
                                                std::uint64_t seed,
                                                std::uint64_t tick);
std::vector<CreateJobCommand> jobsFromDesignation(const DesignationComponent& designation,
                                                  std::uint64_t seed,
                                                  std::uint64_t tick,
                                                  std::uint32_t maxJobs = 64);

// Rooms, stockpiles and logistics.
float computeRoomQuality(RoomQuality& quality);
bool stockpileAccepts(const StockpileComponent& stockpile, const ItemState& item);
float stockpileScore(const StockpileComponent& stockpile, const ItemState& item, float distance);
std::optional<HaulCandidate> chooseStockpile(const ItemState& item,
                                             std::span<const StockpileComponent> stockpiles,
                                             const std::function<float(StockpileId)>& distanceFn);

// Industry and maintenance.
float craftQuality(float skill, float focus, float toolQuality, float inputQuality,
                   float workshopCondition, float environmentSuitability, float difficulty);
float processThroughput(const MachineState& machine, const RecipeProcess& recipe,
                        float operatorSkill, float operatorFocus);
void advanceMachineWear(MaintenanceState& maintenance, float work, float contamination,
                        float environmentPenalty);
bool maintenanceDue(const MaintenanceState& maintenance);

// Agriculture / ecology.
void advanceCrop(CropState& crop, float days, float ambientSuitability);
void advanceLivestock(LivestockState& livestock, float days, float feedAvailability,
                      float environmentSuitability);
float ecologicalPressure(float hunting, float grazing, float pollution, float introducedSpecies,
                         float habitatLoss, float protection);

// Environment / bounded spatial simulation helpers.
bool atmosphereCountsAsSealed(const RoomAtmosphere& room);
void equalizeAtmosphere(RoomAtmosphere& a, RoomAtmosphere& b, float aperture, float dt);
void advanceFire(FireState& fire, RoomAtmosphere& room, float flammability, float suppression, float dt);
void advanceContamination(ContaminationState& contamination, float cleaning, float isolation, float dt);

// Anatomy / medicine.
void recomputeBodyCapabilities(BodyState& body);
TreatmentPlan buildTreatmentPlan(StableId patient, const BodyState& body);
float treatmentEffectiveness(TreatmentKind treatment, float skill, float supplies, float cleanliness);
void advanceHealing(BodyState& body, float days, float nutrition, float immunity, float careQuality);

// Military / security / justice.
float squadReadiness(const SquadState& squad, float equipmentCompleteness, float healthFraction,
                     float scheduleCompliance);
AlertLevel chooseSquadAlert(float threatStrength, float fortressDefense, float civilianRisk);
float evidenceStrength(const CrimeRecord& crime, float forensicQuality, float witnessReliability);
std::string recommendSentence(const JusticeCase& justiceCase, float lawSeverity, float rehabilitationBias);

// Governance / institutions / migration.
float officeSuitability(const OfficeState& office, const Skills& skills, const Values& values,
                        const Personality& personality);
float institutionServiceQuality(const InstitutionState& institution, float staffing,
                                float supplyFraction, float roomQuality);
MigrationDecision decideMigration(const MigrationCandidate& candidate, float destinationStability,
                                  float destinationCapacity);

// Economy / trade / contracts / diplomacy.
void updateMarketGood(MarketGood& good, float imports, float exports, float localProduction,
                      float localConsumption);
ContractState generateContract(const ContractGenerationContext& context, ContractType type,
                               ContractScope scope);
float caravanLossRisk(const CaravanState& caravan, float routeHazard, float hostilePressure);
void advanceDiplomacy(DiplomacyState& state, float trade, float borderConflict,
                      float sharedThreat, float ideologicalDistance);

// Empire / standing / claims.
void applyStanding(StandingState& standing, std::uint64_t system, float favorDelta,
                   float suspicionDelta, float suspicionFloor = 0.0f);
float registerActionPressure(float suspicion, float industrialActivity, float defensesVisible);

// Threats, disease and rift systems.
RiftHorrorState generateRiftHorror(std::uint64_t seed, std::uint64_t epoch, SiteId target);
void advanceThreat(ThreatState& threat, float fortressStrength, float days);
void advanceSyndrome(SyndromeState& syndrome, float days, float treatment, float immunity);
float transmissionRisk(const SyndromeState& syndrome, float contactIntensity, float filtration,
                       float protectiveEquipment);

// Knowledge, research, cultural works and obsession.
void advanceResearch(ResearchProject& project, float researcherSkill, float focus,
                     float facilityQuality, float sampleQuality, float hours);
float knowledgeTransfer(float teacherSkill, float studentAptitude, float relationshipAffinity,
                        float institutionQuality, float hours);
ArtifactState resolveObsession(AethericObsession& obsession, StableId craftedItem,
                               StableId creator, std::uint64_t seed, TimeStamp time);

// Exploration / operative / POI-facing helpers.
float scanProgress(float scannerGrade, float rangeFactor, float targetComplexity,
                   float interference, float seconds);
void advanceOperativeObjective(DirectOperativeState& operative, float progress, bool crisis);

// Vehicles, ships, orbit and freight.
float vehicleMobility(const VehicleState& vehicle, float terrainPenalty, float weatherPenalty);
float shipTakeoffMargin(const ShipState& ship, float gravity, float cargoLoadFraction,
                        float weatherSeverity);
bool canWarp(const ShipState& ship, float routeDistanceLy, float fuelCost);
void advanceShipRoute(ShipState& ship, float routeDistanceLy, float lyPerHour, float hours);
RemoteSiteDelta advanceRemoteSite(const SiteState& site, float days, float tradeThroughput,
                                  float threatPressure);

// Power, automation and rail.
PowerAllocationResult allocatePower(std::span<PowerNode> nodes, float dt);
bool evaluateAutomationRule(AutomationRule& rule, std::span<const AutomationSignal> signals);
void advanceRailCart(RailCartState& cart, float routeLength, float traction, float dt);

// Construction / structure.
std::vector<CreateDesignationCommand> expandBlueprint(const ConstructionBlueprint& blueprint,
                                                       const SpatialAnchor& origin,
                                                       StableId owner,
                                                       std::uint64_t seed,
                                                       std::uint64_t tick);
float supportMargin(const SupportIsland& island);
bool shouldCollapse(const SupportIsland& island);

// History, civilizations, sites and strategic world simulation.
float historicalSignificance(const HistoricalEvent& event);
bool shouldPromoteHistoricalFigure(const HistorySignificance& current,
                                   const HistoricalEvent& event);
void advanceCivilization(CivilizationState& civilization, float years, float resourceAccess,
                         float conflictPressure, float tradeAccess);
void advanceSite(SiteState& site, float days, float incomingFood, float outgoingGoods,
                 float threatPressure);

// Inspector / alert surfaces.
InspectorTrace explainStress(StableId citizen, const StressState& stress, const Needs& needs,
                             const Memories& memories);
InspectorTrace explainJob(const JobComponent& job);
InspectorTrace explainMachine(const MachineState& machine, const MaintenanceState& maintenance);
std::vector<AlertRecord> collectAlerts(SiteId site,
                                       std::span<const ThreatState> threats,
                                       std::span<const MachineState> machines,
                                       std::span<const StressState> populationStress,
                                       std::span<const RoomAtmosphere> rooms);

} // namespace elysium::fortress
