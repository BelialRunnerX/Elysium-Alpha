// Intended function: imported travel implementation for TravelPersistence; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "travel/OrbitalInfrastructure.hpp"
#include "travel/Vehicles.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

struct TravelPersistentState {
    static constexpr std::uint32_t SchemaVersion = 1;

    ShipState ship;
    std::vector<VehicleState> vehicles;
    std::vector<OrbitalInfrastructureState> orbitalInfrastructure;

    bool operator==(const TravelPersistentState&) const = default;
};

bool validateTravelPersistentState(const TravelPersistentState& state,std::string* error=nullptr);
std::string serializeTravelPersistentState(const TravelPersistentState& state);
std::optional<TravelPersistentState> deserializeTravelPersistentState(std::string_view text,std::string* error=nullptr);

} // namespace elysium
