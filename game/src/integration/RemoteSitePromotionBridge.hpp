// Intended function: Promote compact remote-site population/items/vehicles/threats into bounded active-shard materialization and demote back to summaries.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct SitePromotionIntent {
    std::uint64_t intentId{};
    std::uint64_t siteId{};
    std::uint64_t targetTier{};
    double populationBudget{};
    double actorBudget{};
    std::uint64_t revision{};
};
class SitePromotionIntentIndex {
public:
 bool upsert(SitePromotionIntent value); bool erase(std::uint64_t id); [[nodiscard]] const SitePromotionIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<SitePromotionIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const SitePromotionIntent& value) noexcept; std::vector<SitePromotionIntent> rows_;
};
}
