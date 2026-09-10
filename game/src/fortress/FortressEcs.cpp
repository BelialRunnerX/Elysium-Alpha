#include "fortress/FortressEcs.hpp"
#include "fortress/Systems.hpp"

#include <type_traits>

namespace elysium::fortress {

entt::entity findEntityByStableId(entt::registry& registry, StableId stableId) {
    auto view = registry.view<PersistentIdentity>();
    for (const auto entity : view) {
        if (view.get<PersistentIdentity>(entity).stableId == stableId) return entity;
    }
    return entt::null;
}

entt::entity createCitizenEntity(entt::registry& registry, const SpawnCitizenCommand& command) {
    const auto entity = registry.create();
    registry.emplace<PersistentIdentity>(entity, command.identity);
    registry.emplace<SettlementMembership>(entity, command.membership);
    registry.emplace<AgeLifeStage>(entity, command.age);
    registry.emplace<BiologicalState>(entity, command.biology);
    registry.emplace<Personality>(entity);
    registry.emplace<Values>(entity);
    registry.emplace<Preferences>(entity);
    Needs needs{};
    for (unsigned kind = 0; kind <= static_cast<unsigned>(NeedKind::Excitement); ++kind) {
        needs.states.push_back(NeedState{static_cast<NeedKind>(kind), 0.8f, 0.2f, 0.05f});
    }
    registry.emplace<Needs>(entity, std::move(needs));
    registry.emplace<CurrentThoughts>(entity);
    registry.emplace<Memories>(entity);
    registry.emplace<FocusState>(entity);
    registry.emplace<StressState>(entity);
    registry.emplace<Skills>(entity);
    registry.emplace<WorkDetails>(entity);
    registry.emplace<Schedule>(entity);
    registry.emplace<JobHistory>(entity);
    registry.emplace<HistorySignificance>(entity);
    registry.emplace<PersistencePolicy>(entity);
    return entity;
}

entt::entity createJobEntity(entt::registry& registry, const JobComponent& job) {
    const auto entity = registry.create();
    registry.emplace<PersistentIdentity>(entity, PersistentIdentity{StableId{job.id.value}, job.type.value, SimulationShard::ActiveFortress, 1});
    registry.emplace<JobComponent>(entity, job);
    return entity;
}

FortressCommitStats commitFortressCommands(entt::registry& registry,
                                           FortressCommandBuffer& buffer,
                                           const FortressCommitHooks& hooks) {
    FortressCommitStats stats{};
    buffer.sortDeterministic();
    for (const auto& command : buffer.commands()) {
        bool handled = true;
        std::visit([&](const auto& payload) {
            using T = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<T, SpawnCitizenCommand>) {
                createCitizenEntity(registry, payload);
                ++stats.created;
            } else if constexpr (std::is_same_v<T, CreateJobCommand>) {
                createJobEntity(registry, payload.job);
                ++stats.created;
            } else if constexpr (std::is_same_v<T, UpdateJobCommand>) {
                const auto entity = findEntityByStableId(registry, StableId{payload.job.value});
                if (entity != entt::null && registry.all_of<JobComponent>(entity)) {
                    auto& job = registry.get<JobComponent>(entity);
                    job.state = payload.state;
                    job.progress = payload.progress;
                    job.failureReason = payload.reason;
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, CancelJobCommand>) {
                const auto entity = findEntityByStableId(registry, StableId{payload.job.value});
                if (entity != entt::null && registry.all_of<JobComponent>(entity)) {
                    auto& job = registry.get<JobComponent>(entity);
                    job.state = JobState::Cancelled;
                    job.failureReason = payload.reason;
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, CreateDesignationCommand>) {
                const auto entity = registry.create();
                registry.emplace<PersistentIdentity>(entity, PersistentIdentity{StableId{payload.designation.id.value}, "designation", SimulationShard::ActiveFortress, 1});
                registry.emplace<DesignationComponent>(entity, payload.designation);
                ++stats.created;
            } else if constexpr (std::is_same_v<T, CreateRoomCommand>) {
                const auto entity = registry.create();
                registry.emplace<PersistentIdentity>(entity, PersistentIdentity{StableId{payload.room.id.value}, "room", SimulationShard::ActiveFortress, 1});
                registry.emplace<RoomComponent>(entity, payload.room);
                ++stats.created;
            } else if constexpr (std::is_same_v<T, CreateZoneCommand>) {
                const auto entity = registry.create();
                registry.emplace<PersistentIdentity>(entity, PersistentIdentity{StableId{payload.zone.id.value}, "zone", SimulationShard::ActiveFortress, 1});
                registry.emplace<ZoneComponent>(entity, payload.zone);
                ++stats.created;
            } else if constexpr (std::is_same_v<T, CreateStockpileCommand>) {
                const auto entity = registry.create();
                registry.emplace<PersistentIdentity>(entity, PersistentIdentity{StableId{payload.stockpile.id.value}, "stockpile", SimulationShard::ActiveFortress, 1});
                registry.emplace<StockpileComponent>(entity, payload.stockpile);
                ++stats.created;
            } else if constexpr (std::is_same_v<T, SpawnItemCommand>) {
                const auto existing = findEntityByStableId(registry, payload.item.id);
                if (existing == entt::null) {
                    const auto entity = registry.create();
                    registry.emplace<PersistentIdentity>(entity, PersistentIdentity{payload.item.id, payload.item.item.value, SimulationShard::ActiveFortress, 1});
                    registry.emplace<ItemState>(entity, payload.item);
                    ++stats.created;
                } else {
                    registry.emplace_or_replace<ItemState>(existing, payload.item);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, TransferItemCommand>) {
                const auto entity = findEntityByStableId(registry, payload.item);
                if (entity != entt::null && registry.all_of<ItemState>(entity)) {
                    auto& item = registry.get<ItemState>(entity);
                    if (payload.quantity >= item.quantity) {
                        item.container = payload.destination;
                        ++stats.updated;
                    } else {
                        handled = false; // partial-stack split requires authoritative item-ID allocation service.
                    }
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, ConsumeItemCommand>) {
                const auto entity = findEntityByStableId(registry, payload.item);
                if (entity != entt::null && registry.all_of<ItemState>(entity)) {
                    auto& item = registry.get<ItemState>(entity);
                    if (payload.quantity >= item.quantity) {
                        registry.destroy(entity);
                    } else {
                        item.quantity -= payload.quantity;
                    }
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, StartProcessCommand>) {
                const auto entity = findEntityByStableId(registry, payload.machine);
                if (entity != entt::null && registry.all_of<MachineState>(entity)) {
                    auto& machine = registry.get<MachineState>(entity);
                    machine.activeProcess = payload.recipe;
                    machine.processProgress = 0.0f;
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, StopProcessCommand>) {
                const auto entity = findEntityByStableId(registry, payload.machine);
                if (entity != entt::null && registry.all_of<MachineState>(entity)) {
                    auto& machine = registry.get<MachineState>(entity);
                    machine.activeProcess = ContentId{};
                    machine.processProgress = 0.0f;
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, RequestMaintenanceCommand>) {
                const auto entity = findEntityByStableId(registry, payload.machine);
                if (entity != entt::null) {
                    auto& maintenance = registry.get_or_emplace<MaintenanceState>(entity);
                    maintenance.serviceRequested = true;
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, ApplyThoughtCommand>) {
                const auto entity = findEntityByStableId(registry, payload.citizen);
                if (entity != entt::null) {
                    auto& thoughts = registry.get_or_emplace<CurrentThoughts>(entity);
                    thoughts.thoughts.push_back(payload.thought);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, ApplyMemoryCommand>) {
                const auto entity = findEntityByStableId(registry, payload.citizen);
                if (entity != entt::null) {
                    auto& memories = registry.get_or_emplace<Memories>(entity);
                    memories.entries.push_back(payload.memory);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, AdjustStressCommand>) {
                const auto entity = findEntityByStableId(registry, payload.citizen);
                if (entity != entt::null) {
                    auto& stress = registry.get_or_emplace<StressState>(entity);
                    stress.load = saturate(stress.load + payload.delta);
                    stress.band = classifyStress(stress.load);
                    if (!payload.reason.empty()) stress.topReasons.push_back(payload.reason);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, ApplyWoundCommand>) {
                const auto entity = findEntityByStableId(registry, payload.patient);
                if (entity != entt::null) {
                    auto& body = registry.get_or_emplace<BodyState>(entity);
                    body.wounds.push_back(payload.wound);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, SetSquadAlertCommand>) {
                const auto entity = findEntityByStableId(registry, StableId{payload.squad.value});
                if (entity != entt::null && registry.all_of<SquadState>(entity)) {
                    registry.get<SquadState>(entity).alert = payload.alert;
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, CreateCaseCommand>) {
                const auto entity = registry.create();
                registry.emplace<PersistentIdentity>(entity, PersistentIdentity{StableId{payload.justiceCase.id.value}, "justice_case", SimulationShard::ActiveFortress, 1});
                registry.emplace<JusticeCase>(entity, payload.justiceCase);
                ++stats.created;
            } else if constexpr (std::is_same_v<T, CreateInstitutionCommand>) {
                const auto entity = registry.create();
                registry.emplace<PersistentIdentity>(entity, PersistentIdentity{StableId{payload.institution.id.value}, payload.institution.name, SimulationShard::ActiveFortress, 1});
                registry.emplace<InstitutionState>(entity, payload.institution);
                ++stats.created;
            } else if constexpr (std::is_same_v<T, CreateContractCommand>) {
                const auto entity = registry.create();
                registry.emplace<PersistentIdentity>(entity, PersistentIdentity{StableId{payload.contract.id.value}, "contract", SimulationShard::StarSystem, 1});
                registry.emplace<ContractState>(entity, payload.contract);
                ++stats.created;
            } else if constexpr (std::is_same_v<T, SpawnThreatCommand>) {
                const auto entity = registry.create();
                registry.emplace<PersistentIdentity>(entity, PersistentIdentity{payload.threat.id, "threat", SimulationShard::ActiveFortress, 1});
                registry.emplace<ThreatState>(entity, payload.threat);
                ++stats.created;
            } else if constexpr (std::is_same_v<T, CreateResearchCommand>) {
                const auto existing = findEntityByStableId(registry, payload.project.id);
                if (existing == entt::null) {
                    const auto entity = registry.create();
                    registry.emplace<PersistentIdentity>(entity, PersistentIdentity{payload.project.id, "research_project", SimulationShard::ActiveFortress, 1});
                    registry.emplace<ResearchProject>(entity, payload.project);
                    ++stats.created;
                } else {
                    registry.emplace_or_replace<ResearchProject>(existing, payload.project);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, CompleteResearchCommand>) {
                const auto entity = findEntityByStableId(registry, payload.project);
                if (entity != entt::null && registry.all_of<ResearchProject>(entity)) {
                    registry.get<ResearchProject>(entity).completed = true;
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, BeginObsessionCommand>) {
                const auto entity = findEntityByStableId(registry, payload.obsession.citizen);
                if (entity != entt::null) {
                    registry.emplace_or_replace<AethericObsession>(entity, payload.obsession);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, SetOperativeCommand>) {
                const auto entity = findEntityByStableId(registry, payload.operative.citizen);
                if (entity != entt::null) {
                    registry.emplace_or_replace<DirectOperativeState>(entity, payload.operative);
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, MigratePersonCommand>) {
                const auto entity = findEntityByStableId(registry, payload.person);
                if (entity != entt::null) {
                    auto& membership = registry.get_or_emplace<SettlementMembership>(entity);
                    membership.site = payload.to;
                    membership.resident = true;
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, CreateArtifactCommand>) {
                const auto stable = StableId{payload.artifact.artifact.value};
                const auto existing = findEntityByStableId(registry, stable);
                if (existing == entt::null) {
                    const auto entity = registry.create();
                    registry.emplace<PersistentIdentity>(entity, PersistentIdentity{stable, payload.artifact.name, SimulationShard::GalaxyHistory, 1});
                    registry.emplace<ArtifactState>(entity, payload.artifact);
                    ++stats.created;
                } else {
                    registry.emplace_or_replace<ArtifactState>(existing, payload.artifact);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, DispatchVehicleCommand>) {
                const auto entity = findEntityByStableId(registry, StableId{payload.vehicle.value});
                if (entity != entt::null && registry.all_of<VehicleState>(entity)) {
                    auto& vehicle = registry.get<VehicleState>(entity);
                    vehicle.destination = payload.destination;
                    vehicle.route = payload.route;
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, DispatchShipCommand>) {
                const auto entity = findEntityByStableId(registry, payload.ship);
                if (entity != entt::null && registry.all_of<ShipState>(entity)) {
                    registry.get<ShipState>(entity).destinationSystem = payload.destinationSystem;
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, CreateOrbitalSiteCommand>) {
                const auto stable = StableId{payload.site.id.value};
                const auto existing = findEntityByStableId(registry, stable);
                if (existing == entt::null) {
                    const auto entity = registry.create();
                    registry.emplace<PersistentIdentity>(entity, PersistentIdentity{stable, payload.site.type.value, SimulationShard::StarSystem, 1});
                    registry.emplace<OrbitalSiteState>(entity, payload.site);
                    ++stats.created;
                } else {
                    registry.emplace_or_replace<OrbitalSiteState>(existing, payload.site);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, SetAutomationRuleCommand>) {
                const auto existing = findEntityByStableId(registry, payload.rule.id);
                if (existing == entt::null) {
                    const auto entity = registry.create();
                    registry.emplace<PersistentIdentity>(entity, PersistentIdentity{payload.rule.id, "automation_rule", SimulationShard::ActiveFortress, 1});
                    registry.emplace<AutomationRule>(entity, payload.rule);
                    ++stats.created;
                } else {
                    registry.emplace_or_replace<AutomationRule>(existing, payload.rule);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, SetPowerIsolationCommand>) {
                const auto entity = findEntityByStableId(registry, payload.node);
                if (entity != entt::null && registry.all_of<PowerNode>(entity)) {
                    registry.get<PowerNode>(entity).isolated = payload.isolated;
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, DispatchRailCartCommand>) {
                const auto entity = findEntityByStableId(registry, payload.cart);
                if (entity != entt::null && registry.all_of<RailCartState>(entity)) {
                    auto& cart = registry.get<RailCartState>(entity);
                    cart.route = payload.route;
                    cart.progress = 0.0f;
                    cart.derailed = false;
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, StampBlueprintCommand>) {
                const auto stable = StableId{payload.blueprint.id.value};
                const auto existing = findEntityByStableId(registry, stable);
                if (existing == entt::null) {
                    const auto entity = registry.create();
                    registry.emplace<PersistentIdentity>(entity, PersistentIdentity{stable, payload.blueprint.name, SimulationShard::ActiveFortress, 1});
                    registry.emplace<ConstructionBlueprint>(entity, payload.blueprint);
                    ++stats.created;
                } else {
                    registry.emplace_or_replace<ConstructionBlueprint>(existing, payload.blueprint);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, MarkSupportDirtyCommand>) {
                const auto entity = findEntityByStableId(registry, payload.island);
                if (entity != entt::null && registry.all_of<SupportIsland>(entity)) {
                    registry.get<SupportIsland>(entity).dirty = true;
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, UpdateSiteCommand>) {
                const auto stable = StableId{payload.site.id.value};
                const auto existing = findEntityByStableId(registry, stable);
                if (existing == entt::null) {
                    const auto entity = registry.create();
                    registry.emplace<PersistentIdentity>(entity, PersistentIdentity{stable, payload.site.name, SimulationShard::StarSystem, 1});
                    registry.emplace<SiteState>(entity, payload.site);
                    ++stats.created;
                } else {
                    registry.emplace_or_replace<SiteState>(existing, payload.site);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, UpdateCivilizationCommand>) {
                const auto stable = StableId{payload.civilization.id.value};
                const auto existing = findEntityByStableId(registry, stable);
                if (existing == entt::null) {
                    const auto entity = registry.create();
                    registry.emplace<PersistentIdentity>(entity, PersistentIdentity{stable, payload.civilization.name, SimulationShard::GalaxyHistory, 1});
                    registry.emplace<CivilizationState>(entity, payload.civilization);
                    ++stats.created;
                } else {
                    registry.emplace_or_replace<CivilizationState>(existing, payload.civilization);
                    ++stats.updated;
                }
            } else if constexpr (std::is_same_v<T, PromoteHistoricalFigureCommand>) {
                const auto entity = findEntityByStableId(registry, payload.person);
                if (entity != entt::null) {
                    auto& significance = registry.get_or_emplace<HistorySignificance>(entity);
                    significance.score = std::max(significance.score, payload.significance);
                    significance.historicalFigure = true;
                    significance.forcePersist = true;
                    ++stats.updated;
                } else {
                    handled = false;
                }
            } else if constexpr (std::is_same_v<T, RecordHistoricalEventCommand>) {
                if (hooks.historyAppend) hooks.historyAppend(payload.event);
                handled = static_cast<bool>(hooks.historyAppend);
            } else {
                handled = false;
            }
        }, command.payload);

        if (handled) {
            ++stats.applied;
        } else {
            ++stats.deferredExternal;
            if (hooks.externalCommand) hooks.externalCommand(command);
        }
    }
    buffer.clear();
    return stats;
}

} // namespace elysium::fortress
