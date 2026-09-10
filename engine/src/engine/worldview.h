// The engine loop: a camera moving through a planet.
//
// This is where the streamer, the tile builder and the renderer meet, and it is
// deliberately a class rather than a main() so that the whole thing can be
// driven by a scripted camera and rendered headlessly. The windowed
// application and the flythrough tool run *the same code*; only who moves the
// camera differs. That is what makes "does streaming work while moving" a
// question with a measured answer rather than an impression.
//
// Tiles are built on worker threads. This is not premature: generation is the
// dominant cost in the engine, and several of those on the main thread is a
// visible hitch every time the player walks into new ground. Because buildTile
// is a pure function of (terrain, spec), the threading needs no locking around
// the world at all: work goes out, meshes come back, and the only shared state
// is the two queues.
//
// Tile state lives in an EnTT registry rather than in maps here. The reason is
// a bug this class actually had: residency, build state and GPU geometry were
// three separate containers that had to agree, and when they stopped agreeing —
// a tile that meshed to nothing was never recorded as built — the workers spent
// every frame regenerating empty sky while the visible world drained away ahead
// of the camera. As entities with tags there is one population and the
// questions are views, so that class of disagreement cannot be expressed.
#pragma once
#include "../ecs/systems.h"
#include "../jobs/jobsystem.h"
#include "../world/terrain.h"
#include "../gl/glrenderer.h"
#include "../gfx/streamer.h"
#include "../gfx/tilebuild.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

namespace ely {

struct WorldViewConfig {
    StreamerConfig streamer;
    Face face = FACE_PZ;
    double uCentre = 0.0, vCentre = 0.0;

    // Worker threads for tile building. Zero means build on the calling thread,
    // which is what the tests use when they want deterministic timing. Negative
    // means one per core, less the main thread.
    int workers = -1;

    // Uploads are a main-thread cost (they touch GL) and are budgeted
    // separately from builds, because a burst of finished tiles arriving
    // together must not turn into one long frame.
    int maxUploadsPerFrame = 6;
};

struct WorldViewStats {
    size_t tilesResident = 0;
    size_t tilesVisible = 0;
    size_t tilesQueued = 0;
    size_t tilesBuiltTotal = 0;
    size_t uploadsThisFrame = 0;
    uint64_t trianglesDrawn = 0;
    uint64_t bufferBytes = 0;
    double updateMs = 0;
    double renderMs = 0;
};

class WorldView {
public:
    WorldView() = default;
    ~WorldView();

    WorldView(const WorldView&) = delete;
    WorldView& operator=(const WorldView&) = delete;

    // `terrain` and `renderer` must outlive this object. The terrain is read
    // from many threads and is never written, which is the property that makes
    // the whole design safe without a lock.
    void init(const Terrain& terrain, GlRenderer& renderer,
              const WorldViewConfig& cfg);
    void shutdown();

    // One step of the streaming loop: decide residency for the camera, queue
    // what needs building, collect what has finished, upload up to the budget,
    // drop what left range.
    void update();

    // Draw everything resident that survives the frustum, nearest first.
    void render(int width, int height, const float clearColour[3]);

    // Block until every queued tile has been built and uploaded. For tools and
    // tests that want a complete picture rather than a streaming one; a game
    // never calls this.
    void settle(int maxIterations = 4096);

    Camera camera;
    const WorldViewStats& stats() const { return stats_; }

    // The world-space bounds of a tile, in the local metre frame.
    Aabb boundsOf(const TileKey& k) const { return streamer_.boundsOf(k); }

    // The pool, so a caller with its own parallel work can share it rather than
    // starting threads of its own.
    JobSystem* jobs() { return jobs_.get(); }

    static uint64_t tileId(const TileKey& k) { return ecs::tileId(k); }

    // The registry, for systems and tools that want to ask questions of the
    // world's entities directly. Exposed rather than hidden: an ECS whose
    // registry is private is a map with extra steps.
    entt::registry& registry() { return registry_; }
    const entt::registry& registry() const { return registry_; }

private:
    struct Result { entt::entity entity; TileKey key; int lod; BuiltTile tile; };

    TileSpec specFor(const TileKey& key, int lod) const;

    const Terrain* terrain_ = nullptr;
    GlRenderer* renderer_ = nullptr;
    WorldViewConfig cfg_;
    Streamer streamer_;

    // One pool for everything, rather than threads owned here.
    //
    // This used to be a bespoke worker pool: threads, a deque, a condition
    // variable and a hand-rolled shutdown, all private to this class and usable
    // by nothing else. Anything else that wanted parallelism would have started
    // its own threads and oversubscribed the machine. The pool now lives in
    // ely_jobs, and tile building is one of its clients.
    std::unique_ptr<JobSystem> jobs_;

    // Results come back here. The only shared state between the workers and the
    // main thread, because buildTile reads an immutable terrain and writes
    // nothing but its own return value.
    std::mutex mutex_;
    std::vector<Result> finished_;
    std::atomic<int> inFlight_{0};

    entt::registry registry_;
    std::vector<uint64_t> drawList_;
    WorldViewStats stats_;
};

}  // namespace ely
