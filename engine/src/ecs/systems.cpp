#include "systems.h"

#include <algorithm>
#include <unordered_map>

namespace ely::ecs {

void motionSystem(entt::registry& reg, double dt) {
    for (auto [entity, transform, velocity] :
         reg.view<Transform, Velocity>().each()) {
        transform.position += velocity.linear * dt;
    }
}

entt::entity activeCamera(entt::registry& reg) {
    auto view = reg.view<Transform, CameraLens, ActiveCamera>();
    for (auto entity : view) return entity;
    return entt::null;
}

ResidencyResult residencySystem(entt::registry& reg, const Streamer& streamer) {
    ResidencyResult result;

    // An index from tile id to entity, rebuilt each call.
    //
    // Rebuilding rather than caching is deliberate: a cached index is a fourth
    // container that has to agree with the other three, and the bug this whole
    // refactor came out of was exactly that — separate maps for residency,
    // build state and GPU geometry that drifted apart, and a world that drained
    // away ahead of a walking camera. A resident set is a few thousand entries;
    // rebuilding it is microseconds and cannot be wrong.
    std::unordered_map<uint64_t, entt::entity> index;
    index.reserve(reg.view<Tile>().size());
    for (auto [entity, tile] : reg.view<Tile>().each())
        index.emplace(tile.id, entity);

    // Anything the streamer dropped is destroyed, geometry and all. Done first
    // so a tile that left range while it was being built never gets uploaded.
    for (const TileKey& key : streamer.toUnload()) {
        auto it = index.find(tileId(key));
        if (it == index.end()) continue;
        reg.destroy(it->second);
        index.erase(it);
        ++result.destroyed;
    }

    // Anything the streamer wants at a level it is not already built at gets a
    // TileWanted, which is the build queue.
    for (const Streamer::Load& load : streamer.toLoad()) {
        const uint64_t id = tileId(load.key);
        auto it = index.find(id);

        if (it == index.end()) {
            const entt::entity entity = reg.create();
            reg.emplace<Tile>(entity, load.key, id);
            reg.emplace<TileBounds>(entity, streamer.boundsOf(load.key));
            reg.emplace<TileWanted>(entity, load.lod);
            index.emplace(id, entity);
            ++result.created;
            continue;
        }

        const entt::entity entity = it->second;

        // Three ways this tile already has the answer, and running the system
        // again must be a no-op in all of them. Idempotence is not tidiness
        // here: the streamer re-emits a load whenever its own view of a tile
        // changes, and a residency pass that rewrites tags each time it is
        // called turns every such moment into a rebuild of work already done or
        // already in flight.
        if (reg.all_of<TileBuilding>(entity)) continue;          // in flight
        const TileWanted* pending = reg.try_get<TileWanted>(entity);
        if (pending && pending->lod == load.lod) continue;        // already queued
        const TileLevel* level = reg.try_get<TileLevel>(entity);
        if (level && level->lod == load.lod) continue;            // already built

        reg.emplace_or_replace<TileWanted>(entity, load.lod);
        ++result.relevelled;
    }
    return result;
}

size_t cullSystem(entt::registry& reg, const Frustum& frustum) {
    reg.clear<TileVisible>();
    size_t visible = 0;
    for (auto [entity, bounds] : reg.view<TileBounds>().each()) {
        if (!frustum.visible(bounds.box)) continue;
        reg.emplace<TileVisible>(entity);
        ++visible;
    }
    return visible;
}

void drawListSystem(const entt::registry& reg, const Vec3& eye,
                    std::vector<uint64_t>& out) {
    struct Entry { uint64_t id; double distanceSquared; };
    static thread_local std::vector<Entry> entries;
    entries.clear();

    // TileVisible is an empty tag, so it filters the view without appearing in
    // the binding; TileGeometry is real data and does.
    for (auto [entity, tile, bounds, geometry] :
         reg.view<Tile, TileBounds, TileGeometry, TileVisible>().each()) {
        (void)geometry;
        const double dx = bounds.box.centreX() - eye.x;
        const double dy = bounds.box.centreY() - eye.y;
        const double dz = bounds.box.centreZ() - eye.z;
        entries.push_back({tile.id, dx * dx + dy * dy + dz * dz});
    }

    std::sort(entries.begin(), entries.end(),
              [](const Entry& a, const Entry& b) {
                  return a.distanceSquared < b.distanceSquared;
              });

    out.clear();
    out.reserve(entries.size());
    for (const Entry& e : entries) out.push_back(e.id);
}

size_t dispatchSystem(entt::registry& reg, const Vec3& eye, int limit,
                      std::vector<BuildRequest>& out) {
    struct Candidate { entt::entity entity; TileKey key; int lod; double d2; };
    static thread_local std::vector<Candidate> candidates;
    candidates.clear();

    for (auto [entity, tile, bounds, wanted] :
         reg.view<Tile, TileBounds, TileWanted>().each()) {
        const double dx = bounds.box.centreX() - eye.x;
        const double dy = bounds.box.centreY() - eye.y;
        const double dz = bounds.box.centreZ() - eye.z;
        candidates.push_back({entity, tile.key, wanted.lod,
                              dx * dx + dy * dy + dz * dz});
    }

    // Nearest first. The tile under the player is the one whose absence is a
    // hole in the floor; a distant one being a frame late is invisible.
    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& a, const Candidate& b) {
                  return a.d2 < b.d2;
              });

    size_t dispatched = 0;
    for (const Candidate& c : candidates) {
        if (limit >= 0 && dispatched >= size_t(limit)) break;
        // The tag moves from wanted to building here, inside the system, so a
        // caller that ignores the returned list still cannot double-dispatch.
        reg.remove<TileWanted>(c.entity);
        reg.emplace<TileBuilding>(c.entity);
        out.push_back(BuildRequest{c.entity, c.key, c.lod});
        ++dispatched;
    }
    return dispatched;
}

}  // namespace ely::ecs
