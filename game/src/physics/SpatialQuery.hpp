// Intended function: Provide bounded stable spatial-query requests for nearby actors/items/machines/sites without planet-global scans.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::physics {
struct SpatialQueryRequest {
    std::uint64_t queryId{};
    std::uint64_t centerKey{};
    double radius{};
    std::uint64_t categoryMask{};
    std::uint64_t maxResults{};
    std::uint64_t flags{};
};
class SpatialQueryRequestIndex {
public:
 bool upsert(SpatialQueryRequest value); bool erase(std::uint64_t id); [[nodiscard]] const SpatialQueryRequest* find(std::uint64_t id) const; [[nodiscard]] const std::vector<SpatialQueryRequest>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const SpatialQueryRequest& value) noexcept; std::vector<SpatialQueryRequest> rows_;
};
}
