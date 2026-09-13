#include "simulation/RemoteIndustry.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Advance off-shard industry from aggregate inputs, capacities, outages, workforce, and logistics access.
bool RemoteIndustrySystem::apply(const RemoteIndustryRequest& request) {
    if (request.targetId == 0) return false;
    auto& state = states_[request.targetId];
    state.sequence = nextSequence_++;
    state.actorId = request.actorId;
    state.targetId = request.targetId;
    state.value = request.magnitude;
    state.active = true;
    return true;
}

bool RemoteIndustrySystem::erase(std::uint64_t targetId) {
    return states_.erase(targetId) != 0;
}

const RemoteIndustryState* RemoteIndustrySystem::find(std::uint64_t targetId) const {
    const auto it = states_.find(targetId);
    return it == states_.end() ? nullptr : &it->second;
}

std::vector<std::uint64_t> RemoteIndustrySystem::activeIds() const {
    std::vector<std::uint64_t> ids;
    ids.reserve(states_.size());
    for (const auto& [id, state] : states_) if (state.active) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

void RemoteIndustrySystem::clear() {
    states_.clear();
    nextSequence_ = 1;
}

} // namespace elysium
