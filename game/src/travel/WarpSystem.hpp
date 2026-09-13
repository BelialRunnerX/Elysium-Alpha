// Intended function: Manage warp charge, route commitment, interdiction risk, cooldown, and deterministic arrival state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::travel {
struct WarpState {
    std::uint64_t shipId{};
    std::uint64_t routeId{};
    double charge{};
    double requiredCharge{};
    std::uint64_t cooldownTicks{};
    std::uint64_t state{};
};
class WarpStateRegistry {
public:
    bool publish(WarpState record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const WarpState* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<WarpState>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const WarpState& r) noexcept;
    std::vector<WarpState> records_;
};
} // namespace elysium::travel
