// Intended function: Project discoveries, claims, sites, routes, hazards, missions, and historical layers into a renderer-neutral map model.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::ui {
struct MapMarker {
    std::uint64_t markerId{};
    std::uint64_t kind{};
    std::uint64_t siteId{};
    double importance{};
    std::uint64_t visibility{};
    std::uint64_t flags{};
};
class MapMarkerRegistry {
public:
    bool publish(MapMarker record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const MapMarker* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<MapMarker>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const MapMarker& r) noexcept;
    std::vector<MapMarker> records_;
};
} // namespace elysium::ui
