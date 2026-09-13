// Concept-sheet vehicle catalogue (Fourth Edition Part 16.4).
// Maps production-sheet ids to VehicleType ordinals and function modes.
#pragma once

#include "travel/Vehicles.hpp"

#include <array>
#include <cstdint>
#include <string_view>

namespace elysium {

struct ConceptVehicleMode {
    std::string_view id;
    std::string_view name;
    std::string_view verb;
};

struct ConceptVehicleSheet {
    std::string_view sheetId;      // terra_drone / rover / orbi_scout / hover_bike
    std::string_view codename;     // TDR-01 etc.
    VehicleType type;              // save-stable enum
    bool rideable;
    std::array<ConceptVehicleMode, 4> modes;
    std::uint8_t modeCount;
};

inline constexpr ConceptVehicleSheet kConceptVehicles[] = {
    {"terra_drone", "TDR-01", VehicleType::MiningRig, true,
     {{{"idle", "IDLE / SCAN", "Standby scan pulse"},
       {"build", "BUILD / REPAIR", "Holo-construct assist"},
       {"plant", "PLANT SEED", "Ecological assist"},
       {"carry", "CARRY / DELIVER", "Back-rack logistics"}}},
     4},
    {"rover", "R-01", VehicleType::ScoutRover, true,
     {{{"research", "RESEARCH", "Sensors / analysis"},
       {"cargo", "CARGO", "Haul / supply"},
       {"utility", "UTILITY", "Field work"},
       {"excavate", "EXCAVATION", "Excavate / collect"}}},
     4},
    {"orbi_scout", "ORBI", VehicleType::HoverSled, false,
     {{{"hover", "HOVER", "Station keep"},
       {"scan", "SCAN", "Environment hologram"},
       {"follow", "FOLLOW", "Ally assist"},
       {"assist", "ASSIST", "Carry / utility"}}},
     4},
    {"hover_bike", "HB-07", VehicleType::HoverSled, true,
     {{{"explore", "EXPLORATION", "Fast personal travel"},
       {"cargo", "CARGO HAULER", "Logistics rack"},
       {"survey", "SURVEY / SCOUT", "Mapping mast"},
       {"", "", ""}}},
     3},
};

} // namespace elysium
