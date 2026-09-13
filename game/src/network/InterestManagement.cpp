#include "network/InterestManagement.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Select stable entities, chunks, and events relevant to a future client connection using bounded spatial interest.
bool InterestManagementSystem::apply(const InterestManagementRequest& request) {
    if (request.targetId == 0) return false;
    auto& state = states_[request.targetId];
    state.sequence = nextSequence_++;
    state.actorId = request.actorId;
    state.targetId = request.targetId;
    state.value = request.magnitude;
    state.active = true;
    return true;
}

bool InterestManagementSystem::erase(std::uint64_t targetId) {
    return states_.erase(targetId) != 0;
}

const InterestManagementState* InterestManagementSystem::find(std::uint64_t targetId) const {
    const auto it = states_.find(targetId);
    return it == states_.end() ? nullptr : &it->second;
}

std::vector<std::uint64_t> InterestManagementSystem::activeIds() const {
    std::vector<std::uint64_t> ids;
    ids.reserve(states_.size());
    for (const auto& [id, state] : states_) if (state.active) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

void InterestManagementSystem::clear() {
    states_.clear();
    nextSequence_ = 1;
}

} // namespace elysium
