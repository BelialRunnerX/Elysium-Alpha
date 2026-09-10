// Intended function: Generate deterministic stable POI descriptors from planet fields, site grammar, rarity, faction, history, and biome constraints.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::poi {
struct PoiDescriptor {
    std::uint64_t poiId{};
    std::uint64_t typeId{};
    std::uint64_t siteId{};
    double rarity{};
    std::uint64_t factionId{};
    std::uint64_t seed{};
};
class PoiDescriptorStore {
public:
 bool put(PoiDescriptor v); bool erase(std::uint64_t id);
 [[nodiscard]] const PoiDescriptor* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<PoiDescriptor>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const PoiDescriptor& v) noexcept; std::vector<PoiDescriptor> values_;
};
} // namespace elysium::poi
