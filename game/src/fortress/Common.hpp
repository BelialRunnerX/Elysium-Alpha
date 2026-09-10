#pragma once

#include "core/Determinism.hpp"
#include "core/Math.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace elysium::fortress {

template <class Tag>
struct StrongId {
    std::uint64_t value{};
    constexpr explicit operator bool() const noexcept { return value != 0; }
    friend constexpr bool operator==(StrongId, StrongId) = default;
    friend constexpr auto operator<=>(StrongId, StrongId) = default;
};

struct StableTag {};
struct SiteTag {};
struct OrganizationTag {};
struct CivilizationTag {};
struct ArtifactTag {};
struct JobTag {};
struct WorkOrderTag {};
struct ContractTag {};
struct CaseTag {};
struct InstitutionTag {};
struct SquadTag {};
struct VehicleTag {};
struct HistoricalEventTag {};
struct RoomTag {};
struct ZoneTag {};
struct StockpileTag {};
struct DesignationTag {};
struct BlueprintTag {};

using StableId = StrongId<StableTag>;
using SiteId = StrongId<SiteTag>;
using OrganizationId = StrongId<OrganizationTag>;
using CivilizationId = StrongId<CivilizationTag>;
using ArtifactId = StrongId<ArtifactTag>;
using JobId = StrongId<JobTag>;
using WorkOrderId = StrongId<WorkOrderTag>;
using ContractId = StrongId<ContractTag>;
using CaseId = StrongId<CaseTag>;
using InstitutionId = StrongId<InstitutionTag>;
using SquadId = StrongId<SquadTag>;
using VehicleId = StrongId<VehicleTag>;
using HistoricalEventId = StrongId<HistoricalEventTag>;
using RoomId = StrongId<RoomTag>;
using ZoneId = StrongId<ZoneTag>;
using StockpileId = StrongId<StockpileTag>;
using DesignationId = StrongId<DesignationTag>;
using BlueprintId = StrongId<BlueprintTag>;

struct ContentId {
    std::string value;
    friend bool operator==(const ContentId&, const ContentId&) = default;
};

struct ContentRef {
    std::string_view value;
};

// Stable persistent address for a cube-sphere cell. No dense planet-wide index is assumed.
struct CellAddress {
    std::uint64_t planet{};
    std::uint8_t face{};
    std::int32_t u{};
    std::int32_t v{};
    std::int32_t radial{};
    friend bool operator==(const CellAddress&, const CellAddress&) = default;
};

struct SpatialAnchor {
    std::uint64_t world{};
    CellAddress cell{};
    Vec3 localOffset{};
};

struct SpatialExtent {
    SpatialAnchor min{};
    SpatialAnchor max{};
};

struct StableRef {
    StableId id{};
};

struct TimeStamp {
    std::int64_t tick{};
    std::int64_t day{};
    std::int32_t year{};
};

enum class SimulationShard : std::uint8_t {
    DirectOperative,
    ActiveFortress,
    ActivePlanetRemote,
    StarSystem,
    GalaxyHistory
};

enum class ClockFamily : std::uint8_t {
    Frame,
    Tactical,
    LocalSimulation,
    Citizen,
    FortressEconomy,
    Ecology,
    Strategic,
    Historical
};

struct ClockState {
    double accumulator{};
    double intervalSeconds{1.0};
    std::uint64_t epochs{};
};

enum class PriorityBand : std::uint8_t {
    Background,
    Low,
    Normal,
    High,
    Urgent,
    Emergency
};

enum class AccessPolicy : std::uint8_t {
    Open,
    CitizenOnly,
    StaffOnly,
    MilitaryOnly,
    Restricted,
    Quarantine,
    Locked
};

enum class LifeStage : std::uint8_t {
    Dependent,
    Adolescent,
    Adult,
    Elder,
    Dead
};

enum class StressBand : std::uint8_t {
    Stable,
    Strained,
    Distressed,
    Crisis,
    Recovery
};

enum class JobState : std::uint8_t {
    Pending,
    Reserved,
    Navigating,
    Executing,
    Suspended,
    Blocked,
    Completed,
    Cancelled,
    Failed
};

enum class ReservationKind : std::uint8_t {
    Item,
    Tool,
    Workshop,
    Bed,
    TargetCell,
    VehicleCapacity,
    Room,
    Door,
    MachinePort
};

enum class WorkOrderMode : std::uint8_t {
    ProduceQuantity,
    MaintainAtLeast,
    MaintainBetween,
    Repeat
};

enum class WorkOrderFrequency : std::uint8_t {
    Once,
    Periodic,
    ConditionTransition,
    EventTriggered
};

enum class RoomKind : std::uint8_t {
    Bedroom,
    Dormitory,
    Dining,
    Office,
    Hospital,
    Barracks,
    Pasture,
    Stockpile,
    Dump,
    UtilityAccess,
    Hangar,
    Memorial,
    Quarantine,
    Workshop,
    Reactor,
    Bunker,
    Control,
    Greenhouse
};

enum class ZoneKind : std::uint8_t {
    General,
    Stockpile,
    Pasture,
    Dump,
    Quarantine,
    Traffic,
    CivilianRestricted,
    Military,
    Evacuation,
    VehicleLane,
    Firebreak,
    SafeDistrict
};

enum class DesignationKind : std::uint8_t {
    Mine,
    Channel,
    Ramp,
    Stair,
    Bore,
    Smooth,
    Polish,
    Engrave,
    Coat,
    Seal,
    Decontaminate,
    Build,
    Reinforce,
    Repair,
    Replace,
    Deconstruct,
    Gather,
    Salvage,
    Hunt,
    Dump,
    Forbid,
    Quarantine,
    UtilityRoute,
    Traffic,
    Evacuate
};

enum class MediumKind : std::uint8_t {
    Air,
    Water,
    Lava,
    Acid,
    Smoke,
    ToxicGas,
    IndustrialFluid,
    Cryofluid,
    FuelSlurry
};

enum class InjurySeverity : std::uint8_t {
    Minor,
    Moderate,
    Severe,
    Critical,
    Lost
};

enum class TreatmentKind : std::uint8_t {
    Diagnose,
    Clean,
    Dress,
    Suture,
    SetBone,
    Surgery,
    Transfusion,
    Medication,
    Isolation,
    Rehabilitation,
    ProstheticFit
};

enum class AlertLevel : std::uint8_t {
    Green,
    Yellow,
    Red,
    Black
};

enum class CrimeKind : std::uint8_t {
    Theft,
    Assault,
    Murder,
    Sabotage,
    Smuggling,
    Trespass,
    Espionage,
    Vandalism,
    Contraband,
    MandateViolation
};

enum class ContractType : std::uint8_t {
    Survey,
    Procurement,
    Construction,
    Recovery,
    Escort,
    Bounty,
    Infrastructure,
    Smuggling,
    Research,
    Defense
};

enum class ContractScope : std::uint8_t {
    Local,
    System,
    Regional
};

enum class ThreatKind : std::uint8_t {
    Wildlife,
    Raider,
    ImperialRegisterAction,
    Infiltration,
    RiftHorror,
    Titan,
    Plague,
    ReactorIncident,
    EnvironmentalDisaster
};

enum class HistoricalEventKind : std::uint16_t {
    Birth,
    Naming,
    ComingOfAge,
    Death,
    Disappearance,
    RelationshipFormed,
    RelationshipDissolved,
    Mentorship,
    Rivalry,
    Oath,
    Betrayal,
    SiteFounded,
    SiteExpanded,
    SiteRenamed,
    SiteClaimed,
    SiteAbandoned,
    SiteDestroyed,
    SiteContaminated,
    SiteReclaimed,
    OfficeAssumed,
    OfficeRemoved,
    OrganizationFounded,
    OrganizationDissolved,
    Schism,
    WarDeclared,
    Battle,
    Raid,
    Siege,
    Occupation,
    Liberation,
    Armistice,
    Treaty,
    Embargo,
    ArtifactCreated,
    ArtifactStolen,
    ArtifactGifted,
    ArtifactInherited,
    ArtifactLost,
    ArtifactRecovered,
    ArtifactDestroyed,
    Discovery,
    Filing,
    Breakthrough,
    Crime,
    Accusation,
    Conviction,
    Exile,
    Pardon,
    RiftAppearance,
    RiftDefeat,
    Disaster,
    Outbreak,
    Inspection,
    Warrant,
    RegisterAction,
    CourtRuling,
    Amnesty,
    Annexation,
    FirstLanding,
    FirstClaim,
    FortressRetired,
    OperativeIntervention
};

inline float saturate(float value) noexcept {
    return std::clamp(value, 0.0f, 1.0f);
}

inline float boundedPercent(float value) noexcept {
    return std::clamp(value, 0.0f, 100.0f);
}

inline std::uint64_t deterministicToken(std::uint64_t seed, std::uint64_t identity,
                                        std::uint64_t label, std::uint64_t epoch) noexcept {
    return mix64(seed ^ mix64(identity) ^ mix64(label) ^ mix64(epoch));
}

template <class Id>
inline Id makeDerivedId(std::uint64_t seed, std::uint64_t identity,
                        std::uint64_t label, std::uint64_t epoch) noexcept {
    auto value = deterministicToken(seed, identity, label, epoch);
    if (value == 0) {
        value = 1;
    }
    return Id{value};
}

inline float weightedAverage(std::span<const float> values, std::span<const float> weights) {
    const auto n = std::min(values.size(), weights.size());
    float total{};
    float weight{};
    for (std::size_t i = 0; i < n; ++i) {
        total += values[i] * weights[i];
        weight += weights[i];
    }
    return weight > 0.0f ? total / weight : 0.0f;
}

} // namespace elysium::fortress
