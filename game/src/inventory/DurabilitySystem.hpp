// Intended function: Track item/tool/weapon/armor condition, damage causes, repairability, break thresholds, maintenance, and degradation modifiers.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::inventory {
struct DurabilityState {
    std::uint64_t itemId{};
    double condition{};
    double maxCondition{};
    double wearRate{};
    double repairCost{};
    std::uint64_t flags{};
};
class DurabilityStateIndex {
public:
 bool upsert(DurabilityState value); bool erase(std::uint64_t id); [[nodiscard]] const DurabilityState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<DurabilityState>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const DurabilityState& value) noexcept; std::vector<DurabilityState> rows_;
};
}
