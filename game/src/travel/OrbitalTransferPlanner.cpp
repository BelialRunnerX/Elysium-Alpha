#include "travel/OrbitalTransferPlanner.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Plan local orbital transfers between surface, station, moon, and ship targets without owning flight physics.
bool OrbitalTransferPlannerSystem::apply(const OrbitalTransferPlannerRequest& request) {
    if (request.targetId == 0) return false;
    auto& state = states_[request.targetId];
    state.sequence = nextSequence_++;
    state.actorId = request.actorId;
    state.targetId = request.targetId;
    state.value = request.magnitude;
    state.active = true;
    return true;
}

bool OrbitalTransferPlannerSystem::erase(std::uint64_t targetId) {
    return states_.erase(targetId) != 0;
}

const OrbitalTransferPlannerState* OrbitalTransferPlannerSystem::find(std::uint64_t targetId) const {
    const auto it = states_.find(targetId);
    return it == states_.end() ? nullptr : &it->second;
}

std::vector<std::uint64_t> OrbitalTransferPlannerSystem::activeIds() const {
    std::vector<std::uint64_t> ids;
    ids.reserve(states_.size());
    for (const auto& [id, state] : states_) if (state.active) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

void OrbitalTransferPlannerSystem::clear() {
    states_.clear();
    nextSequence_ = 1;
}

} // namespace elysium
