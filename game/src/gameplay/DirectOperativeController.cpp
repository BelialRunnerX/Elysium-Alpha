// Intended function: imported gameplay implementation for DirectOperativeController; preserves the agent-authored subsystem contract for later integration/debugging.
#include "gameplay/DirectOperativeController.hpp"

#include <algorithm>

namespace elysium {

DirectOperativeController::DirectOperativeController(std::uint64_t actorStableId)
    : actorStableId_(actorStableId == 0 ? 1 : actorStableId) {
    refreshDiagnostics();
}

void DirectOperativeController::bindActorStableId(std::uint64_t stableId) {
    if (stableId == 0) return;
    actorStableId_ = stableId;
    refreshDiagnostics();
}

void DirectOperativeController::setMode(PlayerControlMode mode) {
    mode_ = mode;
    refreshDiagnostics();
}

void DirectOperativeController::setToolMode(DirectToolMode mode) {
    toolMode_ = mode;
    refreshDiagnostics();
}

DirectOperativeCommand DirectOperativeController::makeCommand(DirectCommandKind kind, float dt) {
    DirectOperativeCommand command{};
    command.kind = kind;
    command.sequence = nextSequence_++;
    command.actorStableId = actorStableId_;
    command.dt = std::max(0.0f, dt);
    ++diagnostics_.commandsGenerated;
    return command;
}

void DirectOperativeController::refreshDiagnostics() {
    diagnostics_.mode = mode_;
    diagnostics_.toolMode = toolMode_;
    diagnostics_.actorStableId = actorStableId_;
    diagnostics_.nextSequence = nextSequence_;
}

std::vector<DirectOperativeCommand> DirectOperativeController::buildCommands(const DirectOperativeInputFrame& input,
                                                                             const DirectTargetSnapshot& target,
                                                                             const DirectSelectionState& selection) {
    std::vector<DirectOperativeCommand> out;
    out.reserve(6);

    if (input.toggleCommandPressed) {
        if (mode_ == PlayerControlMode::DirectOperative) {
            mode_ = PlayerControlMode::FortressCommand;
            out.push_back(makeCommand(DirectCommandKind::EnterFortressCommand));
        } else {
            mode_ = PlayerControlMode::DirectOperative;
            out.push_back(makeCommand(DirectCommandKind::EnterDirectOperative));
        }
        refreshDiagnostics();
        return out;
    }

    if (mode_ != PlayerControlMode::DirectOperative) {
        if (input.primaryDown || input.primaryPressed || input.secondaryPressed || input.interactPressed ||
            input.attackPressed || input.scanPressed || input.toggleToolPressed) {
            ++diagnostics_.actionsSuppressedByCommandMode;
        }
        refreshDiagnostics();
        return out;
    }

    if (input.toggleToolPressed) {
        toolMode_ = toolMode_ == DirectToolMode::Macro ? DirectToolMode::MicroSculpt : DirectToolMode::Macro;
        out.push_back(makeCommand(DirectCommandKind::ToggleToolMode));
    }

    if (input.attackPressed) out.push_back(makeCommand(DirectCommandKind::AttackRay));
    if (input.scanPressed) out.push_back(makeCommand(DirectCommandKind::ScanRay));

    if (input.interactPressed) {
        auto cmd = makeCommand(DirectCommandKind::Interact);
        cmd.interactionKind = target.interactionKind;
        cmd.targetStableId = target.interactionStableId;
        out.push_back(cmd);
    }

    if (toolMode_ == DirectToolMode::MicroSculpt) {
        if (input.primaryPressed && target.microHit) {
            auto cmd = makeCommand(DirectCommandKind::MicroSculptTarget);
            cmd.micro = target.microTarget;
            out.push_back(cmd);
        }
        // Micro mode intentionally does not accept placement or continuous
        // mining commands. The authoritative sculpt service decides material
        // gates and resource consequences.
        refreshDiagnostics();
        return out;
    }

    if (input.secondaryPressed && target.macroHit) {
        DirectCommandKind kind = DirectCommandKind::PlaceBlock;
        if (selection.placementMode == DirectPlacementMode::Machine) kind = DirectCommandKind::PlaceMachine;
        else if (selection.placementMode == DirectPlacementMode::Portal) kind = DirectCommandKind::PlacePortal;
        auto cmd = makeCommand(kind);
        cmd.cell = target.previousCell;
        cmd.contentId = selection.selectedContentId;
        out.push_back(cmd);
    }

    if (input.primaryDown && target.macroHit) {
        auto cmd = makeCommand(DirectCommandKind::MineTarget, input.dt);
        cmd.cell = target.hitCell;
        out.push_back(cmd);
    }

    refreshDiagnostics();
    return out;
}

const char* playerControlModeName(PlayerControlMode mode) {
    switch (mode) {
        case PlayerControlMode::DirectOperative: return "DIRECT OPERATIVE";
        case PlayerControlMode::FortressCommand: return "FORTRESS COMMAND";
        case PlayerControlMode::TacticalCommand: return "TACTICAL COMMAND";
    }
    return "UNKNOWN";
}

const char* directCommandKindName(DirectCommandKind kind) {
    switch (kind) {
        case DirectCommandKind::None: return "None";
        case DirectCommandKind::ToggleToolMode: return "ToggleToolMode";
        case DirectCommandKind::MineTarget: return "MineTarget";
        case DirectCommandKind::MicroSculptTarget: return "MicroSculptTarget";
        case DirectCommandKind::PlaceBlock: return "PlaceBlock";
        case DirectCommandKind::PlaceMachine: return "PlaceMachine";
        case DirectCommandKind::PlacePortal: return "PlacePortal";
        case DirectCommandKind::Interact: return "Interact";
        case DirectCommandKind::AttackRay: return "AttackRay";
        case DirectCommandKind::ScanRay: return "ScanRay";
        case DirectCommandKind::EnterFortressCommand: return "EnterFortressCommand";
        case DirectCommandKind::EnterDirectOperative: return "EnterDirectOperative";
    }
    return "Unknown";
}

} // namespace elysium
