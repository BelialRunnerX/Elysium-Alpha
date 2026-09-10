// Intended function: imported world implementation for MachineRuntime; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/BaseInfrastructure.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace elysium {

enum class MachineDomain : std::uint8_t { Power, Industry, Logistics, Habitat, Defense, Command };
enum class MachineTickPolicy : std::uint8_t { Passive, FixedLocal, EventDriven };
enum class InventoryPortPolicy : std::uint8_t { None, LocalInventory, RecipePorts, SharedNetwork, DirectedTransport };

struct BuildingComponentDefinition { bool building{true}; };
struct MachineTypeRefComponentDefinition { MachineType type{MachineType::StorageCrate}; std::string_view stableContentId; std::string_view displayName; int tier{}; MachineDomain domain{MachineDomain::Industry}; };
struct FootprintComponentDefinition { std::uint8_t width{1},depth{1},height{1}; };
struct InventoryPortsComponentDefinition { InventoryPortPolicy policy{InventoryPortPolicy::None}; std::uint8_t maxStacks{}; };
struct PowerConsumerComponentDefinition { float demand{}; std::uint8_t priority{5}; };
struct PowerProducerComponentDefinition { float generation{}; bool fuelDriven{}; };
struct PowerStorageComponentDefinition { float capacity{}; float chargeRate{}; float dischargeRate{}; };
struct OperatorRequirementComponentDefinition { bool requiresOperator{}; std::string_view skillFamily; };
struct RecipeProcessorComponentDefinition { bool enabled{}; bool fuelDriven{}; };
struct QueuePolicyComponentDefinition { std::uint8_t maxQueuedEntries{}; bool preservePartialBatch{true}; };
struct EnvironmentRequirementComponentDefinition { bool sealedRoomRequired{}; bool ventilationPreferred{}; bool coolingRequired{}; };
struct MaintenanceComponentDefinition { float wearPerActiveSecond{}; float preventiveThreshold{0.75f}; float predictiveThreshold{0.45f}; float hardFaultThreshold{0.10f}; };
struct AutomationEndpointComponentDefinition { bool exposesSignals{}; bool acceptsActions{}; };
struct AccessPolicyComponentDefinition { bool supportsRestrictions{}; };
struct DamageableComponentDefinition { float nominalDurability{100.0f}; };

struct MachineDefinition {
    BuildingComponentDefinition building{};
    MachineTypeRefComponentDefinition typeRef{};
    FootprintComponentDefinition footprint{};
    InventoryPortsComponentDefinition inventoryPorts{};
    PowerConsumerComponentDefinition consumer{};
    PowerProducerComponentDefinition producer{};
    PowerStorageComponentDefinition storage{};
    OperatorRequirementComponentDefinition operatorRequirement{};
    RecipeProcessorComponentDefinition recipeProcessor{};
    QueuePolicyComponentDefinition queuePolicy{};
    EnvironmentRequirementComponentDefinition environment{};
    MaintenanceComponentDefinition maintenance{};
    AutomationEndpointComponentDefinition automation{};
    AccessPolicyComponentDefinition access{};
    DamageableComponentDefinition damageable{};
    MachineTickPolicy tickPolicy{MachineTickPolicy::FixedLocal};
};

const MachineDefinition& machineDefinition(MachineType type);
const std::vector<MachineDefinition>& machineDefinitions();
bool machineHasRecipeProcessor(MachineType type);
bool machineNeedsElectricPower(MachineType type);
float machinePowerDemand(MachineType type);
int machinePowerPriority(MachineType type);
float machinePowerGeneration(MachineType type);
float machinePowerStorageCapacity(MachineType type);
float machinePowerChargeRate(MachineType type);
float machinePowerDischargeRate(MachineType type);

} // namespace elysium
