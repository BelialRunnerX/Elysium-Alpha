#include "gameplay/DynamicQuestGenerator.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Generate context-aware contracts and quest chains from settlement needs, threats, discoveries, and faction goals.
bool DynamicQuestGeneratorSystem::apply(const DynamicQuestGeneratorRequest& request) {
    if (request.targetId == 0) return false;
    auto& state = states_[request.targetId];
    state.sequence = nextSequence_++;
    state.actorId = request.actorId;
    state.targetId = request.targetId;
    state.value = request.magnitude;
    state.active = true;
    return true;
}

bool DynamicQuestGeneratorSystem::erase(std::uint64_t targetId) {
    return states_.erase(targetId) != 0;
}

const DynamicQuestGeneratorState* DynamicQuestGeneratorSystem::find(std::uint64_t targetId) const {
    const auto it = states_.find(targetId);
    return it == states_.end() ? nullptr : &it->second;
}

std::vector<std::uint64_t> DynamicQuestGeneratorSystem::activeIds() const {
    std::vector<std::uint64_t> ids;
    ids.reserve(states_.size());
    for (const auto& [id, state] : states_) if (state.active) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

void DynamicQuestGeneratorSystem::clear() {
    states_.clear();
    nextSequence_ = 1;
}

} // namespace elysium
