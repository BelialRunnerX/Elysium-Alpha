#include "travel/EmergencyJump.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Produce emergency escape jump candidates with deterministic risk, fuel, damage, and destination tradeoffs.
bool EmergencyJumpSystem::apply(const EmergencyJumpRequest& request) {
    if (request.targetId == 0) return false;
    auto& state = states_[request.targetId];
    state.sequence = nextSequence_++;
    state.actorId = request.actorId;
    state.targetId = request.targetId;
    state.value = request.magnitude;
    state.active = true;
    return true;
}

bool EmergencyJumpSystem::erase(std::uint64_t targetId) {
    return states_.erase(targetId) != 0;
}

const EmergencyJumpState* EmergencyJumpSystem::find(std::uint64_t targetId) const {
    const auto it = states_.find(targetId);
    return it == states_.end() ? nullptr : &it->second;
}

std::vector<std::uint64_t> EmergencyJumpSystem::activeIds() const {
    std::vector<std::uint64_t> ids;
    ids.reserve(states_.size());
    for (const auto& [id, state] : states_) if (state.active) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

void EmergencyJumpSystem::clear() {
    states_.clear();
    nextSequence_ = 1;
}

} // namespace elysium
