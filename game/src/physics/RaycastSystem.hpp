// Intended function: Queue deterministic world/actor raycasts for mining, building, weapons, scanning, interactions, sensors, and AI line-of-sight.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::physics {
struct RaycastQuery {
    std::uint64_t queryId{};
    std::uint64_t sourceId{};
    std::uint64_t originHash{};
    std::uint64_t directionHash{};
    double maxDistance{};
    std::uint64_t mask{};
};
class RaycastQueryIndex {
public:
 bool upsert(RaycastQuery value); bool erase(std::uint64_t id); [[nodiscard]] const RaycastQuery* find(std::uint64_t id) const; [[nodiscard]] const std::vector<RaycastQuery>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const RaycastQuery& value) noexcept; std::vector<RaycastQuery> rows_;
};
}
