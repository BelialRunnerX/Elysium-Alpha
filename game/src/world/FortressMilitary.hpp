// Intended function: imported world implementation for FortressMilitary; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/PlanetSurface.hpp"
#include "world/SurfaceInfrastructure.hpp"
#include "world/SurfaceSiege.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

// Agent 26: portable fortress-military simulation contract. These are narrow
// component-shaped data records intended to be emplaced on EnTT entities by the
// ECS adapter. The portable layer deliberately does not include EnTT and never
// uses a transient ECS handle as durable identity.
enum class MilitaryRoutine : std::uint8_t {
    Reserve = 0,
    Training = 1,
    Patrol = 2,
    Guard = 3,
    Ready = 4,
    Siege = 5,
    Expedition = 6,
    EvacuationSupport = 7
};

enum class MilitaryAlertLevel : std::uint8_t {
    Normal = 0,
    Ready = 1,
    Siege = 2,
    Evacuate = 3,
    Isolate = 4
};

enum class TacticalCommandType : std::uint8_t {
    SetAlert = 0,
    Rally = 1,
    Patrol = 2,
    Breach = 3,
    Evacuate = 4,
    Isolate = 5,
    Guard = 6
};

enum class MilitaryThreatKind : std::uint8_t {
    Unknown = 0,
    Wildlife = 1,
    Raiders = 2,
    ImperialRegisterAction = 3,
    Infiltration = 4,
    Rift = 5
};

enum class MilitaryReservationKind : std::uint8_t { Uniform = 0, Ammunition = 1 };

enum class MilitaryJobKind : std::uint8_t {
    Equip = 0,
    ResupplyAmmo = 1,
    Drill = 2,
    Patrol = 3,
    Guard = 4,
    Rally = 5,
    Breach = 6,
    Evacuate = 7,
    Isolate = 8,
    Medevac = 9
};

enum class MilitaryDefenseAssetKind : std::uint8_t {
    Sensor = 0,
    Turret = 1,
    Shield = 2,
    Portal = 3,
    Atmosphere = 4
};

enum class MilitaryInfrastructureAction : std::uint8_t {
    EnableMachine = 0,
    ClosePortal = 1
};

enum class MilitaryDiagnosticCode : std::uint8_t {
    Ready = 0,
    MissingMember = 1,
    MissingEquipment = 2,
    MissingAmmo = 3,
    MissingBarracks = 4,
    MissingTarget = 5,
    UnsafeTarget = 6,
    WoundedMember = 7,
    DefenseOffline = 8,
    ReservationBlocked = 9
};

const char* militaryRoutineName(MilitaryRoutine value);
const char* militaryAlertName(MilitaryAlertLevel value);
const char* tacticalCommandName(TacticalCommandType value);
const char* militaryThreatName(MilitaryThreatKind value);
const char* militaryDiagnosticName(MilitaryDiagnosticCode value);
struct MilitaryDiagnostic;
std::string militaryDiagnosticText(const MilitaryDiagnostic& diagnostic);

struct MilitarySquadIdentityComponent {
    std::uint64_t stableId{};
    std::string name;
    std::uint64_t factionStableId{};
};

struct MilitarySquadMembershipComponent {
    std::vector<std::uint64_t> members;
    std::uint64_t commanderStableId{};
};

struct MilitaryUniformRequirement {
    int itemId{};
    int countPerMember{1};
    bool optional{};

    friend bool operator==(const MilitaryUniformRequirement&, const MilitaryUniformRequirement&) = default;
};

struct MilitaryUniformProfileComponent {
    std::uint64_t profileStableId{};
    std::vector<MilitaryUniformRequirement> requirements;
};

struct MilitaryEquipmentPolicyComponent {
    bool allowSubstitutes{};
    bool allowReservationPreemption{};
};

struct MilitaryTrainingPolicyComponent {
    float targetProficiency{0.60f};
    float drillGainPerHour{0.08f};
};

struct MilitaryScheduleComponent {
    std::array<MilitaryRoutine, 24> hourly{};

    MilitaryScheduleComponent() { hourly.fill(MilitaryRoutine::Reserve); }
};

struct MilitaryOrderComponent {
    std::uint64_t sequence{};
    TacticalCommandType type{TacticalCommandType::SetAlert};
    bool active{};
    bool hasTargetCell{};
    SurfaceCellAddress targetCell{};
    std::uint64_t targetStableId{};
    std::uint64_t threatStableId{};
};

struct MilitaryMissionComponent {
    std::uint64_t missionStableId{};
    MilitaryThreatKind threatKind{MilitaryThreatKind::Unknown};
    std::uint64_t threatStableId{};
    bool active{};
    bool hasTargetCell{};
    SurfaceCellAddress targetCell{};
};

struct MilitaryAlertStateComponent {
    MilitaryAlertLevel level{MilitaryAlertLevel::Normal};
    std::uint64_t sourceThreatStableId{};
};

struct MilitaryBarracksAssignmentComponent {
    std::uint64_t barracksStableId{};
    bool hasRallyCell{};
    SurfaceCellAddress rallyCell{};
    bool hasTrainingCell{};
    SurfaceCellAddress trainingCell{};
};

struct MilitaryPostAssignmentComponent {
    std::vector<SurfaceCellAddress> patrolRoute;
    std::vector<SurfaceCellAddress> guardCells;
    bool hasEvacuationCell{};
    SurfaceCellAddress evacuationCell{};
    bool hasMedevacCell{};
    SurfaceCellAddress medevacCell{};
};

struct MilitaryAmmoPolicyComponent {
    int ammoItemId{};
    int roundsPerMember{8};
    int squadReserveRounds{16};
};

struct MilitaryReadinessSummaryComponent {
    float equipmentRatio{1.0f};
    float ammoRatio{1.0f};
    float trainingRatio{1.0f};
    float healthRatio{1.0f};
    float positionRatio{1.0f};
    float infrastructureRatio{1.0f};
    float overall{1.0f};
    int missingEquipment{};
    int missingAmmo{};
    int woundedMembers{};
};

struct MilitaryHistoryPolicyComponent {
    bool recordBattles{true};
    bool recordPromotions{true};
};

struct MilitarySquadEntityState {
    MilitarySquadIdentityComponent identity;
    MilitarySquadMembershipComponent membership;
    MilitaryUniformProfileComponent uniform;
    MilitaryEquipmentPolicyComponent equipmentPolicy;
    MilitaryTrainingPolicyComponent trainingPolicy;
    MilitaryScheduleComponent schedule;
    MilitaryOrderComponent currentOrder;
    MilitaryMissionComponent mission;
    MilitaryAlertStateComponent alert;
    MilitaryBarracksAssignmentComponent barracks;
    MilitaryPostAssignmentComponent posts;
    MilitaryAmmoPolicyComponent ammo;
    MilitaryReadinessSummaryComponent readiness;
    MilitaryHistoryPolicyComponent history;
    MilitaryRoutine scheduledRoutine{MilitaryRoutine::Reserve};
    MilitaryRoutine effectiveRoutine{MilitaryRoutine::Reserve};
};

struct MilitaryMemberState {
    std::uint64_t stableId{};
    bool available{true};
    bool incapacitated{};
    bool hasCell{};
    SurfaceCellAddress cell{};
    float healthFraction{1.0f};
    float trainingProficiency{};
};

struct MilitaryStockItem {
    int itemId{};
    int count{};
};

struct MilitaryReservation {
    std::uint64_t stableId{};
    MilitaryReservationKind kind{MilitaryReservationKind::Uniform};
    std::uint64_t squadStableId{};
    std::uint64_t memberStableId{};
    int itemId{};
    int count{};
    int priority{};
    bool preemptible{};
};

struct MilitaryWorldState {
    std::vector<MilitarySquadEntityState> squads;
    std::vector<MilitaryMemberState> members;
    std::vector<MilitaryStockItem> stock;
    std::vector<MilitaryReservation> reservations;
};

struct MilitaryThreatIntent {
    std::uint64_t stableId{};
    MilitaryThreatKind kind{MilitaryThreatKind::Unknown};
    float severity{}; // [0,1]
    bool hasCell{};
    SurfaceCellAddress cell{};
};

struct MilitaryTacticalCommand {
    std::uint64_t sequence{};
    std::uint64_t squadStableId{};
    TacticalCommandType type{TacticalCommandType::SetAlert};
    MilitaryAlertLevel alert{MilitaryAlertLevel::Normal};
    bool hasTargetCell{};
    SurfaceCellAddress targetCell{};
    std::uint64_t targetStableId{};
    std::uint64_t threatStableId{};
};

struct MilitaryJobCompletion {
    std::uint64_t jobStableId{};
    MilitaryJobKind kind{MilitaryJobKind::Drill};
    std::uint64_t squadStableId{};
    std::uint64_t memberStableId{};
    float workHours{};
    bool succeeded{true};
};

struct MilitaryDefenseAsset {
    std::uint64_t stableId{};
    MilitaryDefenseAssetKind kind{MilitaryDefenseAssetKind::Sensor};
    bool enabled{};
    bool powered{};
    int ammunition{};
    float charge{};
};

struct MilitaryUpdateInput {
    int hourOfDay{};
    float dtHours{};
    std::vector<MilitaryThreatIntent> threats;
    std::vector<MilitaryTacticalCommand> tacticalCommands;
    std::vector<MilitaryJobCompletion> completedJobs;
    std::vector<MilitaryDefenseAsset> defenseAssets;
};

struct MilitaryRoutePolicy {
    bool allowRestrictedZones{};
    bool requireBreathable{true};
    bool avoidHighHazard{true};
    bool allowBreach{};
};

struct MilitaryRouteRequest {
    std::uint64_t stableId{};
    std::uint64_t squadStableId{};
    std::uint64_t memberStableId{};
    MilitaryJobKind purpose{MilitaryJobKind::Rally};
    SurfaceCellAddress from{};
    SurfaceCellAddress goal{};
    MilitaryRoutePolicy policy{};
};

struct MilitaryJobRequest {
    std::uint64_t stableId{};
    MilitaryJobKind kind{MilitaryJobKind::Equip};
    std::uint64_t squadStableId{};
    std::uint64_t memberStableId{};
    int itemId{};
    int count{};
    bool hasTargetCell{};
    SurfaceCellAddress targetCell{};
};

struct MilitaryReservationCommand {
    MilitaryReservation reservation;
};

// Owner-thread release of a reservation that is no longer justified by the
// squad roster/equipment policy. Keeping this explicit in the plan makes
// quartermaster cleanup inspectable and deterministic rather than hiding it
// inside stock accounting.
struct MilitaryReservationReleaseCommand {
    std::uint64_t reservationStableId{};
    std::uint64_t squadStableId{};
    int releaseCount{};
};

struct MilitaryInfrastructureIntent {
    std::uint64_t stableId{};
    MilitaryInfrastructureAction action{MilitaryInfrastructureAction::EnableMachine};
    std::uint64_t targetStableId{};
};

struct MilitarySquadMutation {
    std::uint64_t squadStableId{};
    MilitaryRoutine scheduledRoutine{MilitaryRoutine::Reserve};
    MilitaryRoutine effectiveRoutine{MilitaryRoutine::Reserve};
    MilitaryAlertStateComponent alert;
    MilitaryOrderComponent currentOrder;
    MilitaryMissionComponent mission;
    MilitaryReadinessSummaryComponent readiness;
};

struct MilitaryTrainingMutation {
    std::uint64_t memberStableId{};
    float delta{};
};

enum class MilitaryEventType : std::uint8_t { ThreatDetected = 0, AlertChanged = 1, SquadOrderIssued = 2 };

struct MilitaryEvent {
    MilitaryEventType type{MilitaryEventType::ThreatDetected};
    std::uint64_t sequence{};
    std::uint64_t squadStableId{};
    std::uint64_t relatedStableId{};
};

struct MilitaryDiagnostic {
    std::uint64_t squadStableId{};
    MilitaryDiagnosticCode code{MilitaryDiagnosticCode::Ready};
    std::uint64_t relatedStableId{};
    int itemId{};
    int amount{};
};

struct MilitaryPlan {
    std::vector<MilitarySquadMutation> squadMutations;
    std::vector<MilitaryTrainingMutation> trainingMutations;
    std::vector<MilitaryReservationCommand> reservationCommands;
    std::vector<MilitaryReservationReleaseCommand> reservationReleases;
    std::vector<MilitaryJobRequest> jobs;
    std::vector<MilitaryRouteRequest> routes;
    std::vector<MilitaryInfrastructureIntent> infrastructure;
    std::vector<MilitaryEvent> events;
    std::vector<MilitaryDiagnostic> diagnostics;
};

struct MilitaryCommitResult {
    int squadMutations{};
    int trainingMutations{};
    int reservationsAdded{};
    int reservationsReleased{};
    int reservationConflicts{};
};

// Stateless planner/committer. plan() is read-only and safe to execute in a
// deterministic worker partition. commit() is the owner-thread publication
// phase. No system here structurally mutates an ECS registry.
class FortressMilitarySystem {
public:
    MilitaryPlan plan(const MilitaryWorldState& state, const MilitaryUpdateInput& input) const;
    MilitaryCommitResult commit(MilitaryWorldState& state, const MilitaryPlan& plan) const;
};

// Bridge helpers consume existing Agent 16/23 defense infrastructure without
// duplicating its power, ammunition, shield, portal, or atmosphere ownership.
std::vector<MilitaryDefenseAsset> snapshotSurfaceDefense(const SurfaceInfrastructure& infrastructure);
int applySurfaceMilitaryInfrastructureIntents(PlanetSurface& planet,
                                              SurfaceInfrastructure& infrastructure,
                                              const std::vector<MilitaryInfrastructureIntent>& intents);

// Adapter from Agent 25's Register Action director into a generic military
// threat. This observes siege state only and never schedules Imperial actions.
std::optional<MilitaryThreatIntent> militaryThreatFromRegisterAction(const SurfaceSiegeState& state,
                                                                      std::optional<SurfaceCellAddress> target = std::nullopt);

// Narrow versioned codec for the persistent squad entity state. Reservations,
// route requests and derived readiness are rebuildable runtime facts.
std::string serializeMilitarySquad(const MilitarySquadEntityState& squad);
std::optional<MilitarySquadEntityState> deserializeMilitarySquad(std::string_view text,
                                                                 std::string* error = nullptr);

} // namespace elysium
