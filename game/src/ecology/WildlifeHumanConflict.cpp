// Intended function: Track crop damage, predation, disease, settlement encounters, and mitigation.
#include "ecology/WildlifeHumanConflict.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace elysium::ecology {

bool WildlifeHumanConflictService::apply(const WildlifeHumanConflictCommand& command) {
    if (command.subjectId == 0 || !std::isfinite(command.amount) || !std::isfinite(command.rate)) {
        return false;
    }
    auto* state = findMutable(command.subjectId);
    if (!state) {
        states_.push_back({});
        state = &states_.back();
        state->subjectId = command.subjectId;
    }
    state->revision = revision_++;
    state->ownerId = command.ownerId;
    state->targetId = command.targetId;
    state->amount = command.amount;
    state->accumulated += command.amount;
    state->pressure = std::clamp(state->pressure * 0.75 + std::abs(command.amount) * 0.25, 0.0, 1.0e12);
    state->updatedTick = command.tick;
    state->mode = command.mode;
    state->status = command.flags;
    state->active = (command.flags & 0x80000000u) == 0u;
    if ((command.flags & 0x1u) != 0u) {
        emit(command.subjectId, command.targetId, command.amount, command.tick, command.mode);
    }
    return true;
}

bool WildlifeHumanConflictService::erase(std::uint64_t subjectId) {
    const auto it = std::find_if(states_.begin(), states_.end(), [subjectId](const auto& state) {
        return state.subjectId == subjectId;
    });
    if (it == states_.end()) return false;
    states_.erase(it);
    ++revision_;
    return true;
}

void WildlifeHumanConflictService::advance(std::uint64_t tick, double delta) {
    if (!std::isfinite(delta) || delta <= 0.0) return;
    for (auto& state : states_) {
        if (!state.active) continue;
        const double before = state.accumulated;
        state.accumulated += state.amount * delta;
        state.pressure = std::max(0.0, state.pressure * std::exp(-0.05 * delta));
        state.updatedTick = tick;
        state.revision = revision_++;
        if (std::floor(std::abs(before) / 1000.0) != std::floor(std::abs(state.accumulated) / 1000.0)) {
            emit(state.subjectId, state.targetId, state.accumulated, tick, state.mode);
        }
    }
}

const WildlifeHumanConflictState* WildlifeHumanConflictService::find(std::uint64_t subjectId) const {
    const auto it = std::find_if(states_.begin(), states_.end(), [subjectId](const auto& state) {
        return state.subjectId == subjectId;
    });
    return it == states_.end() ? nullptr : &*it;
}

WildlifeHumanConflictState* WildlifeHumanConflictService::findMutable(std::uint64_t subjectId) {
    const auto it = std::find_if(states_.begin(), states_.end(), [subjectId](const auto& state) {
        return state.subjectId == subjectId;
    });
    return it == states_.end() ? nullptr : &*it;
}

std::vector<WildlifeHumanConflictState> WildlifeHumanConflictService::ordered() const {
    auto result = states_;
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        if (a.ownerId != b.ownerId) return a.ownerId < b.ownerId;
        return a.subjectId < b.subjectId;
    });
    return result;
}

std::vector<WildlifeHumanConflictEvent> WildlifeHumanConflictService::drainEvents() {
    std::sort(events_.begin(), events_.end(), [](const auto& a, const auto& b) {
        if (a.tick != b.tick) return a.tick < b.tick;
        return a.eventId < b.eventId;
    });
    auto out = std::move(events_);
    events_.clear();
    return out;
}

void WildlifeHumanConflictService::clear() {
    states_.clear();
    events_.clear();
    ++revision_;
}

void WildlifeHumanConflictService::emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
                         std::uint64_t tick, std::uint32_t kind) {
    events_.push_back({nextEventId_++, subjectId, targetId, magnitude, tick, kind});
}

} // namespace elysium::ecology
