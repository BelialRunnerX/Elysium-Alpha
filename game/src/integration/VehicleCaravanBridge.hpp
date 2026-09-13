// Intended function: Promote/demote strategic caravans into local vehicle/cargo actors while preserving stable convoy, manifest, and ownership identity.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct VehiclePromotionIntent {
    std::uint64_t intentId{};
    std::uint64_t caravanId{};
    std::uint64_t vehicleId{};
    std::uint64_t siteId{};
    std::uint64_t manifestId{};
    std::uint64_t state{};
};
class VehiclePromotionIntentIndex {
public:
 bool upsert(VehiclePromotionIntent value); bool erase(std::uint64_t id); [[nodiscard]] const VehiclePromotionIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<VehiclePromotionIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const VehiclePromotionIntent& value) noexcept; std::vector<VehiclePromotionIntent> rows_;
};
}
