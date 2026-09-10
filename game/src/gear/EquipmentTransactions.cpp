#include "gear/EquipmentTransactions.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <stdexcept>

namespace elysium::gear {
namespace {
constexpr float Limit = 1.0e6f;
bool scalar(float v) { return std::isfinite(v) && std::abs(v) <= Limit; }
float sum(float a, float b) { return std::clamp(a + b, -Limit, Limit); }
bool same(const EquipmentItem& a, const EquipmentItem& b) {
    return a.id == b.id && a.module == b.module && a.enabled == b.enabled;
}
}
EquipmentTransactions::EquipmentTransactions(StableId owner, ContentId host)
    : owner_(owner), host_(host) {
    if (!owner || !host) throw std::invalid_argument("equipment owner and host must be stable IDs");
}
EquipmentError EquipmentTransactions::publish(FamilyModule d) {
    if (!d.id || !d.family || d.slots.empty() || d.slots.size() > 8 ||
        !scalar(d.energyTax) || d.energyTax < 0 || !scalar(d.oxygenTax) || d.oxygenTax < 0 ||
        !scalar(d.signatureDelta) || !scalar(d.carryDelta) ||
        static_cast<unsigned>(d.obtainability) > static_cast<unsigned>(Obtainability::MissingRecipe) ||
        (d.obtainability == Obtainability::Available && !d.recipe))
        return EquipmentError::InvalidDefinition;
    std::set<unsigned> slots;
    for (auto slot : d.slots)
        if (static_cast<unsigned>(slot) >= 8 || !slots.insert(static_cast<unsigned>(slot)).second)
            return EquipmentError::InvalidDefinition;
    for (auto share : d.hazardShares)
        if (!std::isfinite(share) || share < 0 || share > 1) return EquipmentError::InvalidDefinition;
    auto canonicalIds = [](std::vector<ContentId>& ids) {
        std::sort(ids.begin(), ids.end());
        return (ids.empty() || ids.front() != 0) &&
               std::adjacent_find(ids.begin(), ids.end()) == ids.end();
    };
    if (!canonicalIds(d.compatibleHostFamilies) || !canonicalIds(d.passiveIds))
        return EquipmentError::InvalidDefinition;
    // Definitions are immutable after publication so an equipped slot contract
    // cannot silently change underneath an existing save or transaction.
    return definitions_.emplace(d.id, std::move(d)).second ? EquipmentError::None
                                                         : EquipmentError::InvalidDefinition;
}
EquipmentError EquipmentTransactions::validate(const std::vector<EquipmentItem>& items) const {
    if (items.size() > 8) return EquipmentError::SlotConflict;
    std::set<StableId> ids;
    std::array<StableId,8> occupied{};
    for (const auto& item : items) {
        if (!item.id || item.id == owner_) return EquipmentError::InvalidItem;
        if (!ids.insert(item.id).second) return EquipmentError::DuplicateItem;
        auto it = definitions_.find(item.module);
        if (it == definitions_.end()) return EquipmentError::UnknownModule;
        const auto& d = it->second;
        if (d.obtainability != Obtainability::Available) return EquipmentError::Unavailable;
        if (d.family != host_ && !std::binary_search(d.compatibleHostFamilies.begin(),
                                                   d.compatibleHostFamilies.end(), host_))
            return EquipmentError::IncompatibleFamily;
        // Disabled equipment remains physically equipped and reserves its slots.
        for (auto slot : d.slots) {
            auto& owner = occupied[static_cast<unsigned>(slot)];
            if (owner) return EquipmentError::SlotConflict;
            owner = item.id;
        }
    }
    return EquipmentError::None;
}
EquipmentError EquipmentTransactions::commit(std::vector<EquipmentItem> next, std::uint64_t expected) {
    if (expected != revision_ || revision_ == std::numeric_limits<std::uint64_t>::max())
        return EquipmentError::StaleRevision;
    auto error = validate(next);
    if (error != EquipmentError::None) return error;
    std::sort(next.begin(), next.end(), [](auto& a, auto& b) { return a.id < b.id; });
    if (next.size() == items_.size() && std::equal(next.begin(), next.end(), items_.begin(), same))
        return EquipmentError::None;
    // Stage both state and outbox before changing authoritative data. Allocation
    // failure leaves the old state and pending hooks intact.
    auto pending = hooks_;
    const auto changed = [](const EquipmentItem& item, const std::vector<EquipmentItem>& other) {
        return std::none_of(other.begin(), other.end(), [&](auto& entry) { return same(item, entry); });
    };
    for (const auto& old : items_)
        if (old.enabled && changed(old, next))
            pending.push_back({HookKind::Deactivate, owner_, old.id, old.module, revision_ + 1});
    for (const auto& item : next)
        if (item.enabled && changed(item, items_))
            pending.push_back({HookKind::Activate, owner_, item.id, item.module, revision_ + 1});
    items_.swap(next); hooks_.swap(pending); ++revision_;
    return EquipmentError::None;
}
EquipmentError EquipmentTransactions::equip(EquipmentItem item, std::uint64_t expected) {
    auto next = items_; next.push_back(item); return commit(std::move(next), expected);
}
EquipmentError EquipmentTransactions::remove(StableId id, std::uint64_t expected) {
    auto next = items_;
    auto it = std::find_if(next.begin(), next.end(), [&](auto& x) { return x.id == id; });
    if (it == next.end()) return EquipmentError::InvalidItem;
    next.erase(it); return commit(std::move(next), expected);
}
EquipmentError EquipmentTransactions::setEnabled(StableId id, bool enabled, std::uint64_t expected) {
    auto next = items_;
    auto it = std::find_if(next.begin(), next.end(), [&](auto& x) { return x.id == id; });
    if (it == next.end()) return EquipmentError::InvalidItem;
    it->enabled = enabled; return commit(std::move(next), expected);
}
EquipmentError EquipmentTransactions::restore(const EquipmentSnapshot& record, std::uint64_t expected) {
    if (record.schema != 1 || record.owner != owner_ || record.hostFamily != host_)
        return EquipmentError::InvalidSnapshot;
    // Saved revision is diagnostic; never rewind the live concurrency token.
    return commit(record.items, expected);
}
EquipmentSnapshot EquipmentTransactions::snapshot() const { return {1, owner_, host_, revision_, items_}; }
std::vector<EquipmentHook> EquipmentTransactions::drainHooks() {
    std::vector<EquipmentHook> out; out.swap(hooks_); return out;
}
ModuleContributions EquipmentTransactions::contributions() const {
    ModuleContributions out;
    // items_ is StableId sorted, so float composition and passive dispatch have
    // identical order after load, regardless of command arrival order.
    for (const auto& item : items_) if (item.enabled) {
        const auto& d = definitions_.at(item.module);
        for (std::size_t h = 0; h < kSurvivalHazardCount; ++h)
            out.hazardShares[h] = combineIndependentShares(out.hazardShares[h], d.hazardShares[h]);
        out.energyTax = sum(out.energyTax, d.energyTax); out.oxygenTax = sum(out.oxygenTax, d.oxygenTax);
        out.signatureDelta = sum(out.signatureDelta, d.signatureDelta);
        out.carryDelta = sum(out.carryDelta, d.carryDelta);
        for (auto passive : d.passiveIds) out.passives.push_back({item.id, passive});
    }
    return out;
}
void EquipmentTransactions::applySurvival(const ModuleContributions& modules, SurvivalTickInput& tick) {
    for (std::size_t h = 0; h < kSurvivalHazardCount; ++h)
        tick.protection.modules[h] = combineIndependentShares(tick.protection.modules[h], modules.hazardShares[h]);
    tick.activity.extraEnergyDrainPerSecond = sum(tick.activity.extraEnergyDrainPerSecond, modules.energyTax);
    tick.activity.extraOxygenDrainPerSecond = sum(tick.activity.extraOxygenDrainPerSecond, modules.oxygenTax);
}
}
