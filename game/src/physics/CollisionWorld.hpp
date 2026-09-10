// Intended function: Represent renderer-independent collision queries against voxel/world geometry, structures, vehicles, actors, and local gravity frames.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::physics {
struct CollisionQuery {
    std::uint64_t queryId{};
    std::uint64_t actorId{};
    std::uint64_t shapeId{};
    std::uint64_t positionHash{};
    std::uint64_t velocityHash{};
    std::uint64_t flags{};
};
class CollisionQueryIndex {
public:
 bool upsert(CollisionQuery value); bool erase(std::uint64_t id); [[nodiscard]] const CollisionQuery* find(std::uint64_t id) const; [[nodiscard]] const std::vector<CollisionQuery>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const CollisionQuery& value) noexcept; std::vector<CollisionQuery> rows_;
};
}
