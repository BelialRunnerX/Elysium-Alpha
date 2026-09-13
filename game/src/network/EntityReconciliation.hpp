// Intended function: Track predicted/authoritative stable entity revisions, correction requests, rollback windows, and interpolation state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::network {
struct ReconciliationState {
    std::uint64_t stableId{};
    std::uint64_t predictedRevision{};
    std::uint64_t authoritativeRevision{};
    std::uint64_t rollbackTick{};
    double errorMetric{};
    std::uint64_t flags{};
};
class ReconciliationStateCollection {
public:
 bool store(ReconciliationState value); bool erase(std::uint64_t id); [[nodiscard]] const ReconciliationState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ReconciliationState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const ReconciliationState& v) noexcept; std::vector<ReconciliationState> rows_;
};
}
