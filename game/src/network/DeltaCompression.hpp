// Intended function: Represent future replicated state deltas with base revision, changed-field masks, payload hashes, and compression metadata.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::network {
struct StateDelta {
    std::uint64_t deltaId{};
    std::uint64_t stableId{};
    std::uint64_t baseRevision{};
    std::uint64_t newRevision{};
    std::uint64_t fieldMask{};
    std::uint64_t payloadHash{};
};
class StateDeltaCollection {
public:
 bool store(StateDelta value); bool erase(std::uint64_t id); [[nodiscard]] const StateDelta* find(std::uint64_t id) const; [[nodiscard]] const std::vector<StateDelta>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const StateDelta& v) noexcept; std::vector<StateDelta> rows_;
};
}
