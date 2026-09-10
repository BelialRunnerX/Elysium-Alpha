#pragma once
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace elysium::construction {
using ContentId = std::uint64_t;

enum class PlacementMode : std::uint8_t { Free, Grid, Floor, Wall, Ceiling, Socket, Rail, Blueprint };
struct Extent3 { double x{1}, y{1}, z{1}; };
struct PlacementFamily {
    ContentId familyId{};
    std::uint64_t allowedModesMask{};
    Extent3 footprint{};
    ContentId snapGroup{};
    double minimumSupport01{};
    std::uint32_t rotationSteps{4};
    std::vector<ContentId> utilityPortTypes;
    std::uint32_t schemaVersion{1};
};
struct PlacementProbe {
    ContentId familyId{};
    PlacementMode mode{PlacementMode::Grid};
    bool collides{};
    bool surfaceAvailable{true};
    double support01{1.0};
};
struct PlacementDecision { bool accepted{}; reason::ReasonStack reasons; };

class PlacementCatalogue {
public:
    bool publish(PlacementFamily family, reason::ReasonStack* reasons = nullptr);
    [[nodiscard]] const PlacementFamily* find(ContentId id) const;
    [[nodiscard]] PlacementDecision evaluate(const PlacementProbe& probe) const;
private:
    std::unordered_map<ContentId, PlacementFamily> families_;
};

[[nodiscard]] constexpr std::uint64_t placementModeBit(PlacementMode mode) noexcept { return 1ULL << static_cast<unsigned>(mode); }

} // namespace elysium::construction
