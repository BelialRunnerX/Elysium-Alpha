// Intended function: Apply deterministic timed combat effects, stacking policies, periodic pulses, immunity tags, and cleanse requests.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::combat {

struct StatusEffectInstance {
    std::uint64_t instanceId{};
    std::uint64_t effectId{};
    std::uint64_t sourceId{};
    std::uint64_t remainingTicks{};
    std::uint64_t stacks{};
    double magnitude{};
};

class StatusEffectInstanceStore {
public:
    bool upsert(StatusEffectInstance value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const StatusEffectInstance* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<StatusEffectInstance> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const StatusEffectInstance& value) noexcept;
    std::vector<StatusEffectInstance> records_;
};

} // namespace elysium::combat
