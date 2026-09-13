#include "simulation/RemoteCombat.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Resolve off-shard combat from force summaries, terrain/hazard modifiers, objectives, and deterministic rolls.
bool RemoteCombatSystem::apply(const RemoteCombatRequest& request) {
    if (request.targetId == 0) return false;
    auto& state = states_[request.targetId];
    state.sequence = nextSequence_++;
    state.actorId = request.actorId;
    state.targetId = request.targetId;
    state.value = request.magnitude;
    state.active = true;
    return true;
}

bool RemoteCombatSystem::erase(std::uint64_t targetId) {
    return states_.erase(targetId) != 0;
}

const RemoteCombatState* RemoteCombatSystem::find(std::uint64_t targetId) const {
    const auto it = states_.find(targetId);
    return it == states_.end() ? nullptr : &it->second;
}

std::vector<std::uint64_t> RemoteCombatSystem::activeIds() const {
    std::vector<std::uint64_t> ids;
    ids.reserve(states_.size());
    for (const auto& [id, state] : states_) if (state.active) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

void RemoteCombatSystem::clear() {
    states_.clear();
    nextSequence_ = 1;
}

} // namespace elysium
