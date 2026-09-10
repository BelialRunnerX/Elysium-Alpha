#include "strategy/CivilizationPlanner.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Generate civilization-scale goals for expansion, defense, trade, research, migration, and diplomacy.
bool CivilizationPlannerSystem::apply(const CivilizationPlannerRequest& request) {
    if (request.targetId == 0) return false;
    auto& state = states_[request.targetId];
    state.sequence = nextSequence_++;
    state.actorId = request.actorId;
    state.targetId = request.targetId;
    state.value = request.magnitude;
    state.active = true;
    return true;
}

bool CivilizationPlannerSystem::erase(std::uint64_t targetId) {
    return states_.erase(targetId) != 0;
}

const CivilizationPlannerState* CivilizationPlannerSystem::find(std::uint64_t targetId) const {
    const auto it = states_.find(targetId);
    return it == states_.end() ? nullptr : &it->second;
}

std::vector<std::uint64_t> CivilizationPlannerSystem::activeIds() const {
    std::vector<std::uint64_t> ids;
    ids.reserve(states_.size());
    for (const auto& [id, state] : states_) if (state.active) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

void CivilizationPlannerSystem::clear() {
    states_.clear();
    nextSequence_ = 1;
}

} // namespace elysium
