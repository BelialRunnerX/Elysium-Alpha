#pragma once

#include "fortress/Components.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace elysium::fortress {

struct CommandHeader {
    std::uint64_t tick{};
    std::uint32_t producer{};
    std::uint32_t sequence{};
};

struct SpawnCitizenCommand {
    PersistentIdentity identity{};
    SettlementMembership membership{};
    AgeLifeStage age{};
    BiologicalState biology{};
};

struct CreateJobCommand { JobComponent job{}; };
struct UpdateJobCommand { JobId job{}; JobState state{JobState::Pending}; float progress{}; std::string reason; };
struct CancelJobCommand { JobId job{}; std::string reason; };
struct ReserveCommand { ReservationClaim claim{}; };
struct ReleaseReservationCommand { JobId job{}; StableId resource{}; std::string reason; };
struct CreateDesignationCommand { DesignationComponent designation{}; };
struct CreateRoomCommand { RoomComponent room{}; };
struct CreateZoneCommand { ZoneComponent zone{}; };
struct CreateStockpileCommand { StockpileComponent stockpile{}; };
struct TransferItemCommand { StableId item{}; StableId source{}; StableId destination{}; float quantity{}; };
struct SpawnItemCommand { ItemState item{}; };
struct ConsumeItemCommand { StableId item{}; float quantity{}; };
struct StartProcessCommand { StableId machine{}; ContentId recipe; };
struct StopProcessCommand { StableId machine{}; std::string reason; };
struct RequestMaintenanceCommand { StableId machine{}; PriorityBand priority{PriorityBand::Normal}; };
struct ApplyThoughtCommand { StableId citizen{}; Thought thought{}; };
struct ApplyMemoryCommand { StableId citizen{}; Memory memory{}; };
struct AdjustStressCommand { StableId citizen{}; float delta{}; std::string reason; };
struct AdjustRelationshipCommand { StableId a{}; StableId b{}; float affinity{}; float trust{}; float respect{}; float fear{}; float grievance{}; };
struct ApplyWoundCommand { StableId patient{}; WoundState wound{}; };
struct CreateTreatmentCommand { TreatmentOrder order{}; };
struct EquipProstheticCommand { StableId patient{}; ProstheticState prosthetic{}; };
struct SetSquadAlertCommand { SquadId squad{}; AlertLevel alert{AlertLevel::Green}; };
struct CreateCaseCommand { JusticeCase justiceCase{}; };
struct AssignOfficeCommand { OfficeState office{}; };
struct CreateInstitutionCommand { InstitutionState institution{}; };
struct MigratePersonCommand { StableId person{}; SiteId from{}; SiteId to{}; };
struct CreateContractCommand { ContractState contract{}; };
struct AdjustStandingCommand { std::uint64_t system{}; float favorDelta{}; float suspicionDelta{}; };
struct SetClaimCommand { ClaimState claim{}; };
struct SpawnThreatCommand { ThreatState threat{}; };
struct CreateResearchCommand { ResearchProject project{}; };
struct CompleteResearchCommand { StableId project{}; ContentId unlock; };
struct BeginObsessionCommand { AethericObsession obsession{}; };
struct CreateArtifactCommand { ArtifactState artifact{}; };
struct RecordScanCommand { ScanRecord scan{}; };
struct SetOperativeCommand { DirectOperativeState operative{}; };
struct DispatchVehicleCommand { VehicleId vehicle{}; SiteId destination{}; ContentId route; };
struct DispatchShipCommand { StableId ship{}; std::uint64_t destinationSystem{}; };
struct CreateOrbitalSiteCommand { OrbitalSiteState site{}; };
struct SetAutomationRuleCommand { AutomationRule rule{}; };
struct SetPowerIsolationCommand { StableId node{}; bool isolated{}; };
struct DispatchRailCartCommand { StableId cart{}; ContentId route; };
struct StampBlueprintCommand { ConstructionBlueprint blueprint{}; SpatialAnchor origin{}; StableId owner{}; };
struct MarkSupportDirtyCommand { StableId island{}; };
struct UpdateSiteCommand { SiteState site{}; };
struct UpdateCivilizationCommand { CivilizationState civilization{}; };
struct RecordHistoricalEventCommand { HistoricalEvent event{}; };
struct PromoteHistoricalFigureCommand { StableId person{}; float significance{}; };

using FortressCommandPayload = std::variant<
    SpawnCitizenCommand,
    CreateJobCommand,
    UpdateJobCommand,
    CancelJobCommand,
    ReserveCommand,
    ReleaseReservationCommand,
    CreateDesignationCommand,
    CreateRoomCommand,
    CreateZoneCommand,
    CreateStockpileCommand,
    TransferItemCommand,
    SpawnItemCommand,
    ConsumeItemCommand,
    StartProcessCommand,
    StopProcessCommand,
    RequestMaintenanceCommand,
    ApplyThoughtCommand,
    ApplyMemoryCommand,
    AdjustStressCommand,
    AdjustRelationshipCommand,
    ApplyWoundCommand,
    CreateTreatmentCommand,
    EquipProstheticCommand,
    SetSquadAlertCommand,
    CreateCaseCommand,
    AssignOfficeCommand,
    CreateInstitutionCommand,
    MigratePersonCommand,
    CreateContractCommand,
    AdjustStandingCommand,
    SetClaimCommand,
    SpawnThreatCommand,
    CreateResearchCommand,
    CompleteResearchCommand,
    BeginObsessionCommand,
    CreateArtifactCommand,
    RecordScanCommand,
    SetOperativeCommand,
    DispatchVehicleCommand,
    DispatchShipCommand,
    CreateOrbitalSiteCommand,
    SetAutomationRuleCommand,
    SetPowerIsolationCommand,
    DispatchRailCartCommand,
    StampBlueprintCommand,
    MarkSupportDirtyCommand,
    UpdateSiteCommand,
    UpdateCivilizationCommand,
    RecordHistoricalEventCommand,
    PromoteHistoricalFigureCommand>;

struct FortressCommand {
    CommandHeader header{};
    FortressCommandPayload payload{};
};

class FortressCommandBuffer {
public:
    template <class T>
    void push(CommandHeader header, T&& payload) {
        commands_.push_back(FortressCommand{header, FortressCommandPayload{std::forward<T>(payload)}});
    }

    void append(FortressCommand command) { commands_.push_back(std::move(command)); }

    void sortDeterministic() {
        std::stable_sort(commands_.begin(), commands_.end(), [](const FortressCommand& a, const FortressCommand& b) {
            if (a.header.tick != b.header.tick) return a.header.tick < b.header.tick;
            if (a.header.producer != b.header.producer) return a.header.producer < b.header.producer;
            if (a.header.sequence != b.header.sequence) return a.header.sequence < b.header.sequence;
            return a.payload.index() < b.payload.index();
        });
    }

    [[nodiscard]] const std::vector<FortressCommand>& commands() const noexcept { return commands_; }
    [[nodiscard]] std::vector<FortressCommand>& commands() noexcept { return commands_; }
    [[nodiscard]] std::size_t size() const noexcept { return commands_.size(); }
    [[nodiscard]] bool empty() const noexcept { return commands_.empty(); }
    void clear() noexcept { commands_.clear(); }

private:
    std::vector<FortressCommand> commands_;
};

enum class FortressEventKind : std::uint16_t {
    JobCreated,
    JobBlocked,
    JobCompleted,
    ReservationInvalidated,
    ItemMoved,
    MachineStarted,
    MachineFaulted,
    NeedBecameUrgent,
    StressBandChanged,
    RelationshipChanged,
    CitizenDied,
    WoundApplied,
    TreatmentCompleted,
    DiseaseDetected,
    FireStarted,
    AtmosphereLost,
    ContaminationDetected,
    CrimeWitnessed,
    CaseResolved,
    OfficeChanged,
    MigrantArrived,
    CaravanArrived,
    ContractCompleted,
    RegisterActionAnnounced,
    ThreatArrived,
    ThreatDefeated,
    ResearchCompleted,
    ObsessionStarted,
    ArtifactCreated,
    ScanCompleted,
    SiteFounded,
    SiteAbandoned,
    SiteReclaimed,
    HistoricalEventRecorded,
    PowerBrownout,
    RailIncident,
    StructuralCollapse,
    OperativeIntervention
};

struct FortressEvent {
    FortressEventKind kind{FortressEventKind::JobCreated};
    TimeStamp time{};
    StableId primary{};
    StableId secondary{};
    SiteId site{};
    ContentId content;
    float value{};
    std::string message;
};

} // namespace elysium::fortress
