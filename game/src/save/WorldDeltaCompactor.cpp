#include "save/WorldDeltaCompactor.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Compact touched-chunk and stable-object journals while retaining tombstones and exactly-once semantics.
bool WorldDeltaCompactorSystem::apply(const WorldDeltaCompactorRequest& request) {
    if (request.targetId == 0) return false;
    auto& state = states_[request.targetId];
    state.sequence = nextSequence_++;
    state.actorId = request.actorId;
    state.targetId = request.targetId;
    state.value = request.magnitude;
    state.active = true;
    return true;
}

bool WorldDeltaCompactorSystem::erase(std::uint64_t targetId) {
    return states_.erase(targetId) != 0;
}

const WorldDeltaCompactorState* WorldDeltaCompactorSystem::find(std::uint64_t targetId) const {
    const auto it = states_.find(targetId);
    return it == states_.end() ? nullptr : &it->second;
}

std::vector<std::uint64_t> WorldDeltaCompactorSystem::activeIds() const {
    std::vector<std::uint64_t> ids;
    ids.reserve(states_.size());
    for (const auto& [id, state] : states_) if (state.active) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

void WorldDeltaCompactorSystem::clear() {
    states_.clear();
    nextSequence_ = 1;
}

} // namespace elysium
