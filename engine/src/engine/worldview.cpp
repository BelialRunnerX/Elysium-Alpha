#include "worldview.h"

#include <algorithm>
#include <chrono>
#include <thread>

namespace ely {

WorldView::~WorldView() { shutdown(); }

void WorldView::init(const Terrain& terrain, GlRenderer& renderer,
                     const WorldViewConfig& cfg) {
    shutdown();
    terrain_ = &terrain;
    renderer_ = &renderer;
    cfg_ = cfg;
    streamer_ = Streamer(cfg.streamer);
    jobs_ = std::make_unique<JobSystem>(cfg.workers);
}

void WorldView::shutdown() {
    // Destroying the pool joins its threads, and its destructor drains whatever
    // is still queued rather than abandoning it. Only after that is it safe to
    // touch anything a job captured — which is why this is first.
    jobs_.reset();
    inFlight_.store(0);
    finished_.clear();
    if (renderer_) renderer_->clearChunks();
    registry_.clear();
    terrain_ = nullptr;
    renderer_ = nullptr;
}

TileSpec WorldView::specFor(const TileKey& key, int lod) const {
    const double t = cfg_.streamer.tileSize;
    TileSpec spec;
    spec.face = cfg_.face;
    spec.uCentre = cfg_.uCentre;
    spec.vCentre = cfg_.vCentre;
    spec.ox = key.x * t;
    spec.oy = key.y * t;
    spec.oz = key.z * t;
    spec.sizeX = spec.sizeY = spec.sizeZ = t;
    spec.lod = lod;
    // No scene clipping: the world genuinely continues past every tile, so the
    // apron should see real terrain everywhere. Clipping is only for the
    // offline tools, which render a finite slab and would otherwise let the
    // viewer look inside the ground at its edges.
    spec.clipToScene = false;
    return spec;
}

void WorldView::update() {
    const auto t0 = std::chrono::steady_clock::now();
    stats_.uploadsThisFrame = 0;
    if (!terrain_ || !renderer_) return;

    // 1. Residency. The streamer decides what and at what level; the system
    //    turns that into entities. Keeping the policy out of the ECS is what
    //    lets hysteresis and budgets be tested without a registry at all.
    const Frustum residencyFrustum(camera.viewProjection(1.0));
    streamer_.update(camera.position, &residencyFrustum);
    ecs::residencySystem(registry_, streamer_);

    // 2. Dispatch. The system moves each tile from wanted to building as it
    //    hands it over, so a tile cannot be queued twice even if this loop is
    //    interrupted.
    std::vector<ecs::BuildRequest> requests;
    ecs::dispatchSystem(registry_, camera.position, -1, requests);
    for (const ecs::BuildRequest& request : requests) {
        inFlight_.fetch_add(1);
        // The spec is captured by value, so the job depends on nothing that can
        // change under it. buildTile reads an immutable terrain and writes only
        // its own return value; that is the whole of the thread safety here.
        jobs_->dispatch([this, entity = request.entity, key = request.key,
                         lod = request.lod, spec = specFor(request.key, request.lod)] {
            BuiltTile built = buildTile(*terrain_, spec);
            {
                std::lock_guard<std::mutex> lock(mutex_);
                finished_.push_back(Result{entity, key, lod, std::move(built)});
            }
            inFlight_.fetch_sub(1);
        });
    }
    stats_.tilesQueued = size_t(inFlight_.load());

    // 3. Ingest. Uploads touch GL, so they happen here on the main thread and
    //    nowhere else, and they are budgeted separately from builds: a burst of
    //    finished tiles arriving together must not become one long frame.
    std::vector<Result> ready;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const size_t take =
            std::min(finished_.size(), size_t(cfg_.maxUploadsPerFrame));
        ready.insert(ready.end(),
                     std::make_move_iterator(finished_.begin()),
                     std::make_move_iterator(finished_.begin() + long(take)));
        finished_.erase(finished_.begin(), finished_.begin() + long(take));
    }

    for (Result& r : ready) {
        ++stats_.tilesBuiltTotal;
        // The entity can be gone: the camera may have moved past this tile
        // while it was being built, and residency destroys such tiles before
        // anything is uploaded for them. valid() is the whole check — there is
        // no second container to consult and no way for the two to disagree.
        if (!registry_.valid(r.entity)) continue;

        registry_.remove<ecs::TileBuilding>(r.entity);
        registry_.emplace_or_replace<ecs::TileLevel>(r.entity, r.lod);

        const uint64_t id = ecs::tileId(r.key);
        renderer_->uploadChunk(id, r.tile.gpu);

        if (r.tile.empty()) {
            // Built, and it was nothing. Recording that is what stops empty sky
            // being rebuilt every frame forever.
            registry_.remove<ecs::TileGeometry>(r.entity);
            registry_.emplace_or_replace<ecs::TileEmpty>(r.entity);
        } else {
            registry_.remove<ecs::TileEmpty>(r.entity);
            registry_.emplace_or_replace<ecs::TileGeometry>(
                r.entity, uint32_t(r.tile.gpu.triangles()),
                uint32_t(r.tile.gpu.vertexBytes() + r.tile.gpu.indexBytes()));
            ++stats_.uploadsThisFrame;
        }
    }

    stats_.tilesResident = streamer_.residentCount();
    stats_.tilesVisible = streamer_.visibleCount();
    stats_.bufferBytes = renderer_->stats().bufferBytes;
    stats_.updateMs =
        std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - t0).count();
}

void WorldView::settle(int maxIterations) {
    for (int i = 0; i < maxIterations; ++i) {
        update();
        bool idle = inFlight_.load() == 0;
        if (idle) {
            std::lock_guard<std::mutex> lock(mutex_);
            idle = finished_.empty();
        }
        if (idle && streamer_.toLoad().empty()) return;
        if (!idle) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void WorldView::render(int width, int height, const float clearColour[3]) {
    const auto t0 = std::chrono::steady_clock::now();
    if (!renderer_) return;

    const double aspect = double(width) / double(height ? height : 1);
    const Mat4 viewProj = camera.viewProjection(aspect);
    const Frustum frustum(viewProj);

    // Culled and ordered every frame. Caching the list would mean a stale one
    // for the first frame after any camera movement, which is every frame.
    ecs::cullSystem(registry_, frustum);
    ecs::drawListSystem(registry_, camera.position, drawList_);

    renderer_->render(viewProj, drawList_, width, height, clearColour);
    stats_.trianglesDrawn = renderer_->stats().trianglesDrawn;
    stats_.tilesVisible = drawList_.size();
    stats_.renderMs =
        std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - t0).count();
}

}  // namespace ely
