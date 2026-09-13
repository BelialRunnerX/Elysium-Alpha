#include "save/SchemaMigrationRegistry.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Register deterministic save-schema migration steps and reject ambiguous or unsupported upgrade paths.
bool SchemaMigrationRegistrySystem::apply(const SchemaMigrationRegistryRequest& request) {
    if (request.targetId == 0) return false;
    auto& state = states_[request.targetId];
    state.sequence = nextSequence_++;
    state.actorId = request.actorId;
    state.targetId = request.targetId;
    state.value = request.magnitude;
    state.active = true;
    return true;
}

bool SchemaMigrationRegistrySystem::erase(std::uint64_t targetId) {
    return states_.erase(targetId) != 0;
}

const SchemaMigrationRegistryState* SchemaMigrationRegistrySystem::find(std::uint64_t targetId) const {
    const auto it = states_.find(targetId);
    return it == states_.end() ? nullptr : &it->second;
}

std::vector<std::uint64_t> SchemaMigrationRegistrySystem::activeIds() const {
    std::vector<std::uint64_t> ids;
    ids.reserve(states_.size());
    for (const auto& [id, state] : states_) if (state.active) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

void SchemaMigrationRegistrySystem::clear() {
    states_.clear();
    nextSequence_ = 1;
}

} // namespace elysium
