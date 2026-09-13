// Intended function: Apply trade inspection/smuggling/contract standing deltas to system-local Favor/Suspicion without making economy own faction state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct StandingDeltaIntent {
    std::uint64_t intentId{};
    std::uint64_t systemId{};
    std::uint64_t sourceId{};
    double favorDelta{};
    double suspicionDelta{};
    std::uint64_t reason{};
};
class StandingDeltaIntentIndex {
public:
 bool upsert(StandingDeltaIntent value); bool erase(std::uint64_t id); [[nodiscard]] const StandingDeltaIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<StandingDeltaIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const StandingDeltaIntent& value) noexcept; std::vector<StandingDeltaIntent> rows_;
};
}
