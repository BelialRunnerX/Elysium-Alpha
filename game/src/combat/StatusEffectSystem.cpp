// Intended function: Apply deterministic timed combat effects, stacking policies, periodic pulses, immunity tags, and cleanse requests.
#include "StatusEffectSystem.hpp"

namespace elysium::combat {

std::uint64_t StatusEffectInstanceStore::keyOf(const StatusEffectInstance& value) noexcept { return static_cast<std::uint64_t>(value.instanceId); }

bool StatusEffectInstanceStore::upsert(StatusEffectInstance value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const StatusEffectInstance& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool StatusEffectInstanceStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const StatusEffectInstance& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const StatusEffectInstance* StatusEffectInstanceStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const StatusEffectInstance& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<StatusEffectInstance> StatusEffectInstanceStore::ordered() const { return records_; }

} // namespace elysium::combat
