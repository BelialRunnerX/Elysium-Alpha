#pragma once

#include <cstdint>

namespace elysium {

enum class PlanetClass : std::uint8_t {
    Temperate = 0,
    Barren = 1,
    Scorched = 2,
    Frozen = 3,
    Toxic = 4,
    Irradiated = 5,
    Oceanic = 6,
    Anomalous = 7
};

} // namespace elysium
