// Intended function: sparse system materialization policy for the full uint32 galaxy address space.
#include "galaxy/GalaxyResidency.hpp"

#include <algorithm>
#include <limits>
#include "core/Saturating.hpp"

namespace elysium {
namespace {
constexpr std::uint32_t bits(SystemResidencyReason value) noexcept { return std::uint32_t(value); }
constexpr SystemResidencyReason removeBits(SystemResidencyReason value, SystemResidencyReason remove) noexcept {
    return SystemResidencyReason(bits(value) & ~bits(remove));
}
}

GalaxyResidency::GalaxyResidency(SystemResidencyPolicy policy) : policy_(policy) {
    if (policy_.maxHistoryResident != 0 && policy_.maxActive != 0 &&
        policy_.maxHistoryResident < policy_.maxActive) {
        policy_.maxHistoryResident = policy_.maxActive;
    }
}

SystemResidencyTier GalaxyResidency::tierFor(SystemResidencyReason reasons) noexcept {
    if (any(reasons & ActiveReasons)) return SystemResidencyTier::Active;
    if (any(reasons & HistoryReasons)) return SystemResidencyTier::HistoryResident;
    return SystemResidencyTier::AddressOnly;
}

SystemResidencyRecord* GalaxyResidency::findMutable(GalaxySystemId system) {
    auto it = std::lower_bound(records_.begin(), records_.end(), system,
        [](const SystemResidencyRecord& row, GalaxySystemId id){ return row.systemId < id; });
    return it != records_.end() && it->systemId == system ? &*it : nullptr;
}

const SystemResidencyRecord* GalaxyResidency::find(GalaxySystemId system) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), system,
        [](const SystemResidencyRecord& row, GalaxySystemId id){ return row.systemId < id; });
    return it != records_.end() && it->systemId == system ? &*it : nullptr;
}

SystemResidencyRecord& GalaxyResidency::ensure(GalaxySystemId system) {
    auto it = std::lower_bound(records_.begin(), records_.end(), system,
        [](const SystemResidencyRecord& row, GalaxySystemId id){ return row.systemId < id; });
    if (it == records_.end() || it->systemId != system) {
        it = records_.insert(it, SystemResidencyRecord{.systemId = system});
    }
    return *it;
}

SystemResidencyDelta GalaxyResidency::recalc(SystemResidencyRecord& record, std::uint64_t tick) {
    const auto before = record.tier;
    record.tier = tierFor(record.reasons);
    record.lastRequiredTick = tick;
    if (record.tier != SystemResidencyTier::AddressOnly) { record.revision = nextRevision_; nextRevision_ = safe::saturatingIncrement(nextRevision_); }
    return {record.systemId, before, record.tier, record.revision};
}

SystemResidencyDelta GalaxyResidency::require(GalaxySystemId system, SystemResidencyReason reason, std::uint64_t tick) {
    auto& row = ensure(system);
    row.reasons = row.reasons | reason;
    if (any(reason & HistoryReasons)) row.lastHistoryTouchTick = tick;
    return recalc(row, tick);
}

SystemResidencyDelta GalaxyResidency::release(GalaxySystemId system, SystemResidencyReason reason, std::uint64_t tick) {
    auto* row = findMutable(system);
    if (!row) return {system, SystemResidencyTier::AddressOnly, SystemResidencyTier::AddressOnly, 0};
    const auto before = row->tier;
    row->reasons = removeBits(row->reasons, reason);
    // Graceful demotion is handled by collectDemotions; retaining the previous tier
    // prevents frame-boundary churn after the final reason disappears.
    row->lastRequiredTick = tick;
    row->revision = nextRevision_; nextRevision_ = safe::saturatingIncrement(nextRevision_);
    return {system, before, before, row->revision};
}

std::vector<SystemResidencyDelta> GalaxyResidency::setHistoryNeighborhood(
    const std::vector<GalaxySystemId>& systems, std::uint64_t tick) {
    std::vector<GalaxySystemId> next = systems;
    std::sort(next.begin(), next.end());
    next.erase(std::unique(next.begin(), next.end()), next.end());
    if (policy_.maxHistoryResident != 0 && next.size() > policy_.maxHistoryResident) next.resize(policy_.maxHistoryResident);

    std::vector<SystemResidencyDelta> deltas;
    std::vector<GalaxySystemId> removed;
    std::set_difference(historyNeighborhood_.begin(), historyNeighborhood_.end(),
                        next.begin(), next.end(), std::back_inserter(removed));
    for (auto id : removed) deltas.push_back(release(id, SystemResidencyReason::HistoryProximity, tick));

    std::vector<GalaxySystemId> added;
    std::set_difference(next.begin(), next.end(), historyNeighborhood_.begin(), historyNeighborhood_.end(),
                        std::back_inserter(added));
    for (auto id : added) deltas.push_back(require(id, SystemResidencyReason::HistoryProximity, tick));

    for (auto id : next) {
        if (auto* row = findMutable(id)) row->lastHistoryTouchTick = tick;
    }
    historyNeighborhood_ = std::move(next);
    return deltas;
}

void GalaxyResidency::markHistoryDirty(GalaxySystemId system, std::uint64_t tick) {
    auto& row = ensure(system);
    if (row.tier == SystemResidencyTier::AddressOnly) row.tier = SystemResidencyTier::HistoryResident;
    row.historyDirty = true;
    row.lastHistoryTouchTick = tick;
    row.revision = nextRevision_; nextRevision_ = safe::saturatingIncrement(nextRevision_);
}

void GalaxyResidency::markHistoryPersisted(GalaxySystemId system, std::uint64_t tick) {
    if (auto* row = findMutable(system)) {
        row->historyDirty = false;
        row->lastHistoryTouchTick = tick;
        row->revision = nextRevision_; nextRevision_ = safe::saturatingIncrement(nextRevision_);
    }
}

std::vector<SystemResidencyDelta> GalaxyResidency::collectDemotions(std::uint64_t tick) {
    std::vector<SystemResidencyDelta> out;
    for (auto& row : records_) {
        const auto desired = tierFor(row.reasons);
        if (desired == SystemResidencyTier::Active) continue;

        if (row.tier == SystemResidencyTier::Active) {
            const bool expired = tick >= row.lastRequiredTick && tick - row.lastRequiredTick >= policy_.activeGraceTicks;
            if (expired) {
                const auto before = row.tier;
                row.tier = desired == SystemResidencyTier::HistoryResident
                    ? SystemResidencyTier::HistoryResident
                    : (row.historyDirty ? SystemResidencyTier::HistoryResident : SystemResidencyTier::AddressOnly);
                row.revision = nextRevision_; nextRevision_ = safe::saturatingIncrement(nextRevision_);
                out.push_back({row.systemId, before, row.tier, row.revision});
            }
        }

        if (row.tier == SystemResidencyTier::HistoryResident && desired == SystemResidencyTier::AddressOnly && !row.historyDirty) {
            const auto ageBase = std::max(row.lastRequiredTick, row.lastHistoryTouchTick);
            const bool expired = tick >= ageBase && tick - ageBase >= policy_.historyGraceTicks;
            if (expired) {
                const auto before = row.tier;
                row.tier = SystemResidencyTier::AddressOnly;
                row.revision = nextRevision_; nextRevision_ = safe::saturatingIncrement(nextRevision_);
                out.push_back({row.systemId, before, row.tier, row.revision});
            }
        }
    }

    records_.erase(std::remove_if(records_.begin(), records_.end(), [](const auto& row){
        return row.tier == SystemResidencyTier::AddressOnly && row.reasons == SystemResidencyReason::None && !row.historyDirty;
    }), records_.end());
    return out;
}

std::size_t GalaxyResidency::activeCount() const noexcept {
    return std::count_if(records_.begin(), records_.end(), [](const auto& row){ return row.tier == SystemResidencyTier::Active; });
}
std::size_t GalaxyResidency::historyCount() const noexcept {
    return std::count_if(records_.begin(), records_.end(), [](const auto& row){ return row.tier == SystemResidencyTier::HistoryResident; });
}

} // namespace elysium
