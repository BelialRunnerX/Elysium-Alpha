// Intended function: Track fire-response zones, suppressant stores, vent isolation, and bounded emergency suppression intents.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::world {

struct SuppressionZone {
    std::uint64_t zoneId{};
    std::uint64_t suppressantUnits{};
    double temperature{};
    double smoke{};
    std::uint64_t priority{};
    std::uint64_t state{};
};

class SuppressionZoneStore {
public:
    bool upsert(SuppressionZone value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const SuppressionZone* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<SuppressionZone> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const SuppressionZone& value) noexcept;
    std::vector<SuppressionZone> records_;
};

} // namespace elysium::world
