#pragma once

#include "fortress/Components.hpp"

#include <span>
#include <string_view>
#include <vector>

namespace elysium::fortress {

struct LaborDefinition {
    std::string_view id;
    std::string_view family;
    std::string_view representativeJob;
    std::string_view requirement;
};

struct MachineDefinition {
    std::string_view id;
    std::string_view domain;
    std::string_view function;
    std::string_view labor;
    std::string_view environment;
    int tier{};
};

struct InstitutionDefinition {
    std::string_view id;
    std::string_view function;
};

struct ThreatDefinition {
    std::string_view id;
    ThreatKind kind{ThreatKind::Wildlife};
    float baseStrength{};
    bool historical{};
};

struct VehicleDefinition {
    std::string_view id;
    std::string_view role;
    float cargoCapacity{};
    float mobility{};
};

std::span<const LaborDefinition> laborCatalog();
std::span<const MachineDefinition> machineCatalog();
std::span<const InstitutionDefinition> institutionCatalog();
std::span<const ThreatDefinition> threatCatalog();
std::span<const VehicleDefinition> vehicleCatalog();

const LaborDefinition* findLabor(std::string_view id);
const MachineDefinition* findMachine(std::string_view id);

} // namespace elysium::fortress
