// Intended function: Project squads, hostiles, sensors, defenses, evacuation zones, breaches, hazards, and tactical orders into overlay state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct TacticalMarker {
    std::uint64_t markerId{};
    std::uint64_t stableId{};
    std::uint64_t kind{};
    std::uint64_t priority{};
    double readiness{};
    std::uint64_t flags{};
};
class TacticalMarkerTable {
public:
 bool set(TacticalMarker value); bool remove(std::uint64_t id);
 [[nodiscard]] const TacticalMarker* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<TacticalMarker> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const TacticalMarker& value) noexcept; std::vector<TacticalMarker> rows_;
};
}
