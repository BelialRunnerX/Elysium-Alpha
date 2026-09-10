// Intended function: imported world implementation for MachineRuntime; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/MachineRuntime.hpp"

#include <stdexcept>

namespace elysium {
namespace {
MachineDefinition makeDef(MachineType type,std::string_view id,std::string_view name,int tier,MachineDomain domain,
                          float demand,int priority,float generation,bool producerFuel,InventoryPortPolicy ports,std::uint8_t stacks,
                          bool processor,bool processFuel,float wear,bool operatorRequired=false,std::string_view skill={}) {
    MachineDefinition d{};
    d.typeRef={type,id,name,tier,domain}; d.inventoryPorts={ports,stacks};
    d.consumer={demand,static_cast<std::uint8_t>(priority)}; d.producer={generation,producerFuel};
    d.operatorRequirement={operatorRequired,skill}; d.recipeProcessor={processor,processFuel};
    d.queuePolicy={static_cast<std::uint8_t>(processor?16:0),true};
    if(type==MachineType::BatteryBank) d.storage={60.0f,15.0f,15.0f};
    d.maintenance.wearPerActiveSecond=wear; d.automation={true,true}; d.access.supportsRestrictions=true; return d;
}
const std::vector<MachineDefinition>& definitions() {
    static const std::vector<MachineDefinition> defs{
        makeDef(MachineType::BurnerGenerator,"elysium:machine/burner_generator","Burner Generator",2,MachineDomain::Power,0,5,20,true,InventoryPortPolicy::LocalInventory,4,false,false,0.000020f),
        makeDef(MachineType::BatteryBank,"elysium:machine/battery_bank","Battery Bank",2,MachineDomain::Power,0,5,0,false,InventoryPortPolicy::None,0,false,false,0.000003f),
        makeDef(MachineType::AtmosphereUnit,"elysium:machine/atmosphere_unit","Atmosphere Unit",2,MachineDomain::Habitat,8,1,0,false,InventoryPortPolicy::LocalInventory,4,false,false,0.000012f),
        makeDef(MachineType::StorageCrate,"elysium:machine/storage_crate","Storage Crate",1,MachineDomain::Logistics,0,5,0,false,InventoryPortPolicy::LocalInventory,24,false,false,0.000001f),
        makeDef(MachineType::AirlockController,"elysium:machine/airlock_controller","Airlock Controller",3,MachineDomain::Habitat,2.5f,2,0,false,InventoryPortPolicy::None,0,false,false,0.000006f),
        makeDef(MachineType::SensorMast,"elysium:machine/sensor_mast","Sensor Mast",2,MachineDomain::Defense,1.5f,2,0,false,InventoryPortPolicy::None,0,false,false,0.000006f),
        makeDef(MachineType::Turret,"elysium:machine/turret","Turret",3,MachineDomain::Defense,4,2,0,false,InventoryPortPolicy::LocalInventory,4,false,false,0.000020f),
        makeDef(MachineType::ShieldPylon,"elysium:machine/shield_pylon","Shield Pylon",4,MachineDomain::Defense,12,2,0,false,InventoryPortPolicy::None,0,false,false,0.000022f),
        makeDef(MachineType::LogicController,"elysium:machine/logic_controller","Logic Controller",3,MachineDomain::Command,1,2,0,false,InventoryPortPolicy::None,0,false,false,0.000004f),
        makeDef(MachineType::Furnace,"elysium:machine/furnace","Furnace",1,MachineDomain::Industry,0,4,0,false,InventoryPortPolicy::RecipePorts,8,true,true,0.000030f,true,"smelting"),
        makeDef(MachineType::AlloyCrucible,"elysium:machine/alloy_crucible","Alloy Crucible",2,MachineDomain::Industry,5,4,0,false,InventoryPortPolicy::RecipePorts,8,true,false,0.000035f,true,"alloying"),
        makeDef(MachineType::Refinery,"elysium:machine/refinery","Refinery",3,MachineDomain::Industry,8,4,0,false,InventoryPortPolicy::RecipePorts,8,true,false,0.000040f,true,"metallurgy"),
        makeDef(MachineType::NetworkStorage,"elysium:machine/network_storage","Network Storage",3,MachineDomain::Logistics,1.5f,3,0,false,InventoryPortPolicy::SharedNetwork,64,false,false,0.000004f),
        makeDef(MachineType::Conveyor,"elysium:machine/conveyor","Conveyor",3,MachineDomain::Logistics,0.75f,3,0,false,InventoryPortPolicy::DirectedTransport,8,false,false,0.000010f),
        makeDef(MachineType::Sorter,"elysium:machine/sorter","Sorter",3,MachineDomain::Logistics,1,3,0,false,InventoryPortPolicy::DirectedTransport,8,false,false,0.000012f),
        makeDef(MachineType::CargoLoader,"elysium:machine/cargo_loader","Cargo Loader",3,MachineDomain::Logistics,2,3,0,false,InventoryPortPolicy::DirectedTransport,8,false,false,0.000015f),
        makeDef(MachineType::Crusher,"elysium:machine/crusher","Crusher",2,MachineDomain::Industry,4,4,0,false,InventoryPortPolicy::RecipePorts,8,true,false,0.000045f,true,"machining"),
        makeDef(MachineType::ChemicalVat,"elysium:machine/chemical_vat","Chemical Vat",3,MachineDomain::Industry,6,4,0,false,InventoryPortPolicy::RecipePorts,8,true,false,0.000035f,true,"chemistry"),
        makeDef(MachineType::Fabricator,"elysium:machine/fabricator","Fabricator",4,MachineDomain::Industry,9,4,0,false,InventoryPortPolicy::RecipePorts,8,true,false,0.000030f,true,"fabrication"),
        makeDef(MachineType::Extractor,"elysium:machine/extractor","Extractor",4,MachineDomain::Industry,10,4,0,false,InventoryPortPolicy::RecipePorts,8,false,false,0.000050f),
        // Appended only. Exotic recipes remain intentionally absent until their documented world inputs exist.
        makeDef(MachineType::ArcSmelter,"elysium:machine/arc_smelter","Arc Smelter",4,MachineDomain::Industry,12,4,0,false,InventoryPortPolicy::RecipePorts,8,true,false,0.000055f,true,"exotic_metallurgy")
    };
    return defs;
}
}
const std::vector<MachineDefinition>& machineDefinitions(){return definitions();}
const MachineDefinition& machineDefinition(MachineType type){const auto i=static_cast<std::size_t>(type); const auto& d=definitions(); if(i>=d.size()||d[i].typeRef.type!=type) throw std::out_of_range("unknown MachineType"); return d[i];}
bool machineHasRecipeProcessor(MachineType type){return machineDefinition(type).recipeProcessor.enabled;}
bool machineNeedsElectricPower(MachineType type){return machineDefinition(type).consumer.demand>0.0f;}
float machinePowerDemand(MachineType type){return machineDefinition(type).consumer.demand;}
int machinePowerPriority(MachineType type){return machineDefinition(type).consumer.priority;}
float machinePowerGeneration(MachineType type){return machineDefinition(type).producer.generation;}
float machinePowerStorageCapacity(MachineType type){return machineDefinition(type).storage.capacity;}
float machinePowerChargeRate(MachineType type){return machineDefinition(type).storage.chargeRate;}
float machinePowerDischargeRate(MachineType type){return machineDefinition(type).storage.dischargeRate;}
} // namespace elysium
