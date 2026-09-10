#pragma once
#include <cstdint>

namespace elysium::world {

enum class ResourceProvenance : std::uint8_t {
    NaturalGenerated = 0,
    PlayerPlaced,
    Recycled,
    Imported,
    AdministrativeGrant,
    Unknown
};

[[nodiscard]] constexpr bool eligibleForNaturalExtractionReward(ResourceProvenance value) noexcept {
    return value == ResourceProvenance::NaturalGenerated;
}

} // namespace elysium::world
