// Intended function: Translate stockpile/external haul requests into drone jobs while preserving stable item/container/dependency identities.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct DroneHaulIntent {
    std::uint64_t intentId{};
    std::uint64_t droneId{};
    std::uint64_t sourceId{};
    std::uint64_t destinationId{};
    std::uint64_t itemId{};
    std::uint64_t units{};
};
class DroneHaulIntentIndex {
public:
 bool upsert(DroneHaulIntent value); bool erase(std::uint64_t id); [[nodiscard]] const DroneHaulIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<DroneHaulIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const DroneHaulIntent& value) noexcept; std::vector<DroneHaulIntent> rows_;
};
}
