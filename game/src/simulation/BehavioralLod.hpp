// Intended function: Track active/local/remote representation state, promotion requirements, demotion summaries, and equivalence revisions.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::simulation {
struct LodEntityState {
    std::uint64_t stableId{};
    std::uint64_t lodTier{};
    std::uint64_t siteId{};
    double importance{};
    std::uint64_t lastActiveTick{};
    std::uint64_t revision{};
};
class LodEntityStateTable {
public:
 bool set(LodEntityState value); bool remove(std::uint64_t id);
 [[nodiscard]] const LodEntityState* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<LodEntityState> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const LodEntityState& value) noexcept; std::vector<LodEntityState> rows_;
};
}
