// ECS tests.
//
// Every system here is a plain function over a registry, so all of this runs
// with no planet, no GPU and no threads — which is the whole argument for
// having moved tile state into an ECS in the first place. The bug that motivated
// it (a tile that meshed to nothing was never recorded as built, so the workers
// rebuilt empty sky forever while the visible world drained away) is expressible
// as a three-line test down at the bottom.
#include "harness.h"

#include "ecs/components.h"
#include "ecs/systems.h"
#include "gfx/frustum.h"
#include "gfx/mat.h"
#include "gfx/streamer.h"

#include <cmath>
#include <cstdio>
#include <vector>

using namespace ely;
using namespace ely::ecs;
using elytest::section;

static void testTileIds() {
    section("tile identity");

    // Ids are packed, not hashed, so this must round-trip exactly for every
    // coordinate the streamer can reach. A collision would silently draw one
    // piece of the world in another's place.
    const int32_t values[] = {0, 1, -1, 7, -7, 1000, -1000, 100000, -100000,
                              (1 << 19), -(1 << 19)};
    bool roundTrips = true;
    for (int32_t x : values)
        for (int32_t y : values)
            for (int32_t z : values) {
                const TileKey key{x, y, z};
                const TileKey back = tileKeyFromId(tileId(key));
                if (!(back == key)) roundTrips = false;
            }
    CHECK(roundTrips, "tile ids do not round-trip through the packed form");

    // Distinctness on a dense block, which is the case that actually occurs.
    std::vector<uint64_t> ids;
    for (int32_t x = -6; x <= 6; ++x)
        for (int32_t y = -2; y <= 2; ++y)
            for (int32_t z = -6; z <= 6; ++z)
                ids.push_back(tileId(TileKey{x, y, z}));
    std::sort(ids.begin(), ids.end());
    CHECK(std::adjacent_find(ids.begin(), ids.end()) == ids.end(),
          "two different tiles share an id");
}

static void testMotion() {
    section("motion");

    entt::registry reg;
    const entt::entity moving = reg.create();
    reg.emplace<Transform>(moving, Vec3{0, 0, 0}, 0.0, 0.0);
    reg.emplace<Velocity>(moving, Vec3{2, 0, -1});

    // An entity with a transform and no velocity must not move. Obvious, and
    // exactly the kind of thing a view with the wrong component list breaks.
    const entt::entity still = reg.create();
    reg.emplace<Transform>(still, Vec3{5, 5, 5}, 0.0, 0.0);

    motionSystem(reg, 0.5);
    const Transform& a = reg.get<Transform>(moving);
    const Transform& b = reg.get<Transform>(still);
    CHECK(std::fabs(a.position.x - 1.0) < 1e-12 &&
          std::fabs(a.position.z + 0.5) < 1e-12,
          "motion moved to (%.3f, %.3f); expected (1.000, -0.500)",
          a.position.x, a.position.z);
    CHECK(b.position.x == 5.0 && b.position.y == 5.0,
          "an entity with no velocity moved");
}

static void testActiveCamera() {
    section("active camera");

    entt::registry reg;
    CHECK(activeCamera(reg) == entt::null, "an empty world had an active camera");

    const entt::entity other = reg.create();
    reg.emplace<Transform>(other, Vec3{1, 1, 1}, 0.0, 0.0);
    CHECK(activeCamera(reg) == entt::null,
          "an entity with only a transform was taken for the camera");

    const entt::entity cam = reg.create();
    reg.emplace<Transform>(cam, Vec3{0, 10, 0}, 0.0, 0.0);
    reg.emplace<CameraLens>(cam);
    reg.emplace<ActiveCamera>(cam);
    CHECK(activeCamera(reg) == cam, "the active camera was not found");
}

// A streamer with a known residency, for driving the residency system.
static Streamer makeStreamer(double viewDistance, int budget) {
    StreamerConfig cfg;
    cfg.tileSize = 32.0;
    cfg.viewDistance = viewDistance;
    cfg.nearDistance = 48.0;
    cfg.finestLod = 0;
    cfg.coarsestLod = 4;
    cfg.verticalTiles = 1;
    cfg.maxLoadsPerUpdate = budget;
    return Streamer(cfg);
}

static void testResidency() {
    section("tile residency");

    entt::registry reg;
    Streamer streamer = makeStreamer(160.0, 100000);
    streamer.update(Vec3{0, 0, 0}, nullptr);

    ResidencyResult first = residencySystem(reg, streamer);
    CHECK(first.created > 0, "residency created no tiles");
    CHECK(first.created == streamer.residentCount(),
          "residency created %zu entities for %zu resident tiles",
          first.created, streamer.residentCount());
    CHECK(reg.view<Tile>().size() == first.created,
          "the registry does not hold one entity per resident tile");
    CHECK(reg.view<TileWanted>().size() == first.created,
          "not every new tile was queued for building");

    // Running it again with nothing changed must do nothing at all. A system
    // that re-queues stable tiles is the bug that starves the workers.
    ResidencyResult second = residencySystem(reg, streamer);
    CHECK(second.created == 0 && second.destroyed == 0 && second.relevelled == 0,
          "an unchanged camera produced %zu creates, %zu destroys, %zu relevels",
          second.created, second.destroyed, second.relevelled);

    // Move far away: everything should be destroyed and a new set created.
    const size_t before = reg.view<Tile>().size();
    streamer.update(Vec3{100000, 0, 100000}, nullptr);
    ResidencyResult third = residencySystem(reg, streamer);
    CHECK(third.destroyed == before,
          "a teleport destroyed %zu of %zu tiles", third.destroyed, before);
    CHECK(reg.view<Tile>().size() == streamer.residentCount(),
          "after a teleport the registry holds %zu tiles for %zu resident",
          size_t(reg.view<Tile>().size()), streamer.residentCount());
}

static void testDispatch() {
    section("build dispatch");

    entt::registry reg;
    Streamer streamer = makeStreamer(120.0, 100000);
    streamer.update(Vec3{0, 0, 0}, nullptr);
    residencySystem(reg, streamer);

    const size_t wanted = reg.view<TileWanted>().size();
    CHECK(wanted > 8, "only %zu tiles wanted; the test needs more", wanted);

    std::vector<BuildRequest> batch;
    const size_t sent = dispatchSystem(reg, Vec3{0, 0, 0}, 8, batch);
    CHECK(sent == 8, "dispatch returned %zu against a limit of 8", sent);
    CHECK(batch.size() == 8, "the batch holds %zu requests", batch.size());
    CHECK(reg.view<TileBuilding>().size() == 8,
          "%zu tiles are marked building", size_t(reg.view<TileBuilding>().size()));
    CHECK(reg.view<TileWanted>().size() == wanted - 8,
          "dispatch left %zu wanted, expected %zu",
          size_t(reg.view<TileWanted>().size()), wanted - 8);

    // Nearest first: the tile under the camera is the one whose absence is a
    // hole in the floor.
    bool ordered = true;
    double previous = -1.0;
    for (const BuildRequest& r : batch) {
        const Aabb b = streamer.boundsOf(r.key);
        const double d = b.centreX() * b.centreX() + b.centreY() * b.centreY() +
                         b.centreZ() * b.centreZ();
        if (d + 1e-9 < previous) ordered = false;
        previous = d;
    }
    CHECK(ordered, "dispatch is not ordered nearest first");

    // A second dispatch must not hand out the same tiles again. This is the
    // property that keeps a tile from being built twice concurrently.
    std::vector<BuildRequest> again;
    dispatchSystem(reg, Vec3{0, 0, 0}, 8, again);
    bool overlap = false;
    for (const BuildRequest& a : batch)
        for (const BuildRequest& b : again)
            if (a.entity == b.entity) overlap = true;
    CHECK(!overlap, "a tile was dispatched twice");

    // And residency must not re-queue a tile that is already building.
    ResidencyResult r = residencySystem(reg, streamer);
    CHECK(r.relevelled == 0, "residency re-queued %zu tiles that are building",
          r.relevelled);
}

static void testEmptyTilesAreRemembered() {
    section("tiles that build to nothing");

    // The regression this whole design exists for.
    //
    // Most tiles in a resident set are empty: open sky above the ground, solid
    // rock below it with no exposed faces. When "built and empty" was not
    // recorded, such a tile looked unbuilt on the next update and was queued
    // again — every frame, forever. The workers spent all their time
    // regenerating air while the tiles the player could see waited behind them,
    // and the visible world drained away ahead of a walking camera.
    entt::registry reg;
    Streamer streamer = makeStreamer(80.0, 100000);
    streamer.update(Vec3{0, 0, 0}, nullptr);
    residencySystem(reg, streamer);

    std::vector<BuildRequest> batch;
    dispatchSystem(reg, Vec3{0, 0, 0}, -1, batch);
    CHECK(!batch.empty(), "nothing was dispatched");

    // Every one comes back empty, as the sky does.
    for (const BuildRequest& r : batch) {
        reg.remove<TileBuilding>(r.entity);
        reg.emplace_or_replace<TileLevel>(r.entity, r.lod);
        reg.emplace_or_replace<TileEmpty>(r.entity);
    }

    ResidencyResult after = residencySystem(reg, streamer);
    CHECK(after.relevelled == 0,
          "%zu tiles that were built and empty were queued again; the workers"
          " would rebuild empty sky forever", after.relevelled);
    CHECK(reg.view<TileWanted>().size() == 0,
          "%zu tiles are still wanted after all of them were built",
          size_t(reg.view<TileWanted>().size()));
}

static void testCullAndDraw() {
    section("culling and the draw list");

    entt::registry reg;
    // A ring of tiles around the origin, all with geometry.
    for (int i = 0; i < 24; ++i) {
        const double angle = i * 0.2617993878;   // 15 degrees
        const double x = std::cos(angle) * 80.0, z = std::sin(angle) * 80.0;
        const entt::entity e = reg.create();
        const TileKey key{int32_t(i), 0, 0};
        reg.emplace<Tile>(e, key, tileId(key));
        reg.emplace<TileBounds>(e, Aabb::fromOriginSize(x - 8, -8, z - 8, 16, 16, 16));
        reg.emplace<TileLevel>(e, 0);
        reg.emplace<TileGeometry>(e, uint32_t(100), uint32_t(4096));
    }

    Camera cam;
    cam.position = Vec3{0, 0, 0};
    cam.yaw = 0.0;      // looking down -Z
    cam.zNear = 0.5;
    cam.zFar = 400.0;
    const Frustum ahead(cam.viewProjection(1.6));

    const size_t visibleAhead = cullSystem(reg, ahead);
    CHECK(visibleAhead > 0 && visibleAhead < 24,
          "the frustum kept %zu of 24 tiles on a ring; it is not culling",
          visibleAhead);
    CHECK(reg.view<TileVisible>().size() == visibleAhead,
          "the visible tag count disagrees with the returned count");

    std::vector<uint64_t> list;
    drawListSystem(reg, cam.position, list);
    CHECK(list.size() == visibleAhead,
          "the draw list holds %zu entries for %zu visible tiles",
          list.size(), visibleAhead);

    // Turning round must produce a different visible set, and the tag must not
    // be stale from the previous frame.
    cam.yaw = 3.14159265358979;
    const Frustum behind(cam.viewProjection(1.6));
    const size_t visibleBehind = cullSystem(reg, behind);
    CHECK(reg.view<TileVisible>().size() == visibleBehind,
          "visibility tags are stale after turning around");
    CHECK(visibleAhead + visibleBehind <= 24,
          "facing two opposite directions marked %zu of 24 tiles visible",
          visibleAhead + visibleBehind);

    // A tile with no geometry must never reach the draw list, however visible.
    reg.clear();
    const entt::entity empty = reg.create();
    const TileKey key{0, 0, 0};
    reg.emplace<Tile>(empty, key, tileId(key));
    reg.emplace<TileBounds>(empty, Aabb::fromOriginSize(-8, -8, -40, 16, 16, 16));
    reg.emplace<TileLevel>(empty, 0);
    reg.emplace<TileEmpty>(empty);
    cam.yaw = 0.0;
    cullSystem(reg, Frustum(cam.viewProjection(1.6)));
    drawListSystem(reg, cam.position, list);
    CHECK(list.empty(), "an empty tile produced a draw call");
}

int main() {
    std::printf("Elysium — ecs tests\n");
    testTileIds();
    testMotion();
    testActiveCamera();
    testResidency();
    testDispatch();
    testEmptyTilesAreRemembered();
    testCullAndDraw();
    return elytest::report("ecs tests");
}
