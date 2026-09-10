#include "fortress/Systems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

float computeRoomQuality(RoomQuality& quality) {
    const float areaScore = std::log1p(std::max(0.0f, quality.area)) * 0.10f;
    const float valueScore = std::log1p(std::max(0.0f, quality.materialValue)) * 0.06f;
    const float craftsmanship = saturate(quality.craftsmanship) * 0.16f;
    const float clean = saturate(quality.cleanliness) * 0.14f;
    const float decor = saturate(quality.decor) * 0.10f;
    const float privacy = saturate(quality.privacy) * 0.09f;
    const float quiet = (1.0f - saturate(quality.noise)) * 0.08f;
    const float comfort = saturate(quality.comfort) * 0.10f;
    const float safety = saturate(quality.safety) * 0.10f;
    const float view = saturate(quality.view) * 0.03f;
    const float culture = saturate(quality.culturalMatch) * 0.04f;
    quality.composite = std::clamp(areaScore + valueScore + craftsmanship + clean + decor + privacy + quiet + comfort + safety + view + culture, 0.0f, 2.0f);
    return quality.composite;
}

bool stockpileAccepts(const StockpileComponent& stockpile, const ItemState& item) {
    if (stockpile.used + item.quantity > stockpile.capacity) return false;
    if (item.contamination > stockpile.filter.maxContamination) return false;
    if (item.quality < stockpile.filter.minQuality) return false;
    if (!stockpile.filter.allowOwned && static_cast<bool>(item.owner)) return false;
    if (!stockpile.filter.allowArtifacts && item.identityTier == ItemIdentityTier::Artifact) return false;

    if (!stockpile.filter.materials.empty()) {
        const bool materialMatch = std::any_of(stockpile.filter.materials.begin(), stockpile.filter.materials.end(),
                                               [&](const ContentId& id) { return id == item.material; });
        if (!materialMatch) return false;
    }
    if (!stockpile.filter.categories.empty()) {
        const bool categoryMatch = std::any_of(stockpile.filter.categories.begin(), stockpile.filter.categories.end(),
                                               [&](const ContentId& id) { return id == item.item; });
        if (!categoryMatch) return false;
    }
    return true;
}

float stockpileScore(const StockpileComponent& stockpile, const ItemState& item, float distance) {
    if (!stockpileAccepts(stockpile, item)) return -std::numeric_limits<float>::infinity();
    float score = 0.0f;
    switch (stockpile.priority) {
        case PriorityBand::Background: score += 0.0f; break;
        case PriorityBand::Low: score += 10.0f; break;
        case PriorityBand::Normal: score += 25.0f; break;
        case PriorityBand::High: score += 45.0f; break;
        case PriorityBand::Urgent: score += 70.0f; break;
        case PriorityBand::Emergency: score += 100.0f; break;
    }
    const float fill = stockpile.capacity > 0.0f ? stockpile.used / stockpile.capacity : 1.0f;
    score -= saturate(fill) * 25.0f;
    score -= std::max(0.0f, distance) * 0.25f;
    score -= item.contamination * 4.0f;
    return score;
}

std::optional<HaulCandidate> chooseStockpile(const ItemState& item,
                                             std::span<const StockpileComponent> stockpiles,
                                             const std::function<float(StockpileId)>& distanceFn) {
    std::optional<HaulCandidate> best;
    for (const auto& stockpile : stockpiles) {
        const float score = stockpileScore(stockpile, item, distanceFn ? distanceFn(stockpile.id) : 0.0f);
        if (!std::isfinite(score)) continue;
        if (!best || score > best->score || (score == best->score && stockpile.id.value < best->stockpile.value)) {
            best = HaulCandidate{item.id, stockpile.id, score};
        }
    }
    return best;
}

} // namespace elysium::fortress
