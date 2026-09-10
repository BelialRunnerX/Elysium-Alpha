// Intended function: imported gameplay implementation for DirectOperativeController; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/PlanetSurface.hpp"

#include <cstdint>
#include <vector>

namespace elysium {

enum class PlayerControlMode : std::uint8_t {
    DirectOperative = 0,
    FortressCommand = 1,
    TacticalCommand = 2
};

enum class DirectToolMode : std::uint8_t {
    Macro = 0,
    MicroSculpt = 1
};

enum class DirectPlacementMode : std::uint8_t {
    Block = 0,
    Machine = 1,
    Portal = 2
};

enum class DirectInteractionKind : std::uint8_t {
    Context = 0,
    Machine = 1,
    Portal = 2,
    Vehicle = 3,
    Ship = 4,
    Actor = 5
};

enum class DirectCommandKind : std::uint8_t {
    None = 0,
    ToggleToolMode,
    MineTarget,
    MicroSculptTarget,
    PlaceBlock,
    PlaceMachine,
    PlacePortal,
    Interact,
    AttackRay,
    ScanRay,
    EnterFortressCommand,
    EnterDirectOperative
};

struct DirectOperativeInputFrame {
    float dt{};
    bool primaryDown{};
    bool primaryPressed{};
    bool secondaryPressed{};
    bool interactPressed{};
    bool attackPressed{};
    bool scanPressed{};
    bool toggleToolPressed{};
    bool toggleCommandPressed{};
};

struct DirectTargetSnapshot {
    bool macroHit{};
    SurfaceCellAddress hitCell{};
    SurfaceCellAddress previousCell{};

    bool microHit{};
    SurfaceMicroAddress microTarget{};

    DirectInteractionKind interactionKind{DirectInteractionKind::Context};
    std::uint64_t interactionStableId{};
};

struct DirectSelectionState {
    DirectPlacementMode placementMode{DirectPlacementMode::Block};
    int selectedContentId{};
};

struct DirectOperativeCommand {
    DirectCommandKind kind{DirectCommandKind::None};
    std::uint64_t sequence{};
    std::uint64_t actorStableId{};
    float dt{};
    SurfaceCellAddress cell{};
    SurfaceMicroAddress micro{};
    DirectInteractionKind interactionKind{DirectInteractionKind::Context};
    std::uint64_t targetStableId{};
    int contentId{};
};

struct DirectOperativeDiagnostics {
    PlayerControlMode mode{PlayerControlMode::DirectOperative};
    DirectToolMode toolMode{DirectToolMode::Macro};
    std::uint64_t actorStableId{};
    std::uint64_t nextSequence{1};
    int commandsGenerated{};
    int actionsSuppressedByCommandMode{};
};

// Backend-neutral input-to-intent translator for the embodied action layer.
// It owns no world, inventory, combat, or rendering state. Commands are only
// requests: authoritative services remain responsible for validation, resource
// consumption, persistent edits, and final outcomes.
class DirectOperativeController {
public:
    explicit DirectOperativeController(std::uint64_t actorStableId = 1);

    std::uint64_t actorStableId() const { return actorStableId_; }
    PlayerControlMode mode() const { return mode_; }
    DirectToolMode toolMode() const { return toolMode_; }
    const DirectOperativeDiagnostics& diagnostics() const { return diagnostics_; }

    // Changing control surface must never change the stable identity of the
    // controlled actor. This is deliberately independent of entt::entity.
    void bindActorStableId(std::uint64_t stableId);
    void setMode(PlayerControlMode mode);
    void setToolMode(DirectToolMode mode);

    std::vector<DirectOperativeCommand> buildCommands(const DirectOperativeInputFrame& input,
                                                      const DirectTargetSnapshot& target,
                                                      const DirectSelectionState& selection);

private:
    std::uint64_t actorStableId_{1};
    PlayerControlMode mode_{PlayerControlMode::DirectOperative};
    DirectToolMode toolMode_{DirectToolMode::Macro};
    std::uint64_t nextSequence_{1};
    DirectOperativeDiagnostics diagnostics_{};

    DirectOperativeCommand makeCommand(DirectCommandKind kind, float dt = 0.0f);
    void refreshDiagnostics();
};

const char* playerControlModeName(PlayerControlMode mode);
const char* directCommandKindName(DirectCommandKind kind);

} // namespace elysium
