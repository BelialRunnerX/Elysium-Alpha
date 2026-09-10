// Intended function: imported ui implementation for UiViewModels; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/SurfaceIndustry.hpp"
#include "world/SurfaceSiege.hpp"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

// UI state is derived presentation data. None of these types owns authoritative
// simulation state, ECS identity, voxel geometry, or save-format identity.
enum class UiSeverity : std::uint8_t {
    Info = 0,
    Advisory = 1,
    High = 2,
    Critical = 3
};

const char* uiSeverityName(UiSeverity severity);

enum class UiObjectKind : std::uint8_t {
    System = 0,
    Machine = 1,
    Portal = 2,
    Airlock = 3,
    Job = 4,
    Citizen = 5,
    Room = 6,
    Stockpile = 7,
    Squad = 8,
    Case = 9,
    Contract = 10,
    HistoricalEvent = 11
};

struct UiFact {
    std::string label;
    std::string value;
};

struct UiDiagnostic {
    // Stable presentation key used for deduplication/acknowledgement. This is
    // not a save identity and must never be used as gameplay truth.
    std::string key;
    UiSeverity severity{UiSeverity::Info};
    std::string category;
    std::uint64_t sourceStableId{};
    std::string summary;
    std::string cause;
    std::string remediation;

    friend bool operator==(const UiDiagnostic&, const UiDiagnostic&) = default;
};

struct WhyInspectorModel {
    UiObjectKind kind{UiObjectKind::System};
    std::uint64_t stableId{};
    std::string title;
    std::string subtitle;
    std::vector<UiFact> facts;
    std::vector<UiDiagnostic> diagnostics;
};

struct UiDiagnosticContext {
    PowerNetworkSummary power{};
    SurfaceIndustryTelemetry industry{};
    SurfaceDefenseTelemetry defense{};
    SurfaceSiegeState siege{};
};

// Renderer-neutral fortress overview. This is a projection of authoritative
// subsystem snapshots only; it deliberately contains no mutable simulation
// state. Panels whose upstream simulation does not exist can be represented as
// unavailable rather than fabricating placeholder truth.
enum class UiPanelAvailability : std::uint8_t { Available = 0, Unavailable = 1 };

struct UiPanelLink {
    std::string id;
    std::string title;
    UiPanelAvailability availability{UiPanelAvailability::Available};
    std::string detail;
};

struct FortressDashboardModel {
    std::string title;
    std::vector<UiFact> overview;
    std::vector<UiDiagnostic> diagnostics;
    std::vector<UiPanelLink> panels;
};

FortressDashboardModel buildFortressDashboard(const SurfaceInfrastructure& infrastructure,
                                               const UiDiagnosticContext& context);

// Build a read-only Why Inspector for currently implemented infrastructure.
// The function intentionally mirrors only public subsystem contracts; it does
// not reach into private simulation state or mutate any object.
WhyInspectorModel buildMachineInspector(const SurfaceInfrastructure& infrastructure,
                                        const SurfaceMachineObject& machine,
                                        const UiDiagnosticContext& context = {});
WhyInspectorModel buildPortalInspector(const SurfaceInfrastructure& infrastructure,
                                       const SurfacePortalObject& portal);
WhyInspectorModel buildAirlockInspector(const SurfaceInfrastructure& infrastructure,
                                        const SurfaceAirlockAssembly& airlock);

struct UiAlert {
    std::string key;
    UiSeverity severity{UiSeverity::Info};
    std::string category;
    std::uint64_t sourceStableId{};
    std::string title;
    std::string detail;
    std::string remediation;

    friend bool operator==(const UiAlert&, const UiAlert&) = default;
};

// Derive player-facing alert candidates from authoritative subsystem snapshots.
// Output is deduplicated and sorted deterministically (severity, category, key).
std::vector<UiAlert> buildInfrastructureAlerts(const SurfaceInfrastructure& infrastructure,
                                               const UiDiagnosticContext& context);

struct UiPresentedAlert {
    UiAlert alert;
    bool acknowledged{};
    bool pinned{};
};

// Presentation-only alert state. Acknowledging/muting/pinning never writes back
// into the simulation; destroying and rebuilding this object cannot alter game
// truth. New/changed alert content clears acknowledgement automatically.
class UiAlertCenter {
public:
    std::vector<UiPresentedAlert> update(const std::vector<UiAlert>& candidates);
    bool acknowledge(std::string_view key);
    bool setPinned(std::string_view key, bool pinned);
    void setCategoryMuted(std::string category, bool muted);
    bool categoryMuted(std::string_view category) const;
    void clearPresentationState();

private:
    struct State {
        std::string fingerprint;
        bool acknowledged{};
        bool pinned{};
        bool active{};
    };

    std::map<std::string, State, std::less<>> states_;
    std::set<std::string, std::less<>> mutedCategories_;
};

// UI commands are intents only. Authoritative systems decide whether they are
// valid and perform mutation in their normal owner-thread/commit phase.
enum class UiCommandKind : std::uint8_t {
    SetMachineEnabled = 0,
    SetPortalOpen = 1,
    SelectRecipe = 2,
    ConfigureLogistics = 3,
    ClearLogistics = 4,
    Designation = 5,
    TacticalOrder = 6
};

enum class UiDesignationKind : std::uint8_t {
    Mine = 0,
    Channel = 1,
    Construct = 2,
    Deconstruct = 3,
    Gather = 4,
    TrafficRestricted = 5,
    UtilityRoute = 6,
    EvacuationZone = 7
};

enum class UiTacticalOrderKind : std::uint8_t {
    Alert = 0,
    Rally = 1,
    Patrol = 2,
    Breach = 3,
    Evacuate = 4,
    Isolate = 5
};

struct UiCommandRequest {
    UiCommandKind kind{UiCommandKind::SetMachineEnabled};
    std::uint64_t primaryStableId{};
    std::uint64_t secondaryStableId{};
    std::uint64_t tertiaryStableId{};
    std::uint64_t alternateStableId{};
    SurfaceCellAddress cell{};
    bool hasCell{};
    bool boolValue{};
    int intValue{};
    UiDesignationKind designation{UiDesignationKind::Mine};
    UiTacticalOrderKind tacticalOrder{UiTacticalOrderKind::Alert};
};

UiCommandRequest makeSetMachineEnabledCommand(std::uint64_t machineStableId, bool enabled);
UiCommandRequest makeSetPortalOpenCommand(std::uint64_t portalStableId, bool open);
UiCommandRequest makeSelectRecipeCommand(std::uint64_t machineStableId, SurfaceRecipeId recipe);
UiCommandRequest makeConfigureLogisticsCommand(std::uint64_t transportStableId,
                                               std::uint64_t sourceStableId,
                                               std::uint64_t targetStableId,
                                               std::uint64_t alternateTargetStableId = 0,
                                               int sorterFilterItemId = 0);
UiCommandRequest makeClearLogisticsCommand(std::uint64_t transportStableId);
UiCommandRequest makeDesignationCommand(UiDesignationKind designation,
                                        SurfaceCellAddress cell,
                                        int priority = 0);
UiCommandRequest makeTacticalOrderCommand(UiTacticalOrderKind order,
                                          SurfaceCellAddress cell,
                                          std::uint64_t squadStableId = 0);

bool validateUiCommand(const UiCommandRequest& command, std::string* error = nullptr);
std::string formatUiCommand(const UiCommandRequest& command);

std::string formatSurfaceAddress(SurfaceCellAddress address);
std::string formatStableId(std::uint64_t stableId);

} // namespace elysium
