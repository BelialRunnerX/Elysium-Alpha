#include "world/HazardForecast.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Produce short strategic forecasts for storms, radiation, seismic activity, fire weather, and anomaly instability.
bool HazardForecastSystem::apply(const HazardForecastRequest& request) {
    if (request.targetId == 0) return false;
    auto& state = states_[request.targetId];
    state.sequence = nextSequence_++;
    state.actorId = request.actorId;
    state.targetId = request.targetId;
    state.value = request.magnitude;
    state.active = true;
    return true;
}

bool HazardForecastSystem::erase(std::uint64_t targetId) {
    return states_.erase(targetId) != 0;
}

const HazardForecastState* HazardForecastSystem::find(std::uint64_t targetId) const {
    const auto it = states_.find(targetId);
    return it == states_.end() ? nullptr : &it->second;
}

std::vector<std::uint64_t> HazardForecastSystem::activeIds() const {
    std::vector<std::uint64_t> ids;
    ids.reserve(states_.size());
    for (const auto& [id, state] : states_) if (state.active) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

void HazardForecastSystem::clear() {
    states_.clear();
    nextSequence_ = 1;
}

} // namespace elysium
