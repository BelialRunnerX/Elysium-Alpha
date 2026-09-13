// Intended function: imported travel implementation for OrbitalInfrastructure; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "travel/ShipTravel.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

enum class OrbitalInfrastructureType : std::uint8_t {
    CargoDepot = 0,
    SurveyRelay = 1,
    Drydock = 2,
    OrbitalRefinery = 3,
    DefensePlatform = 4,
    GateAnchor = 5
};

struct OrbitalInfrastructureDefinition {
    OrbitalInfrastructureType type{};
    std::string_view name;
    int cargoSlots{};
    float baseHull{};
    float powerDemand{};
    float surveyRangeBonus{};
    float refineryThroughput{};
    float defenseStrength{};
    float gateRangeMultiplier{1.0f};
    float exposure{};
    bool shipService{};
};

const OrbitalInfrastructureDefinition& orbitalInfrastructureDefinition(OrbitalInfrastructureType type);
const std::vector<OrbitalInfrastructureDefinition>& orbitalInfrastructureRegistry();

struct OrbitalCargoStack {
    int itemId{};
    int count{};

    bool operator==(const OrbitalCargoStack&) const = default;
};

struct OrbitalInfrastructureState {
    std::uint64_t stableId{};
    OrbitalInfrastructureType type{OrbitalInfrastructureType::CargoDepot};
    std::uint32_t systemId{};
    std::uint32_t planetId{};
    std::uint64_t ownerStableId{};
    bool enabled{true};
    bool powered{};
    float hull{};
    float maintenance{1.0f};
    std::vector<OrbitalCargoStack> cargo;
    std::vector<ShipModuleId> serviceInventory;
    float processProgressSeconds{};

    bool operator==(const OrbitalInfrastructureState&) const = default;
};

struct OrbitalServiceSummary {
    int cargoSlots{};
    float surveyRangeBonus{};
    float refineryThroughput{};
    float defenseStrength{};
    float gateRangeMultiplier{1.0f};
    float exposure{};
    int poweredAssets{};
    bool hasDrydock{};
};

class OrbitalInfrastructureSystem {
public:
    explicit OrbitalInfrastructureSystem(std::uint64_t systemSeed = 0);

    std::uint64_t place(OrbitalInfrastructureType type,
                        std::uint32_t systemId,
                        std::uint32_t planetId,
                        std::uint64_t ownerStableId = 0);
    bool restore(const OrbitalInfrastructureState& state);
    bool remove(std::uint64_t stableId);
    OrbitalInfrastructureState* find(std::uint64_t stableId);
    const OrbitalInfrastructureState* find(std::uint64_t stableId) const;

    OrbitalServiceSummary summary(std::uint32_t systemId,std::uint32_t planetId) const;

    int cargoInsert(std::uint64_t stableId,int itemId,int count);
    int cargoExtract(std::uint64_t stableId,int itemId,int count);

    // Drydock service is deliberately explicit: a ship must be docked to the
    // exact drydock StableId and the module must exist in serviceInventory.
    bool installModuleAtDrydock(ShipState& ship,
                                std::uint64_t drydockStableId,
                                ShipModuleId module,
                                std::string* error = nullptr);
    float repairShipAtDrydock(ShipState& ship,
                              std::uint64_t drydockStableId,
                              float requestedHull,
                              std::string* error = nullptr);

    const std::vector<OrbitalInfrastructureState>& objects() const { return objects_; }

private:
    std::uint64_t systemSeed_{};
    std::uint64_t nextSerial_{1};
    std::vector<OrbitalInfrastructureState> objects_;

    std::uint64_t allocateStableId();
};

} // namespace elysium
