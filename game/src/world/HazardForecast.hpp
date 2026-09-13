#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Produce short strategic forecasts for storms, radiation, seismic activity, fire weather, and anomaly instability.
struct HazardForecastRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct HazardForecastState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class HazardForecastSystem {
public:
    bool apply(const HazardForecastRequest& request);
    bool erase(std::uint64_t targetId);
    const HazardForecastState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, HazardForecastState> states_;
};

} // namespace elysium
