#pragma once
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>

namespace elysium::world {
using RoomStableId = std::uint64_t;
using ContentId = std::uint64_t;

// Supplied by the atmosphere/spatial owner. This service deliberately does not flood-fill.
struct BoundedVolumeSnapshot {
    std::uint64_t volumeId{};
    std::uint64_t cellCount{};
    bool terminatedBounded{};
    bool reachedSky{};
    bool budgetExhausted{};
    double floorArea{};
    double volume{};
    std::vector<ContentId> installedCapabilities;
};
struct RoomFunctionDefinition {
    ContentId functionId{};
    double minimumFloorArea{};
    double minimumVolume{};
    bool requiresSealed{};
    std::vector<ContentId> requiredCapabilities;
};
struct RoomRecognitionResult {
    bool recognized{};
    bool sealed{};
    ContentId functionId{};
    reason::ReasonStack reasons;
};

class RoomFunctionRegistry {
public:
    bool publish(RoomFunctionDefinition definition, reason::ReasonStack* reasons = nullptr);
    [[nodiscard]] const RoomFunctionDefinition* find(ContentId id) const;
    [[nodiscard]] RoomRecognitionResult recognize(ContentId functionId, const BoundedVolumeSnapshot& volume) const;
private:
    std::unordered_map<ContentId, RoomFunctionDefinition> definitions_;
};
} // namespace elysium::world
