#include "faction/RegisterActions.hpp"
#include "core/Saturating.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace elysium::faction {
namespace {

std::uint64_t mix64(std::uint64_t x) noexcept {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

bool terminal(RegisterPhase phase) noexcept {
    return phase == RegisterPhase::Succeeded || phase == RegisterPhase::Failed ||
           phase == RegisterPhase::Cancelled;
}

bool validBand(RegisterBand band) noexcept {
    return static_cast<unsigned>(band) <= static_cast<unsigned>(RegisterBand::Hunted);
}

bool validPhase(RegisterPhase phase) noexcept {
    return static_cast<unsigned>(phase) <= static_cast<unsigned>(RegisterPhase::Cancelled);
}

std::size_t expectedWaveCount(RegisterBand band) noexcept {
    switch (band) {
        case RegisterBand::Noted: return 1;
        case RegisterBand::Marked: return 3;
        case RegisterBand::Hunted: return 5;
    }
    return 0;
}

bool validPolicy(const RegisterPolicy& policy) {
    if (!validBand(policy.band) || !std::isfinite(policy.minimumSuspicion) ||
        policy.minimumSuspicion < 0.0 || policy.minimumSuspicion > 100.0 ||
        policy.preparationTicks == 0 || policy.waves.size() != expectedWaveCount(policy.band) ||
        !std::isfinite(policy.pressureRelief) || policy.pressureRelief < 0.0 ||
        !std::isfinite(policy.successFloorHint) || policy.successFloorHint < 0.0 ||
        policy.successFloorHint > 100.0) {
        return false;
    }

    std::set<std::uint32_t> ordinals;
    for (std::size_t i = 0; i < policy.waves.size(); ++i) {
        const auto& wave = policy.waves[i];
        if (wave.encounterTemplate == 0 || wave.ordinal != i || !ordinals.insert(wave.ordinal).second)
            return false;
        if (wave.praetorStyle && (policy.band != RegisterBand::Hunted || i + 1 != policy.waves.size()))
            return false;
    }
    if (policy.band == RegisterBand::Hunted && !policy.waves.back().praetorStyle) return false;
    return true;
}

bool validActionIdentity(const RegisterAction& action) noexcept {
    return action.id != 0 && action.player != 0 && action.system != 0 &&
           action.claim != 0 && action.beacon != 0;
}

} // namespace

bool RegisterActions::fail(RegisterFailure failure) noexcept {
    lastFailure_ = failure;
    return false;
}

bool RegisterActions::publish(RegisterPolicy policy) {
    if (!validPolicy(policy)) return fail(RegisterFailure::InvalidPolicy);
    if (!policies_.emplace(policy.band, std::move(policy)).second)
        return fail(RegisterFailure::InvalidPolicy);
    lastFailure_ = RegisterFailure::None;
    return true;
}

bool RegisterActions::replacePolicy(RegisterPolicy policy) {
    if (!validPolicy(policy)) return fail(RegisterFailure::InvalidPolicy);
    policies_[policy.band] = std::move(policy);
    lastFailure_ = RegisterFailure::None;
    return true;
}

std::optional<RegisterBand> RegisterActions::bandForSuspicion(
    double suspicion,
    const std::map<RegisterBand, RegisterPolicy>& policies) {
    if (!std::isfinite(suspicion)) return std::nullopt;
    suspicion = safe::finiteClamp(suspicion, 0.0, 100.0);
    const RegisterPolicy* selected = nullptr;
    for (const auto& [_, policy] : policies) {
        if (suspicion >= policy.minimumSuspicion &&
            (!selected || policy.minimumSuspicion > selected->minimumSuspicion)) {
            selected = &policy;
        }
    }
    return selected ? std::optional<RegisterBand>(selected->band) : std::nullopt;
}

std::uint64_t RegisterActions::waveSeed(const RegisterAction& action) const noexcept {
    const auto index = static_cast<std::uint64_t>(action.wave) + 1ULL;
    const auto templateId = action.wave < action.policy.waves.size()
        ? action.policy.waves[action.wave].encounterTemplate
        : 0ULL;
    return mix64(action.seed ^ mix64(action.id) ^ mix64(index) ^ mix64(templateId));
}

void RegisterActions::emit(const RegisterAction& action,
                           RegisterNoticeKind kind,
                           std::uint64_t content,
                           double pressureRelief,
                           double floorHint) {
    RegisterNotice notice{};
    notice.sequence = nextNoticeSequence_;
    nextNoticeSequence_ = safe::saturatingIncrement(nextNoticeSequence_);
    notice.action = action.id;
    notice.revision = action.revision;
    notice.player = action.player;
    notice.system = action.system;
    notice.claim = action.claim;
    notice.target = action.beacon;
    notice.content = content;
    notice.deterministicSeed = waveSeed(action);
    notice.dueTick = action.dueTick;
    notice.kind = kind;
    notice.pressureRelief = safe::finiteClamp(pressureRelief, 0.0, 100.0);
    notice.floorHint = safe::finiteClamp(floorHint, 0.0, 100.0);
    notices_.push_back(notice);
}

bool RegisterActions::schedule(RegisterAction action, double suspicionSnapshot, std::uint64_t now) {
    if (!validActionIdentity(action)) return fail(RegisterFailure::InvalidIdentity);
    if (!std::isfinite(suspicionSnapshot)) return fail(RegisterFailure::InvalidSuspicion);
    if (now < clock_) return fail(RegisterFailure::ClockRegression);
    if (actions_.contains(action.id)) return fail(RegisterFailure::DuplicateAction);

    const auto selectedBand = bandForSuspicion(suspicionSnapshot, policies_);
    if (!selectedBand) return fail(RegisterFailure::NoEligiblePolicy);
    const auto policyIt = policies_.find(*selectedBand);
    if (policyIt == policies_.end() || !validPolicy(policyIt->second))
        return fail(RegisterFailure::NoEligiblePolicy);

    for (const auto& [_, existing] : actions_) {
        if (!terminal(existing.phase) && existing.player == action.player && existing.system == action.system)
            return fail(RegisterFailure::AlreadyActiveForPlayerSystem);
    }

    const auto preparation = policyIt->second.preparationTicks;
    if (preparation > std::numeric_limits<std::uint64_t>::max() - now)
        return fail(RegisterFailure::TickOverflow);

    action.policy = policyIt->second;
    action.band = policyIt->second.band;
    action.phase = RegisterPhase::Announced;
    action.wave = 0;
    action.revision = 1;
    action.dueTick = now + preparation;
    clock_ = now;
    actions_.emplace(action.id, action);
    emit(action, RegisterNoticeKind::Announcement);
    lastFailure_ = RegisterFailure::None;
    return true;
}

bool RegisterActions::reschedule(std::uint64_t id,
                                 std::uint64_t revision,
                                 std::uint64_t dueTick) {
    auto it = actions_.find(id);
    if (it == actions_.end()) return fail(RegisterFailure::InvalidIdentity);
    auto& action = it->second;
    if (action.revision != revision || revision == std::numeric_limits<std::uint64_t>::max())
        return fail(RegisterFailure::RevisionConflict);
    if (action.phase != RegisterPhase::Announced && action.phase != RegisterPhase::InterWave)
        return fail(RegisterFailure::InvalidPhase);
    if (dueTick < clock_) return fail(RegisterFailure::ClockRegression);

    action.dueTick = dueTick;
    action.revision = safe::saturatingIncrement(action.revision);
    emit(action, RegisterNoticeKind::Rescheduled);
    lastFailure_ = RegisterFailure::None;
    return true;
}

bool RegisterActions::cancel(std::uint64_t id, std::uint64_t revision) {
    auto it = actions_.find(id);
    if (it == actions_.end()) return fail(RegisterFailure::InvalidIdentity);
    auto& action = it->second;
    if (action.revision != revision || revision == std::numeric_limits<std::uint64_t>::max())
        return fail(RegisterFailure::RevisionConflict);
    if (terminal(action.phase)) return fail(RegisterFailure::InvalidPhase);

    action.phase = RegisterPhase::Cancelled;
    action.revision = safe::saturatingIncrement(action.revision);
    emit(action, RegisterNoticeKind::Cancelled);
    lastFailure_ = RegisterFailure::None;
    return true;
}

bool RegisterActions::advance(std::uint64_t now) {
    if (now < clock_) return fail(RegisterFailure::ClockRegression);
    clock_ = now;

    for (auto& [_, action] : actions_) {
        if ((action.phase != RegisterPhase::Announced && action.phase != RegisterPhase::InterWave) ||
            action.dueTick > now) {
            continue;
        }
        if (action.wave >= action.policy.waves.size()) return fail(RegisterFailure::InvalidSnapshot);
        if (action.revision == std::numeric_limits<std::uint64_t>::max())
            return fail(RegisterFailure::RevisionConflict);

        action.phase = RegisterPhase::Fighting;
        action.revision = safe::saturatingIncrement(action.revision);
        emit(action,
             RegisterNoticeKind::WaveRequested,
             action.policy.waves[action.wave].encounterTemplate);
    }

    lastFailure_ = RegisterFailure::None;
    return true;
}

bool RegisterActions::finishWave(std::uint64_t id,
                                 std::uint64_t revision,
                                 bool success,
                                 std::uint64_t now) {
    auto it = actions_.find(id);
    if (it == actions_.end()) return fail(RegisterFailure::InvalidIdentity);
    if (now < clock_) return fail(RegisterFailure::ClockRegression);

    auto& action = it->second;
    if (action.revision != revision || revision == std::numeric_limits<std::uint64_t>::max())
        return fail(RegisterFailure::RevisionConflict);
    if (action.phase != RegisterPhase::Fighting || action.wave >= action.policy.waves.size())
        return fail(RegisterFailure::InvalidPhase);

    clock_ = now;
    action.revision = safe::saturatingIncrement(action.revision);

    if (!success) {
        action.phase = RegisterPhase::Failed;
        // Failure can only request a strike against the claim/beacon. This scheduler
        // has no API capable of deleting terrain, structures, chunks, or saved deltas.
        emit(action, RegisterNoticeKind::BeaconStrike);
        lastFailure_ = RegisterFailure::None;
        return true;
    }

    emit(action,
         RegisterNoticeKind::WaveCleared,
         action.policy.waves[action.wave].encounterTemplate);

    ++action.wave;
    if (action.wave >= action.policy.waves.size()) {
        action.phase = RegisterPhase::Succeeded;
        action.dueTick = now;
        emit(action,
             RegisterNoticeKind::Success,
             action.policy.rewardContent,
             action.policy.pressureRelief,
             action.policy.successFloorHint);
        if (action.policy.pressureRelief > 0.0) {
            emit(action,
                 RegisterNoticeKind::StandingRelief,
                 0,
                 action.policy.pressureRelief,
                 action.policy.successFloorHint);
        }
        if (action.policy.rewardContent != 0)
            emit(action, RegisterNoticeKind::RewardGranted, action.policy.rewardContent);
        lastFailure_ = RegisterFailure::None;
        return true;
    }

    if (action.policy.waveGapTicks > std::numeric_limits<std::uint64_t>::max() - now)
        return fail(RegisterFailure::TickOverflow);
    action.phase = RegisterPhase::InterWave;
    action.dueTick = now + action.policy.waveGapTicks;
    emit(action, RegisterNoticeKind::Rescheduled);
    lastFailure_ = RegisterFailure::None;
    return true;
}

const RegisterAction* RegisterActions::find(std::uint64_t id) const {
    const auto it = actions_.find(id);
    return it == actions_.end() ? nullptr : &it->second;
}

std::optional<RegisterObservation> RegisterActions::observe(std::uint64_t id,
                                                            std::uint64_t now) const {
    const auto* action = find(id);
    if (!action) return std::nullopt;

    RegisterObservation out{};
    out.action = action->id;
    out.revision = action->revision;
    out.player = action->player;
    out.system = action->system;
    out.claim = action->claim;
    out.beacon = action->beacon;
    out.band = action->band;
    out.phase = action->phase;
    out.currentWave = action->wave;
    out.totalWaves = static_cast<std::uint32_t>(action->policy.waves.size());
    out.dueTick = action->dueTick;
    out.remainingTicks = action->dueTick > now ? action->dueTick - now : 0;
    out.nextEncounterTemplate = action->wave < action->policy.waves.size()
        ? action->policy.waves[action->wave].encounterTemplate
        : 0;
    out.preparationWindow = action->phase == RegisterPhase::Announced;
    out.terminal = terminal(action->phase);
    return out;
}

std::vector<RegisterObservation> RegisterActions::observeAll(std::uint64_t now) const {
    std::vector<RegisterObservation> out;
    out.reserve(actions_.size());
    for (const auto& [id, _] : actions_) {
        if (auto observation = observe(id, now)) out.push_back(*observation);
    }
    return out;
}

RegisterSnapshot RegisterActions::snapshot() const {
    RegisterSnapshot out{};
    out.clock = clock_;
    out.nextNoticeSequence = nextNoticeSequence_;
    for (const auto& [_, policy] : policies_) out.policies.push_back(policy);
    for (const auto& [_, action] : actions_) out.actions.push_back(action);
    out.outbox = notices_;
    return out;
}

bool RegisterActions::restore(const RegisterSnapshot& snapshot) {
    std::map<RegisterBand, RegisterPolicy> nextPolicies;
    std::map<std::uint64_t, RegisterAction> nextActions;

    if (snapshot.nextNoticeSequence == 0) return fail(RegisterFailure::InvalidSnapshot);

    for (const auto& policy : snapshot.policies) {
        if (!validPolicy(policy) || !nextPolicies.emplace(policy.band, policy).second)
            return fail(RegisterFailure::InvalidSnapshot);
    }

    for (const auto& action : snapshot.actions) {
        if (!validActionIdentity(action) || !validBand(action.band) || !validPhase(action.phase) ||
            action.revision == 0 || !validPolicy(action.policy) || action.band != action.policy.band ||
            action.wave > action.policy.waves.size() ||
            (!terminal(action.phase) && action.wave >= action.policy.waves.size()) ||
            !nextActions.emplace(action.id, action).second) {
            return fail(RegisterFailure::InvalidSnapshot);
        }
    }

    std::uint64_t previousSequence = 0;
    for (const auto& notice : snapshot.outbox) {
        const auto actionIt = nextActions.find(notice.action);
        if (notice.sequence == 0 || notice.sequence <= previousSequence ||
            actionIt == nextActions.end() || notice.revision > actionIt->second.revision ||
            static_cast<unsigned>(notice.kind) > static_cast<unsigned>(RegisterNoticeKind::Cancelled) ||
            !std::isfinite(notice.pressureRelief) || notice.pressureRelief < 0.0 ||
            !std::isfinite(notice.floorHint) || notice.floorHint < 0.0 || notice.floorHint > 100.0) {
            return fail(RegisterFailure::InvalidSnapshot);
        }
        previousSequence = notice.sequence;
    }

    if (previousSequence >= snapshot.nextNoticeSequence &&
        snapshot.nextNoticeSequence != std::numeric_limits<std::uint64_t>::max()) {
        return fail(RegisterFailure::InvalidSnapshot);
    }

    policies_.swap(nextPolicies);
    actions_.swap(nextActions);
    notices_ = snapshot.outbox;
    clock_ = snapshot.clock;
    nextNoticeSequence_ = snapshot.nextNoticeSequence;
    lastFailure_ = RegisterFailure::None;
    return true;
}

std::vector<RegisterNotice> RegisterActions::drain() {
    std::vector<RegisterNotice> out;
    out.swap(notices_);
    return out;
}

} // namespace elysium::faction
