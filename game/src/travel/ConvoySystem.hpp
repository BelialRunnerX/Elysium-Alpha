// Intended function: Coordinate strategic convoys with escorts, manifests, route risk, speed, cohesion, and loss/recovery states.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::travel {
struct ConvoyState {
    std::uint64_t convoyId{};
    std::uint64_t routeId{};
    std::uint64_t manifestId{};
    std::uint64_t escortStrength{};
    double speed{};
    double risk{};
};
class ConvoyStateRegistry {
public:
    bool publish(ConvoyState record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const ConvoyState* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<ConvoyState>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const ConvoyState& r) noexcept;
    std::vector<ConvoyState> records_;
};
} // namespace elysium::travel
