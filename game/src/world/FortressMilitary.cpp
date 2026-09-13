// Intended function: imported world implementation for FortressMilitary; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/FortressMilitary.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace elysium {
namespace {

constexpr std::uint64_t kReservationLabel = 0x4d494c525356ULL; // MILRSV
constexpr std::uint64_t kJobLabel = 0x4d494c4a4f42ULL;         // MILJOB
constexpr std::uint64_t kRouteLabel = 0x4d494c525445ULL;       // MILRTE
constexpr std::uint64_t kIntentLabel = 0x4d494c494e54ULL;      // MILINT

float clamp01(float value) { return std::clamp(value, 0.0f, 1.0f); }

std::uint64_t stableActionId(std::uint64_t label,
                             std::uint64_t squad,
                             std::uint64_t member,
                             std::uint64_t detail) {
    auto h = mix64(label ^ squad);
    h = mix64(h ^ member);
    h = mix64(h ^ detail);
    return h == 0 ? 1 : h;
}

int totalStock(const MilitaryWorldState& state, int itemId) {
    int total = 0;
    for (const auto& stack : state.stock) if (stack.itemId == itemId) total += std::max(0, stack.count);
    return total;
}

int reservedStock(const MilitaryWorldState& state, int itemId) {
    int total = 0;
    for (const auto& reservation : state.reservations)
        if (reservation.itemId == itemId) total += std::max(0, reservation.count);
    return total;
}

int reservedFor(const MilitaryWorldState& state,
                std::uint64_t squadId,
                std::uint64_t memberId,
                MilitaryReservationKind kind,
                int itemId) {
    int total = 0;
    for (const auto& reservation : state.reservations) {
        if (reservation.squadStableId == squadId && reservation.memberStableId == memberId &&
            reservation.kind == kind && reservation.itemId == itemId)
            total += std::max(0, reservation.count);
    }
    return total;
}

const MilitaryMemberState* findMember(const MilitaryWorldState& state, std::uint64_t stableId) {
    const auto it = std::find_if(state.members.begin(), state.members.end(),
                                 [stableId](const auto& member) { return member.stableId == stableId; });
    return it == state.members.end() ? nullptr : &*it;
}

MilitaryMemberState* findMember(MilitaryWorldState& state, std::uint64_t stableId) {
    const auto it = std::find_if(state.members.begin(), state.members.end(),
                                 [stableId](const auto& member) { return member.stableId == stableId; });
    return it == state.members.end() ? nullptr : &*it;
}

MilitarySquadEntityState* findSquad(MilitaryWorldState& state, std::uint64_t stableId) {
    const auto it = std::find_if(state.squads.begin(), state.squads.end(),
                                 [stableId](const auto& squad) { return squad.identity.stableId == stableId; });
    return it == state.squads.end() ? nullptr : &*it;
}

MilitaryRoutine routineForAlert(MilitaryRoutine scheduled, MilitaryAlertLevel alert) {
    switch (alert) {
        case MilitaryAlertLevel::Normal: return scheduled;
        case MilitaryAlertLevel::Ready: return MilitaryRoutine::Ready;
        case MilitaryAlertLevel::Siege: return MilitaryRoutine::Siege;
        case MilitaryAlertLevel::Evacuate: return MilitaryRoutine::EvacuationSupport;
        case MilitaryAlertLevel::Isolate: return MilitaryRoutine::Guard;
    }
    return scheduled;
}

MilitaryAlertLevel maxAlert(MilitaryAlertLevel a, MilitaryAlertLevel b) {
    return static_cast<int>(a) >= static_cast<int>(b) ? a : b;
}

MilitaryAlertLevel alertForThreat(const MilitaryThreatIntent& threat) {
    const float s = clamp01(threat.severity);
    switch (threat.kind) {
        case MilitaryThreatKind::ImperialRegisterAction:
        case MilitaryThreatKind::Rift:
            return s >= 0.45f ? MilitaryAlertLevel::Siege : MilitaryAlertLevel::Ready;
        case MilitaryThreatKind::Raiders:
            return s >= 0.80f ? MilitaryAlertLevel::Siege : MilitaryAlertLevel::Ready;
        case MilitaryThreatKind::Infiltration:
            return MilitaryAlertLevel::Isolate;
        case MilitaryThreatKind::Wildlife:
            return s >= 0.55f ? MilitaryAlertLevel::Ready : MilitaryAlertLevel::Normal;
        case MilitaryThreatKind::Unknown:
            return s >= 0.75f ? MilitaryAlertLevel::Ready : MilitaryAlertLevel::Normal;
    }
    return MilitaryAlertLevel::Normal;
}

MilitaryReadinessSummaryComponent calculateReadiness(const MilitaryWorldState& state,
                                                      const MilitarySquadEntityState& squad,
                                                      const std::vector<MilitaryDefenseAsset>& defenseAssets,
                                                      MilitaryPlan& out) {
    MilitaryReadinessSummaryComponent r{};
    const int memberCount = static_cast<int>(squad.membership.members.size());
    if (memberCount == 0) {
        r.equipmentRatio = r.ammoRatio = r.trainingRatio = r.healthRatio = r.positionRatio = r.infrastructureRatio = r.overall = 0.0f;
        out.diagnostics.push_back({squad.identity.stableId, MilitaryDiagnosticCode::MissingMember, 0, 0, 1});
        return r;
    }

    int presentMembers = 0;
    int healthy = 0;
    int positioned = 0;
    float trainingSum = 0.0f;
    int requiredEquipment = 0;
    int heldEquipment = 0;
    int ammoRequired = 0;
    int ammoHeld = 0;

    for (const auto memberId : squad.membership.members) {
        const auto* member = findMember(state, memberId);
        if (!member) {
            out.diagnostics.push_back({squad.identity.stableId, MilitaryDiagnosticCode::MissingMember, memberId, 0, 1});
            continue;
        }
        ++presentMembers;
        if (!member->incapacitated && member->healthFraction >= 0.60f) ++healthy;
        else {
            ++r.woundedMembers;
            out.diagnostics.push_back({squad.identity.stableId, MilitaryDiagnosticCode::WoundedMember, memberId, 0, 1});
        }
        if (member->hasCell) ++positioned;
        trainingSum += clamp01(member->trainingProficiency / std::max(0.01f, squad.trainingPolicy.targetProficiency));

        for (const auto& req : squad.uniform.requirements) {
            if (req.optional || req.itemId <= 0 || req.countPerMember <= 0) continue;
            requiredEquipment += req.countPerMember;
            heldEquipment += std::min(req.countPerMember,
                reservedFor(state, squad.identity.stableId, memberId, MilitaryReservationKind::Uniform, req.itemId));
        }
        if (squad.ammo.ammoItemId > 0 && squad.ammo.roundsPerMember > 0) {
            ammoRequired += squad.ammo.roundsPerMember;
            ammoHeld += std::min(squad.ammo.roundsPerMember,
                reservedFor(state, squad.identity.stableId, memberId, MilitaryReservationKind::Ammunition, squad.ammo.ammoItemId));
        }
    }

    if (requiredEquipment > 0) {
        r.equipmentRatio = clamp01(static_cast<float>(heldEquipment) / static_cast<float>(requiredEquipment));
        r.missingEquipment = requiredEquipment - heldEquipment;
    }
    if (ammoRequired > 0) {
        r.ammoRatio = clamp01(static_cast<float>(ammoHeld) / static_cast<float>(ammoRequired));
        r.missingAmmo = ammoRequired - ammoHeld;
    }
    r.trainingRatio = presentMembers > 0 ? clamp01(trainingSum / static_cast<float>(presentMembers)) : 0.0f;
    r.healthRatio = clamp01(static_cast<float>(healthy) / static_cast<float>(memberCount));
    r.positionRatio = clamp01(static_cast<float>(positioned) / static_cast<float>(memberCount));

    int supportKinds = 0;
    bool sawSensor = false, sawTurret = false, sawShield = false;
    bool operationalSensor = false, operationalTurret = false;
    float bestShield = 0.0f;
    for (const auto& asset : defenseAssets) {
        if (asset.kind == MilitaryDefenseAssetKind::Sensor) {
            sawSensor = true;
            operationalSensor = operationalSensor || (asset.enabled && asset.powered);
        } else if (asset.kind == MilitaryDefenseAssetKind::Turret) {
            sawTurret = true;
            operationalTurret = operationalTurret || (asset.enabled && asset.powered && asset.ammunition > 0);
        } else if (asset.kind == MilitaryDefenseAssetKind::Shield) {
            sawShield = true;
            if (asset.enabled && asset.powered) bestShield = std::max(bestShield, clamp01(asset.charge));
        }
    }
    float supportScore = 0.0f;
    if (sawSensor) { ++supportKinds; if (operationalSensor) supportScore += 1.0f; }
    if (sawTurret) { ++supportKinds; if (operationalTurret) supportScore += 1.0f; }
    if (sawShield) { ++supportKinds; supportScore += bestShield; }
    if (supportKinds > 0) r.infrastructureRatio = clamp01(supportScore / static_cast<float>(supportKinds));

    // TUNING: readiness weights are intentionally centralized and inspectable.
    r.overall = clamp01(0.30f * r.equipmentRatio + 0.20f * r.ammoRatio + 0.20f * r.trainingRatio +
                        0.15f * r.healthRatio + 0.10f * r.positionRatio + 0.05f * r.infrastructureRatio);
    return r;
}

void emitReservationNeeds(const MilitaryWorldState& state,
                          const MilitarySquadEntityState& squad,
                          MilitaryPlan& out) {
    std::unordered_map<int, int> plannedByItem;
    auto availableFor = [&](int itemId) {
        return std::max(0, totalStock(state, itemId) - reservedStock(state, itemId) - plannedByItem[itemId]);
    };

    std::vector<std::uint64_t> members = squad.membership.members;
    std::sort(members.begin(), members.end());
    for (const auto memberId : members) {
        if (!findMember(state, memberId)) continue;
        for (const auto& req : squad.uniform.requirements) {
            if (req.optional || req.itemId <= 0 || req.countPerMember <= 0) continue;
            const int have = reservedFor(state, squad.identity.stableId, memberId,
                                         MilitaryReservationKind::Uniform, req.itemId);
            const int need = std::max(0, req.countPerMember - have);
            if (need == 0) continue;
            const int granted = std::min(need, availableFor(req.itemId));
            if (granted > 0) {
                MilitaryReservation reservation{};
                reservation.kind = MilitaryReservationKind::Uniform;
                reservation.squadStableId = squad.identity.stableId;
                reservation.memberStableId = memberId;
                reservation.itemId = req.itemId;
                reservation.count = granted;
                reservation.priority = 60;
                reservation.preemptible = false;
                reservation.stableId = stableActionId(kReservationLabel, squad.identity.stableId, memberId,
                                                      static_cast<std::uint64_t>(static_cast<std::uint32_t>(req.itemId)));
                out.reservationCommands.push_back({reservation});
                plannedByItem[req.itemId] += granted;
                out.jobs.push_back({stableActionId(kJobLabel, squad.identity.stableId, memberId,
                                                   static_cast<std::uint64_t>(static_cast<std::uint32_t>(req.itemId))),
                                    MilitaryJobKind::Equip, squad.identity.stableId, memberId,
                                    req.itemId, granted, false, {}});
            }
            if (granted < need) {
                out.diagnostics.push_back({squad.identity.stableId, MilitaryDiagnosticCode::MissingEquipment,
                                           memberId, req.itemId, need - granted});
            }
        }

        if (squad.ammo.ammoItemId > 0 && squad.ammo.roundsPerMember > 0) {
            const int have = reservedFor(state, squad.identity.stableId, memberId,
                                         MilitaryReservationKind::Ammunition, squad.ammo.ammoItemId);
            const int need = std::max(0, squad.ammo.roundsPerMember - have);
            const int granted = std::min(need, availableFor(squad.ammo.ammoItemId));
            if (granted > 0) {
                MilitaryReservation reservation{};
                reservation.kind = MilitaryReservationKind::Ammunition;
                reservation.squadStableId = squad.identity.stableId;
                reservation.memberStableId = memberId;
                reservation.itemId = squad.ammo.ammoItemId;
                reservation.count = granted;
                reservation.priority = 70;
                reservation.preemptible = false;
                reservation.stableId = stableActionId(kReservationLabel ^ 0xAULL, squad.identity.stableId, memberId,
                                                      static_cast<std::uint64_t>(static_cast<std::uint32_t>(squad.ammo.ammoItemId)));
                out.reservationCommands.push_back({reservation});
                plannedByItem[squad.ammo.ammoItemId] += granted;
                out.jobs.push_back({stableActionId(kJobLabel ^ 0xAULL, squad.identity.stableId, memberId,
                                                   static_cast<std::uint64_t>(static_cast<std::uint32_t>(squad.ammo.ammoItemId))),
                                    MilitaryJobKind::ResupplyAmmo, squad.identity.stableId, memberId,
                                    squad.ammo.ammoItemId, granted, false, {}});
            }
            if (granted < need) {
                out.diagnostics.push_back({squad.identity.stableId, MilitaryDiagnosticCode::MissingAmmo,
                                           memberId, squad.ammo.ammoItemId, need - granted});
            }
        }
    }

    if (squad.ammo.ammoItemId > 0 && squad.ammo.squadReserveRounds > 0) {
        const int have = reservedFor(state, squad.identity.stableId, 0,
                                     MilitaryReservationKind::Ammunition, squad.ammo.ammoItemId);
        const int need = std::max(0, squad.ammo.squadReserveRounds - have);
        const int granted = std::min(need, availableFor(squad.ammo.ammoItemId));
        if (granted > 0) {
            MilitaryReservation reservation{};
            reservation.kind = MilitaryReservationKind::Ammunition;
            reservation.squadStableId = squad.identity.stableId;
            reservation.memberStableId = 0;
            reservation.itemId = squad.ammo.ammoItemId;
            reservation.count = granted;
            reservation.priority = 75;
            reservation.preemptible = false;
            reservation.stableId = stableActionId(kReservationLabel ^ 0xBULL, squad.identity.stableId, 0,
                                                  static_cast<std::uint64_t>(static_cast<std::uint32_t>(squad.ammo.ammoItemId)));
            out.reservationCommands.push_back({reservation});
            plannedByItem[squad.ammo.ammoItemId] += granted;
            out.jobs.push_back({stableActionId(kJobLabel ^ 0xBULL, squad.identity.stableId, 0,
                                               static_cast<std::uint64_t>(static_cast<std::uint32_t>(squad.ammo.ammoItemId))),
                                MilitaryJobKind::ResupplyAmmo, squad.identity.stableId, 0,
                                squad.ammo.ammoItemId, granted, false, {}});
        }
        if (granted < need) {
            out.diagnostics.push_back({squad.identity.stableId, MilitaryDiagnosticCode::MissingAmmo,
                                       0, squad.ammo.ammoItemId, need - granted});
        }
    }
}


int desiredReservationCount(const MilitarySquadEntityState& squad, const MilitaryReservation& reservation) {
    if (reservation.squadStableId != squad.identity.stableId) return 0;

    if (reservation.kind == MilitaryReservationKind::Uniform) {
        if (reservation.memberStableId == 0 ||
            std::find(squad.membership.members.begin(), squad.membership.members.end(), reservation.memberStableId) ==
                squad.membership.members.end()) return 0;
        int desired = 0;
        for (const auto& req : squad.uniform.requirements) {
            if (!req.optional && req.itemId == reservation.itemId && req.countPerMember > 0)
                desired += req.countPerMember;
        }
        return desired;
    }

    if (reservation.itemId != squad.ammo.ammoItemId || squad.ammo.ammoItemId <= 0) return 0;
    if (reservation.memberStableId == 0) return std::max(0, squad.ammo.squadReserveRounds);
    if (std::find(squad.membership.members.begin(), squad.membership.members.end(), reservation.memberStableId) ==
        squad.membership.members.end()) return 0;
    return std::max(0, squad.ammo.roundsPerMember);
}

void emitReservationReleases(const MilitaryWorldState& state,
                             const MilitarySquadEntityState& squad,
                             MilitaryPlan& out) {
    for (const auto& reservation : state.reservations) {
        if (reservation.squadStableId != squad.identity.stableId) continue;
        const int desired = desiredReservationCount(squad, reservation);
        const int current = std::max(0, reservation.count);
        if (current <= desired) continue;
        out.reservationReleases.push_back({reservation.stableId, squad.identity.stableId, current - desired});
    }
}

std::optional<SurfaceCellAddress> targetForRoutine(const MilitarySquadEntityState& squad,
                                                   MilitaryRoutine routine) {
    if (squad.currentOrder.active && squad.currentOrder.hasTargetCell) return squad.currentOrder.targetCell;
    switch (routine) {
        case MilitaryRoutine::Training:
            if (squad.barracks.hasTrainingCell) return squad.barracks.trainingCell;
            break;
        case MilitaryRoutine::Ready:
            if (squad.barracks.hasRallyCell) return squad.barracks.rallyCell;
            break;
        case MilitaryRoutine::Guard:
        case MilitaryRoutine::Siege:
            if (!squad.posts.guardCells.empty()) return squad.posts.guardCells.front();
            if (squad.barracks.hasRallyCell) return squad.barracks.rallyCell;
            break;
        case MilitaryRoutine::Patrol:
            if (!squad.posts.patrolRoute.empty()) return squad.posts.patrolRoute.front();
            break;
        case MilitaryRoutine::EvacuationSupport:
            if (squad.posts.hasEvacuationCell) return squad.posts.evacuationCell;
            break;
        case MilitaryRoutine::Reserve:
        case MilitaryRoutine::Expedition:
            break;
    }
    return std::nullopt;
}

MilitaryJobKind jobForRoutine(MilitaryRoutine routine, const MilitaryOrderComponent& order) {
    if (order.active) {
        switch (order.type) {
            case TacticalCommandType::Rally: return MilitaryJobKind::Rally;
            case TacticalCommandType::Patrol: return MilitaryJobKind::Patrol;
            case TacticalCommandType::Breach: return MilitaryJobKind::Breach;
            case TacticalCommandType::Evacuate: return MilitaryJobKind::Evacuate;
            case TacticalCommandType::Isolate: return MilitaryJobKind::Isolate;
            case TacticalCommandType::Guard: return MilitaryJobKind::Guard;
            case TacticalCommandType::SetAlert: break;
        }
    }
    switch (routine) {
        case MilitaryRoutine::Training: return MilitaryJobKind::Drill;
        case MilitaryRoutine::Patrol: return MilitaryJobKind::Patrol;
        case MilitaryRoutine::Guard: return MilitaryJobKind::Guard;
        case MilitaryRoutine::Ready: return MilitaryJobKind::Rally;
        case MilitaryRoutine::Siege: return MilitaryJobKind::Guard;
        case MilitaryRoutine::EvacuationSupport: return MilitaryJobKind::Evacuate;
        case MilitaryRoutine::Reserve:
        case MilitaryRoutine::Expedition: return MilitaryJobKind::Rally;
    }
    return MilitaryJobKind::Rally;
}

void sortPlan(MilitaryPlan& plan) {
    std::sort(plan.squadMutations.begin(), plan.squadMutations.end(), [](const auto& a, const auto& b) {
        return a.squadStableId < b.squadStableId;
    });
    std::sort(plan.trainingMutations.begin(), plan.trainingMutations.end(), [](const auto& a, const auto& b) {
        return a.memberStableId < b.memberStableId;
    });
    std::sort(plan.reservationCommands.begin(), plan.reservationCommands.end(), [](const auto& a, const auto& b) {
        return std::tie(a.reservation.squadStableId, a.reservation.memberStableId, a.reservation.itemId, a.reservation.stableId) <
               std::tie(b.reservation.squadStableId, b.reservation.memberStableId, b.reservation.itemId, b.reservation.stableId);
    });
    std::sort(plan.reservationReleases.begin(), plan.reservationReleases.end(), [](const auto& a, const auto& b) {
        return std::tie(a.squadStableId, a.reservationStableId, a.releaseCount) <
               std::tie(b.squadStableId, b.reservationStableId, b.releaseCount);
    });
    std::sort(plan.jobs.begin(), plan.jobs.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    std::sort(plan.routes.begin(), plan.routes.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    std::sort(plan.infrastructure.begin(), plan.infrastructure.end(), [](const auto& a, const auto& b) {
        return std::tie(a.targetStableId, a.action, a.stableId) < std::tie(b.targetStableId, b.action, b.stableId);
    });
    std::sort(plan.events.begin(), plan.events.end(), [](const auto& a, const auto& b) {
        return std::tie(a.sequence, a.squadStableId, a.relatedStableId) < std::tie(b.sequence, b.squadStableId, b.relatedStableId);
    });
    std::sort(plan.diagnostics.begin(), plan.diagnostics.end(), [](const auto& a, const auto& b) {
        return std::tie(a.squadStableId, a.code, a.relatedStableId, a.itemId) <
               std::tie(b.squadStableId, b.code, b.relatedStableId, b.itemId);
    });
}

bool readBool(std::istream& in, bool& value) {
    int v = 0;
    if (!(in >> v) || (v != 0 && v != 1)) return false;
    value = v != 0;
    return true;
}

void writeCell(std::ostream& out, const SurfaceCellAddress& cell) {
    out << ' ' << static_cast<int>(cell.face) << ' ' << cell.u << ' ' << cell.v << ' ' << cell.radial;
}

bool readCell(std::istream& in, SurfaceCellAddress& cell) {
    int face = 0;
    if (!(in >> face >> cell.u >> cell.v >> cell.radial) || face < 0 || face > 5) return false;
    cell.face = static_cast<CubeFace>(face);
    return true;
}

} // namespace

const char* militaryRoutineName(MilitaryRoutine value) {
    switch (value) {
        case MilitaryRoutine::Reserve: return "reserve";
        case MilitaryRoutine::Training: return "training";
        case MilitaryRoutine::Patrol: return "patrol";
        case MilitaryRoutine::Guard: return "guard";
        case MilitaryRoutine::Ready: return "ready";
        case MilitaryRoutine::Siege: return "siege";
        case MilitaryRoutine::Expedition: return "expedition";
        case MilitaryRoutine::EvacuationSupport: return "evacuation-support";
    }
    return "unknown";
}

const char* militaryAlertName(MilitaryAlertLevel value) {
    switch (value) {
        case MilitaryAlertLevel::Normal: return "normal";
        case MilitaryAlertLevel::Ready: return "ready";
        case MilitaryAlertLevel::Siege: return "siege";
        case MilitaryAlertLevel::Evacuate: return "evacuate";
        case MilitaryAlertLevel::Isolate: return "isolate";
    }
    return "unknown";
}

const char* tacticalCommandName(TacticalCommandType value) {
    switch (value) {
        case TacticalCommandType::SetAlert: return "set-alert";
        case TacticalCommandType::Rally: return "rally";
        case TacticalCommandType::Patrol: return "patrol";
        case TacticalCommandType::Breach: return "breach";
        case TacticalCommandType::Evacuate: return "evacuate";
        case TacticalCommandType::Isolate: return "isolate";
        case TacticalCommandType::Guard: return "guard";
    }
    return "unknown";
}

const char* militaryThreatName(MilitaryThreatKind value) {
    switch (value) {
        case MilitaryThreatKind::Unknown: return "unknown";
        case MilitaryThreatKind::Wildlife: return "wildlife";
        case MilitaryThreatKind::Raiders: return "raiders";
        case MilitaryThreatKind::ImperialRegisterAction: return "register-action";
        case MilitaryThreatKind::Infiltration: return "infiltration";
        case MilitaryThreatKind::Rift: return "rift";
    }
    return "unknown";
}

const char* militaryDiagnosticName(MilitaryDiagnosticCode value) {
    switch (value) {
        case MilitaryDiagnosticCode::Ready: return "ready";
        case MilitaryDiagnosticCode::MissingMember: return "missing-member";
        case MilitaryDiagnosticCode::MissingEquipment: return "missing-equipment";
        case MilitaryDiagnosticCode::MissingAmmo: return "missing-ammo";
        case MilitaryDiagnosticCode::MissingBarracks: return "missing-barracks";
        case MilitaryDiagnosticCode::MissingTarget: return "missing-target";
        case MilitaryDiagnosticCode::UnsafeTarget: return "unsafe-target";
        case MilitaryDiagnosticCode::WoundedMember: return "wounded-member";
        case MilitaryDiagnosticCode::DefenseOffline: return "defense-offline";
        case MilitaryDiagnosticCode::ReservationBlocked: return "reservation-blocked";
    }
    return "unknown";
}

std::string militaryDiagnosticText(const MilitaryDiagnostic& diagnostic) {
    std::ostringstream out;
    out << "squad " << diagnostic.squadStableId << ": " << militaryDiagnosticName(diagnostic.code);
    if (diagnostic.relatedStableId) out << " related=" << diagnostic.relatedStableId;
    if (diagnostic.itemId) out << " item=" << diagnostic.itemId;
    if (diagnostic.amount) out << " amount=" << diagnostic.amount;
    return out.str();
}


MilitaryPlan FortressMilitarySystem::plan(const MilitaryWorldState& state, const MilitaryUpdateInput& input) const {
    MilitaryPlan out;
    const int hour = ((input.hourOfDay % 24) + 24) % 24;

    std::vector<const MilitarySquadEntityState*> squads;
    squads.reserve(state.squads.size());
    for (const auto& squad : state.squads) squads.push_back(&squad);
    std::sort(squads.begin(), squads.end(), [](const auto* a, const auto* b) {
        return a->identity.stableId < b->identity.stableId;
    });

    std::vector<MilitaryThreatIntent> threats = input.threats;
    std::sort(threats.begin(), threats.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    std::vector<MilitaryTacticalCommand> commands = input.tacticalCommands;
    std::sort(commands.begin(), commands.end(), [](const auto& a, const auto& b) {
        return std::tie(a.sequence, a.squadStableId, a.type) < std::tie(b.sequence, b.squadStableId, b.type);
    });

    for (const auto* original : squads) {
        MilitarySquadEntityState working = *original;
        const MilitaryRoutine scheduled = working.schedule.hourly[static_cast<std::size_t>(hour)];
        MilitaryAlertStateComponent alert = working.alert;
        MilitaryOrderComponent order = working.currentOrder;
        MilitaryMissionComponent mission = working.mission;
        float missionSeverity = -1.0f;

        for (const auto& threat : threats) {
            const auto desired = alertForThreat(threat);
            if (desired != MilitaryAlertLevel::Normal) {
                out.events.push_back({MilitaryEventType::ThreatDetected, threat.stableId,
                                      working.identity.stableId, threat.stableId});
                if (clamp01(threat.severity) > missionSeverity ||
                    (clamp01(threat.severity) == missionSeverity && threat.stableId < mission.threatStableId)) {
                    missionSeverity = clamp01(threat.severity);
                    mission.missionStableId = stableActionId(0x4d494c4d534eULL, working.identity.stableId, threat.stableId, static_cast<std::uint64_t>(threat.kind));
                    mission.threatKind = threat.kind;
                    mission.threatStableId = threat.stableId;
                    mission.active = true;
                    mission.hasTargetCell = threat.hasCell;
                    mission.targetCell = threat.cell;
                }
                if (static_cast<int>(desired) > static_cast<int>(alert.level)) {
                    alert.level = maxAlert(alert.level, desired);
                    alert.sourceThreatStableId = threat.stableId;
                }
            }
        }

        for (const auto& command : commands) {
            if (command.squadStableId != working.identity.stableId) continue;
            const auto oldAlert = alert.level;
            if (command.type == TacticalCommandType::SetAlert) {
                alert.level = command.alert;
                alert.sourceThreatStableId = command.threatStableId;
            } else {
                order.sequence = command.sequence;
                order.type = command.type;
                order.active = true;
                order.hasTargetCell = command.hasTargetCell;
                order.targetCell = command.targetCell;
                order.targetStableId = command.targetStableId;
                order.threatStableId = command.threatStableId;
                mission.missionStableId = stableActionId(0x4d494c4d534eULL, working.identity.stableId, command.sequence, static_cast<std::uint64_t>(command.type));
                mission.threatStableId = command.threatStableId;
                mission.active = true;
                mission.hasTargetCell = command.hasTargetCell;
                mission.targetCell = command.targetCell;
                if (command.type == TacticalCommandType::Evacuate) alert.level = MilitaryAlertLevel::Evacuate;
                if (command.type == TacticalCommandType::Isolate) alert.level = MilitaryAlertLevel::Isolate;
                if (command.type == TacticalCommandType::Breach) alert.level = maxAlert(alert.level, MilitaryAlertLevel::Siege);
                if (command.type == TacticalCommandType::Rally || command.type == TacticalCommandType::Guard ||
                    command.type == TacticalCommandType::Patrol)
                    alert.level = maxAlert(alert.level, MilitaryAlertLevel::Ready);
                out.events.push_back({MilitaryEventType::SquadOrderIssued, command.sequence,
                                      working.identity.stableId, command.targetStableId});
            }
            if (oldAlert != alert.level)
                out.events.push_back({MilitaryEventType::AlertChanged, command.sequence,
                                      working.identity.stableId, command.threatStableId});
        }

        const MilitaryRoutine effective = routineForAlert(scheduled, alert.level);
        working.scheduledRoutine = scheduled;
        working.effectiveRoutine = effective;
        working.alert = alert;
        working.currentOrder = order;
        working.mission = mission;

        emitReservationReleases(state, working, out);
        emitReservationNeeds(state, working, out);

        // Completed training jobs are the only path that changes proficiency;
        // merely being scheduled to train never grants skill for free.
        for (const auto& completion : input.completedJobs) {
            if (completion.squadStableId != working.identity.stableId || !completion.succeeded ||
                completion.kind != MilitaryJobKind::Drill || completion.memberStableId == 0) continue;
            const float delta = std::max(0.0f, completion.workHours) * working.trainingPolicy.drillGainPerHour;
            if (delta > 0.0f) out.trainingMutations.push_back({completion.memberStableId, delta});
        }

        if (effective == MilitaryRoutine::Training && !working.barracks.hasTrainingCell)
            out.diagnostics.push_back({working.identity.stableId, MilitaryDiagnosticCode::MissingBarracks,
                                       working.barracks.barracksStableId, 0, 1});

        const auto target = targetForRoutine(working, effective);
        const MilitaryJobKind movementJob = jobForRoutine(effective, order);
        if ((effective == MilitaryRoutine::Training || effective == MilitaryRoutine::Patrol ||
             effective == MilitaryRoutine::Guard || effective == MilitaryRoutine::Ready ||
             effective == MilitaryRoutine::Siege || effective == MilitaryRoutine::EvacuationSupport || order.active) && !target) {
            out.diagnostics.push_back({working.identity.stableId, MilitaryDiagnosticCode::MissingTarget, 0, 0, 1});
        }

        if (target) {
            std::vector<std::uint64_t> memberIds = working.membership.members;
            std::sort(memberIds.begin(), memberIds.end());
            for (const auto memberId : memberIds) {
                const auto* member = findMember(state, memberId);
                if (!member || !member->available || member->incapacitated) continue;
                MilitaryJobRequest job{};
                job.stableId = stableActionId(kJobLabel ^ static_cast<std::uint64_t>(movementJob),
                                              working.identity.stableId, memberId, order.sequence);
                job.kind = movementJob;
                job.squadStableId = working.identity.stableId;
                job.memberStableId = memberId;
                job.hasTargetCell = true;
                job.targetCell = *target;
                out.jobs.push_back(job);

                if (member->hasCell) {
                    MilitaryRouteRequest route{};
                    route.stableId = stableActionId(kRouteLabel ^ static_cast<std::uint64_t>(movementJob),
                                                    working.identity.stableId, memberId, order.sequence);
                    route.squadStableId = working.identity.stableId;
                    route.memberStableId = memberId;
                    route.purpose = movementJob;
                    route.from = member->cell;
                    route.goal = *target;
                    route.policy.allowRestrictedZones = effective == MilitaryRoutine::Siege || movementJob == MilitaryJobKind::Breach;
                    route.policy.requireBreathable = movementJob != MilitaryJobKind::Breach;
                    route.policy.avoidHighHazard = true;
                    route.policy.allowBreach = movementJob == MilitaryJobKind::Breach;
                    out.routes.push_back(route);
                }
            }
        }

        // Wounded personnel create medevac jobs only when a staging point exists.
        if (working.posts.hasMedevacCell) {
            for (const auto memberId : working.membership.members) {
                const auto* member = findMember(state, memberId);
                if (!member || (member->healthFraction >= 0.35f && !member->incapacitated)) continue;
                out.jobs.push_back({stableActionId(kJobLabel ^ 0x4d4544ULL, working.identity.stableId, memberId, 0),
                                    MilitaryJobKind::Medevac, working.identity.stableId, memberId, 0, 0,
                                    true, working.posts.medevacCell});
            }
        }

        // Defense integration is intent-only. SurfaceInfrastructure remains the
        // authority on network power, turret ammo, shield charge and portal voxels.
        if (alert.level == MilitaryAlertLevel::Ready || alert.level == MilitaryAlertLevel::Siege) {
            bool usefulDefense = false;
            for (const auto& asset : input.defenseAssets) {
                if (asset.kind == MilitaryDefenseAssetKind::Sensor || asset.kind == MilitaryDefenseAssetKind::Turret ||
                    asset.kind == MilitaryDefenseAssetKind::Shield) {
                    if (asset.powered && (asset.kind != MilitaryDefenseAssetKind::Turret || asset.ammunition > 0) &&
                        (asset.kind != MilitaryDefenseAssetKind::Shield || asset.charge > 0.0f)) usefulDefense = true;
                    if (!asset.enabled)
                        out.infrastructure.push_back({stableActionId(kIntentLabel, working.identity.stableId,
                                                                    asset.stableId, static_cast<std::uint64_t>(asset.kind)),
                                                      MilitaryInfrastructureAction::EnableMachine, asset.stableId});
                }
            }
            if (!input.defenseAssets.empty() && !usefulDefense)
                out.diagnostics.push_back({working.identity.stableId, MilitaryDiagnosticCode::DefenseOffline, 0, 0, 1});
        }
        if (order.active && order.type == TacticalCommandType::Isolate && order.targetStableId != 0) {
            out.infrastructure.push_back({stableActionId(kIntentLabel ^ 0xC105EULL, working.identity.stableId,
                                                        order.targetStableId, order.sequence),
                                          MilitaryInfrastructureAction::ClosePortal, order.targetStableId});
        }

        const auto readiness = calculateReadiness(state, working, input.defenseAssets, out);
        out.squadMutations.push_back({working.identity.stableId, scheduled, effective, alert, order, mission, readiness});
    }

    sortPlan(out);
    return out;
}

MilitaryCommitResult FortressMilitarySystem::commit(MilitaryWorldState& state, const MilitaryPlan& plan) const {
    MilitaryCommitResult result{};
    for (const auto& mutation : plan.squadMutations) {
        if (auto* squad = findSquad(state, mutation.squadStableId)) {
            squad->scheduledRoutine = mutation.scheduledRoutine;
            squad->effectiveRoutine = mutation.effectiveRoutine;
            squad->alert = mutation.alert;
            squad->currentOrder = mutation.currentOrder;
            squad->mission = mutation.mission;
            squad->readiness = mutation.readiness;
            ++result.squadMutations;
        }
    }
    // Release obsolete or excess quartermaster claims before adding new ones.
    // Releases are scoped to the owning squad and stable reservation ID, so a
    // stale plan cannot free another squad's stock.
    for (const auto& release : plan.reservationReleases) {
        auto it = std::find_if(state.reservations.begin(), state.reservations.end(), [&](const auto& r) {
            return r.stableId == release.reservationStableId && r.squadStableId == release.squadStableId;
        });
        if (it == state.reservations.end() || release.releaseCount <= 0) continue;
        const int amount = std::min(std::max(0, it->count), release.releaseCount);
        if (amount <= 0) continue;
        it->count -= amount;
        result.reservationsReleased += amount;
        if (it->count <= 0) state.reservations.erase(it);
    }

    for (const auto& mutation : plan.trainingMutations) {
        if (auto* member = findMember(state, mutation.memberStableId)) {
            member->trainingProficiency = clamp01(member->trainingProficiency + std::max(0.0f, mutation.delta));
            ++result.trainingMutations;
        }
    }

    // Recheck stock at publication time. Existing reservations are never stolen
    // by this commit path, satisfying the default non-preemption policy.
    for (const auto& command : plan.reservationCommands) {
        const auto& reservation = command.reservation;
        const auto duplicate = std::find_if(state.reservations.begin(), state.reservations.end(), [&](const auto& r) {
            return r.stableId == reservation.stableId;
        });
        if (duplicate != state.reservations.end()) continue;
        const int available = std::max(0, totalStock(state, reservation.itemId) - reservedStock(state, reservation.itemId));
        if (reservation.count <= 0 || available < reservation.count) {
            ++result.reservationConflicts;
            continue;
        }
        state.reservations.push_back(reservation);
        ++result.reservationsAdded;
    }
    std::sort(state.reservations.begin(), state.reservations.end(), [](const auto& a, const auto& b) {
        return a.stableId < b.stableId;
    });
    return result;
}

std::vector<MilitaryDefenseAsset> snapshotSurfaceDefense(const SurfaceInfrastructure& infrastructure) {
    std::vector<MilitaryDefenseAsset> out;
    for (const auto& object : infrastructure.objects()) {
        MilitaryDefenseAssetKind kind{};
        bool relevant = true;
        switch (object.type) {
            case MachineType::SensorMast: kind = MilitaryDefenseAssetKind::Sensor; break;
            case MachineType::Turret: kind = MilitaryDefenseAssetKind::Turret; break;
            case MachineType::ShieldPylon: kind = MilitaryDefenseAssetKind::Shield; break;
            case MachineType::AtmosphereUnit: kind = MilitaryDefenseAssetKind::Atmosphere; break;
            default: relevant = false; break;
        }
        if (!relevant) continue;
        float charge = 1.0f;
        if (kind == MilitaryDefenseAssetKind::Shield) charge = clamp01(object.shieldCharge / 100.0f);
        if (kind == MilitaryDefenseAssetKind::Atmosphere) charge = object.roomSealed ? clamp01(object.roomPressure) : 0.0f;
        out.push_back({object.stableId, kind, object.enabled, object.powered, object.ammo, charge});
    }
    for (const auto& portal : infrastructure.portals())
        out.push_back({portal.stableId, MilitaryDefenseAssetKind::Portal, !portal.open, true, 0, portal.open ? 0.0f : 1.0f});
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    return out;
}

int applySurfaceMilitaryInfrastructureIntents(PlanetSurface& planet,
                                              SurfaceInfrastructure& infrastructure,
                                              const std::vector<MilitaryInfrastructureIntent>& intents) {
    std::vector<MilitaryInfrastructureIntent> sorted = intents;
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
        return std::tie(a.targetStableId, a.action, a.stableId) < std::tie(b.targetStableId, b.action, b.stableId);
    });
    int applied = 0;
    std::unordered_set<std::uint64_t> seen;
    for (const auto& intent : sorted) {
        if (!seen.insert(intent.stableId).second) continue;
        if (intent.action == MilitaryInfrastructureAction::EnableMachine) {
            if (auto* machine = infrastructure.find(intent.targetStableId)) {
                if (!machine->enabled) { machine->enabled = true; ++applied; }
            }
        } else if (intent.action == MilitaryInfrastructureAction::ClosePortal) {
            const auto* portal = infrastructure.findPortal(intent.targetStableId);
            if (portal && portal->open && infrastructure.setPortalOpen(planet, intent.targetStableId, false)) ++applied;
        }
    }
    return applied;
}

std::optional<MilitaryThreatIntent> militaryThreatFromRegisterAction(const SurfaceSiegeState& state,
                                                                      std::optional<SurfaceCellAddress> target) {
    if (state.type != ImperialEnforcementType::RegisterAction ||
        state.phase == RegisterActionPhase::Idle || state.phase == RegisterActionPhase::Cleared ||
        state.phase == RegisterActionPhase::Failed)
        return std::nullopt;
    MilitaryThreatIntent threat{};
    threat.stableId = state.actionId;
    threat.kind = MilitaryThreatKind::ImperialRegisterAction;
    threat.severity = state.band == ImperialAttentionBand::Hunted ? 1.0f : 0.75f;
    if (target) { threat.hasCell = true; threat.cell = *target; }
    return threat;
}

std::string serializeMilitarySquad(const MilitarySquadEntityState& s) {
    std::ostringstream out;
    out << "ELYSIUM_MILITARY_SQUAD 1\n";
    out << s.identity.stableId << ' ' << std::quoted(s.identity.name) << ' ' << s.identity.factionStableId << '\n';
    out << s.membership.commanderStableId << ' ' << s.membership.members.size();
    for (auto id : s.membership.members) out << ' ' << id;
    out << '\n';
    out << s.uniform.profileStableId << ' ' << s.uniform.requirements.size();
    for (const auto& req : s.uniform.requirements)
        out << ' ' << req.itemId << ' ' << req.countPerMember << ' ' << (req.optional ? 1 : 0);
    out << '\n';
    out << (s.equipmentPolicy.allowSubstitutes ? 1 : 0) << ' ' << (s.equipmentPolicy.allowReservationPreemption ? 1 : 0) << '\n';
    out << s.trainingPolicy.targetProficiency << ' ' << s.trainingPolicy.drillGainPerHour << '\n';
    for (std::size_t i = 0; i < s.schedule.hourly.size(); ++i) out << static_cast<int>(s.schedule.hourly[i]) << (i + 1 == s.schedule.hourly.size() ? '\n' : ' ');
    out << s.currentOrder.sequence << ' ' << static_cast<int>(s.currentOrder.type) << ' ' << (s.currentOrder.active ? 1 : 0)
        << ' ' << (s.currentOrder.hasTargetCell ? 1 : 0);
    writeCell(out, s.currentOrder.targetCell);
    out << ' ' << s.currentOrder.targetStableId << ' ' << s.currentOrder.threatStableId << '\n';
    out << s.mission.missionStableId << ' ' << static_cast<int>(s.mission.threatKind) << ' ' << s.mission.threatStableId
        << ' ' << (s.mission.active ? 1 : 0) << ' ' << (s.mission.hasTargetCell ? 1 : 0);
    writeCell(out, s.mission.targetCell);
    out << '\n';
    out << static_cast<int>(s.alert.level) << ' ' << s.alert.sourceThreatStableId << '\n';
    out << s.barracks.barracksStableId << ' ' << (s.barracks.hasRallyCell ? 1 : 0);
    writeCell(out, s.barracks.rallyCell);
    out << ' ' << (s.barracks.hasTrainingCell ? 1 : 0);
    writeCell(out, s.barracks.trainingCell);
    out << '\n';
    out << s.posts.patrolRoute.size(); for (const auto& cell : s.posts.patrolRoute) writeCell(out, cell); out << '\n';
    out << s.posts.guardCells.size(); for (const auto& cell : s.posts.guardCells) writeCell(out, cell); out << '\n';
    out << (s.posts.hasEvacuationCell ? 1 : 0); writeCell(out, s.posts.evacuationCell);
    out << ' ' << (s.posts.hasMedevacCell ? 1 : 0); writeCell(out, s.posts.medevacCell); out << '\n';
    out << s.ammo.ammoItemId << ' ' << s.ammo.roundsPerMember << ' ' << s.ammo.squadReserveRounds << '\n';
    out << (s.history.recordBattles ? 1 : 0) << ' ' << (s.history.recordPromotions ? 1 : 0) << '\n';
    return out.str();
}

std::optional<MilitarySquadEntityState> deserializeMilitarySquad(std::string_view text, std::string* error) {
    auto fail = [&](std::string message) -> std::optional<MilitarySquadEntityState> {
        if (error) *error = std::move(message);
        return std::nullopt;
    };
    std::istringstream in{std::string(text)};
    std::string magic;
    int version = 0;
    if (!(in >> magic >> version) || magic != "ELYSIUM_MILITARY_SQUAD" || version != 1)
        return fail("unsupported military squad header");
    MilitarySquadEntityState s{};
    if (!(in >> s.identity.stableId >> std::quoted(s.identity.name) >> s.identity.factionStableId) || s.identity.stableId == 0)
        return fail("invalid military squad identity");
    std::size_t count = 0;
    if (!(in >> s.membership.commanderStableId >> count) || count > 4096) return fail("invalid military membership count");
    s.membership.members.resize(count);
    for (auto& id : s.membership.members) if (!(in >> id) || id == 0) return fail("invalid military member id");
    if (!(in >> s.uniform.profileStableId >> count) || count > 256) return fail("invalid uniform requirement count");
    s.uniform.requirements.resize(count);
    for (auto& req : s.uniform.requirements) {
        bool optional = false;
        if (!(in >> req.itemId >> req.countPerMember) || !readBool(in, optional) || req.itemId <= 0 || req.countPerMember <= 0)
            return fail("invalid uniform requirement");
        req.optional = optional;
    }
    if (!readBool(in, s.equipmentPolicy.allowSubstitutes) || !readBool(in, s.equipmentPolicy.allowReservationPreemption))
        return fail("invalid equipment policy");
    if (!(in >> s.trainingPolicy.targetProficiency >> s.trainingPolicy.drillGainPerHour) ||
        s.trainingPolicy.targetProficiency <= 0.0f || s.trainingPolicy.drillGainPerHour < 0.0f)
        return fail("invalid training policy");
    for (auto& routine : s.schedule.hourly) {
        int raw = 0; if (!(in >> raw) || raw < 0 || raw > 7) return fail("invalid military schedule");
        routine = static_cast<MilitaryRoutine>(raw);
    }
    int type = 0;
    if (!(in >> s.currentOrder.sequence >> type) || type < 0 || type > 6 || !readBool(in, s.currentOrder.active) ||
        !readBool(in, s.currentOrder.hasTargetCell) || !readCell(in, s.currentOrder.targetCell) ||
        !(in >> s.currentOrder.targetStableId >> s.currentOrder.threatStableId)) return fail("invalid military order");
    s.currentOrder.type = static_cast<TacticalCommandType>(type);
    int threatKind = 0;
    if (!(in >> s.mission.missionStableId >> threatKind >> s.mission.threatStableId) || threatKind < 0 || threatKind > 5 ||
        !readBool(in, s.mission.active) || !readBool(in, s.mission.hasTargetCell) || !readCell(in, s.mission.targetCell))
        return fail("invalid military mission");
    s.mission.threatKind = static_cast<MilitaryThreatKind>(threatKind);
    int alert = 0;
    if (!(in >> alert >> s.alert.sourceThreatStableId) || alert < 0 || alert > 4) return fail("invalid military alert");
    s.alert.level = static_cast<MilitaryAlertLevel>(alert);
    if (!(in >> s.barracks.barracksStableId) || !readBool(in, s.barracks.hasRallyCell) || !readCell(in, s.barracks.rallyCell) ||
        !readBool(in, s.barracks.hasTrainingCell) || !readCell(in, s.barracks.trainingCell)) return fail("invalid barracks assignment");
    if (!(in >> count) || count > 4096) return fail("invalid patrol route count");
    s.posts.patrolRoute.resize(count); for (auto& cell : s.posts.patrolRoute) if (!readCell(in, cell)) return fail("invalid patrol route cell");
    if (!(in >> count) || count > 4096) return fail("invalid guard cell count");
    s.posts.guardCells.resize(count); for (auto& cell : s.posts.guardCells) if (!readCell(in, cell)) return fail("invalid guard cell");
    if (!readBool(in, s.posts.hasEvacuationCell) || !readCell(in, s.posts.evacuationCell) ||
        !readBool(in, s.posts.hasMedevacCell) || !readCell(in, s.posts.medevacCell)) return fail("invalid emergency posts");
    if (!(in >> s.ammo.ammoItemId >> s.ammo.roundsPerMember >> s.ammo.squadReserveRounds) ||
        s.ammo.roundsPerMember < 0 || s.ammo.squadReserveRounds < 0) return fail("invalid ammunition policy");
    if (!readBool(in, s.history.recordBattles) || !readBool(in, s.history.recordPromotions)) return fail("invalid history policy");
    std::string trailing;
    if (in >> trailing) return fail("trailing military squad data");
    if (error) error->clear();
    return s;
}

} // namespace elysium
