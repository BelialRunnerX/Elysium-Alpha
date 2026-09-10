// Systems: plain functions over views.
//
// None of these owns state, allocates entities outside the registry, or knows
// about the others. That is what makes them testable — every one can be run
// against a registry populated by hand, with no planet, no GPU and no threads.
//
// The order they run in is the frame, and it is fixed for a reason:
//
//   residency  -> which tiles should exist at what level, from the camera
//   dispatch   -> hand what needs building to the workers
//   ingest     -> take back what finished, upload it, record what it was
//   cull       -> mark what the frustum can see
//   drawList   -> order what is visible, nearest first
//
// Residency before dispatch so a tile that left range is dropped before it is
// built. Ingest before cull so geometry that arrived this frame can be drawn
// this frame. Cull before draw for the obvious reason.
#pragma once
#include "components.h"

#include <entt/entt.hpp>

#include <cstdint>
#include <vector>

namespace ely::ecs {

// A stable 64-bit id for a tile.
//
// Packed, not hashed: three 21-bit fields are exact for any coordinate the
// streamer can reach, and a hash would trade that certainty for nothing. A
// collision here would silently draw one piece of the world in another's place.
inline uint64_t tileId(const TileKey& k) {
    const uint64_t x = uint64_t(uint32_t(k.x + (1 << 20))) & 0x1FFFFF;
    const uint64_t y = uint64_t(uint32_t(k.y + (1 << 20))) & 0x1FFFFF;
    const uint64_t z = uint64_t(uint32_t(k.z + (1 << 20))) & 0x1FFFFF;
    return (x << 42) | (y << 21) | z;
}

inline TileKey tileKeyFromId(uint64_t id) {
    return TileKey{int32_t((id >> 42) & 0x1FFFFF) - (1 << 20),
                   int32_t((id >> 21) & 0x1FFFFF) - (1 << 20),
                   int32_t(id & 0x1FFFFF) - (1 << 20)};
}

// Move everything that has a velocity. The whole of physics, for now.
void motionSystem(entt::registry& reg, double dt);

// The active camera's transform and lens, or null if there is not exactly one.
// Returns the entity so a caller can write back to it.
entt::entity activeCamera(entt::registry& reg);

struct ResidencyResult {
    size_t created = 0;
    size_t destroyed = 0;
    size_t relevelled = 0;   // resident already, but wanted at a new level
};

// Sync the tile entities to what the streamer says should be resident.
//
// The streamer remains the authority on *what* and *at which level*; this turns
// its answer into entities. Keeping the two separate is what lets the streaming
// policy — hysteresis, budgets, ordering — be tested without an ECS at all.
ResidencyResult residencySystem(entt::registry& reg, const Streamer& streamer);

// Mark visible tiles. Clears every TileVisible tag first, so the answer is
// always about this frame's frustum and never a leftover from the last one.
size_t cullSystem(entt::registry& reg, const Frustum& frustum);

// Chunk ids of everything visible with geometry, nearest to the eye first, so
// the depth test rejects the most fragments before they are shaded.
void drawListSystem(const entt::registry& reg, const Vec3& eye,
                    std::vector<uint64_t>& out);

// Tiles that want building, nearest first, up to `limit`. Removes TileWanted
// and adds TileBuilding for each one returned — so a tile is dispatched exactly
// once even if the caller drops the result on the floor.
struct BuildRequest {
    entt::entity entity;
    TileKey key;
    int lod;
};
size_t dispatchSystem(entt::registry& reg, const Vec3& eye, int limit,
                      std::vector<BuildRequest>& out);

}  // namespace ely::ecs
