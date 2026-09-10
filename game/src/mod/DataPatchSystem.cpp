#include "mod/DataPatchSystem.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Apply ordered data-driven patches to registries while preserving immutable stable IDs after registry freeze.
bool DataPatchSystemSystem::apply(const DataPatchSystemRequest& request) {
    if (request.targetId == 0) return false;
    auto& state = states_[request.targetId];
    state.sequence = nextSequence_++;
    state.actorId = request.actorId;
    state.targetId = request.targetId;
    state.value = request.magnitude;
    state.active = true;
    return true;
}

bool DataPatchSystemSystem::erase(std::uint64_t targetId) {
    return states_.erase(targetId) != 0;
}

const DataPatchSystemState* DataPatchSystemSystem::find(std::uint64_t targetId) const {
    const auto it = states_.find(targetId);
    return it == states_.end() ? nullptr : &it->second;
}

std::vector<std::uint64_t> DataPatchSystemSystem::activeIds() const {
    std::vector<std::uint64_t> ids;
    ids.reserve(states_.size());
    for (const auto& [id, state] : states_) if (state.active) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

void DataPatchSystemSystem::clear() {
    states_.clear();
    nextSequence_ = 1;
}

} // namespace elysium
