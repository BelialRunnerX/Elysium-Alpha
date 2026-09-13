// Intended function: Represent UI scale, contrast, motion reduction, subtitle/caption policy, color-independent indicators, input assists, and text timing.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct AccessibilityState {
    std::uint64_t profileId{};
    double uiScale{};
    double contrast{};
    double motionScale{};
    std::uint64_t captionMode{};
    std::uint64_t flags{};
};
class AccessibilityStateCollection {
public:
 bool store(AccessibilityState value); bool erase(std::uint64_t id); [[nodiscard]] const AccessibilityState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<AccessibilityState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const AccessibilityState& v) noexcept; std::vector<AccessibilityState> rows_;
};
}
