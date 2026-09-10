// What is resident, at what resolution, as the camera moves.
//
// The LOD rule itself is trivial — one level coarser per doubling of distance
// (see world/lod.h). What is not trivial is doing that to a *moving* camera
// without the world tearing itself apart, and this file exists for exactly two
// problems that only show up in motion:
//
// 1. THRASHING. A tile sitting exactly on a level boundary flips between two
//    levels every time the camera breathes. Each flip is a full regenerate,
//    remesh and re-upload of that tile. A player standing still at the wrong
//    distance would hold a core at 100% forever. The fix is hysteresis: a tile
//    must be pulled meaningfully *closer* than the boundary to refine, and
//    pushed meaningfully further to coarsen, so the two thresholds are
//    different and the region between them is stable.
//
// 2. UNBOUNDED WORK. A camera that teleports, or simply turns a corner at
//    speed, can invalidate everything resident at once. Submitting all of it in
//    one frame is a multi-second freeze. The scheduler therefore hands out a
//    bounded amount of work per update, nearest first, and lets the rest wait —
//    a slightly stale distant tile is invisible; a stalled frame is not.
//
// The scheduler owns no geometry and does no generation. It answers "which
// tiles, at which level" and nothing else, which is what makes it testable
// without a GPU, a thread pool or a planet.
#pragma once
#include "../world/lod.h"
#include "frustum.h"
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ely {

// A tile is a cube of world, identified by its integer position on a grid whose
// spacing is the tile's edge in metres. Tiles are the unit of generation,
// meshing, upload and culling — one tile is one draw call.
struct TileKey {
    int32_t x = 0, y = 0, z = 0;   // tile coordinates, not metres
    bool operator==(const TileKey& o) const {
        return x == o.x && y == o.y && z == o.z;
    }
};

struct TileKeyHash {
    size_t operator()(const TileKey& k) const {
        // Three-way mix; the multipliers are odd and large so that the common
        // case of tiles differing in one axis does not collide in runs.
        uint64_t h = uint64_t(uint32_t(k.x)) * 0x9E3779B97F4A7C15ull;
        h ^= uint64_t(uint32_t(k.y)) * 0xC2B2AE3D27D4EB4Full;
        h ^= uint64_t(uint32_t(k.z)) * 0x165667B19E3779F9ull;
        h ^= h >> 29;
        return size_t(h);
    }
};

struct TileState {
    int lod = 0;          // the level this tile is currently built at
    bool visible = true;  // survived the frustum test on the last update
};

struct StreamerConfig {
    double tileSize = 32.0;        // metres per tile edge
    double viewDistance = 512.0;   // beyond this, nothing is resident
    double nearDistance = 40.0;    // inside this, the finest level in use
    int finestLod = 0;             // micro-voxels are opt-in, not the default
    int coarsestLod = 5;

    // Hysteresis, as a fraction of the distance to a level boundary. 0.15 means
    // a tile refines only once it is 15% nearer than the boundary and coarsens
    // only once it is 15% further, so a camera hovering on the boundary sees no
    // rebuilds at all.
    //
    // Zero disables it, which is useful in tests to check the underlying rule
    // and useless in a game.
    double hysteresis = 0.15;

    // Ceiling on tiles handed to the builders per update. Distant work waits;
    // near work never does.
    //
    // This bounds *queueing*, which is cheap, not building, which is not — the
    // builders are on their own threads and the uploads have their own budget.
    // Setting it too low is not conservative, it is a bug: with a 340 m view
    // and 32 m tiles the resident set is around two thousand tiles, and at six
    // per frame a walking camera outruns its own terrain and the visible world
    // drains away ahead of it. That is exactly what happened. The number should
    // be large enough that the queue, not the budget, is the limit.
    int maxLoadsPerUpdate = 64;

    // Tiles kept above and below the camera. Terrain is a shell: a full cube of
    // tiles spends most of its budget on solid rock and empty sky, and the
    // vertical reach is what decides how much of that budget is wasted.
    int verticalTiles = 2;
};

class Streamer {
public:
    explicit Streamer(const StreamerConfig& cfg = StreamerConfig{}) : cfg_(cfg) {}

    const StreamerConfig& config() const { return cfg_; }

    // Recompute residency for a camera position and view.
    //
    // Pass a null frustum to skip visibility marking — residency is decided by
    // distance alone, deliberately. Tiles behind the camera stay loaded because
    // turning around must not cost a reload; the frustum decides what is
    // *drawn*, not what is *kept*.
    void update(const Vec3& eye, const Frustum* frustum) {
        loads_.clear();
        unloads_.clear();

        // The residency radius needs hysteresis for the same reason the level
        // boundaries do, and it is easy to miss because it is a different
        // boundary. A tile sitting at exactly the view distance loads, unloads
        // and reloads as the camera breathes — a full generate and mesh each
        // time, for terrain at the very edge of visibility. A tile is admitted
        // at viewDistance and only evicted once it is past viewDistance plus
        // the margin, so the two are never the same number.
        const double keepDistance = cfg_.viewDistance * (1.0 + cfg_.hysteresis);
        const int reach = int(std::ceil(keepDistance / cfg_.tileSize));
        const int cx = tileOf(eye.x), cy = tileOf(eye.y), cz = tileOf(eye.z);

        const int reachY = cfg_.verticalTiles;

        struct Candidate { TileKey key; int lod; double dist; };
        std::vector<Candidate> wanted;
        wanted.reserve(size_t(reach * 2 + 1) * (reach * 2 + 1) * (reachY * 2 + 1));

        for (int ty = cy - reachY; ty <= cy + reachY; ++ty)
            for (int tz = cz - reach; tz <= cz + reach; ++tz)
                for (int tx = cx - reach; tx <= cx + reach; ++tx) {
                    const TileKey key{tx, ty, tz};
                    const double d = distanceTo(key, eye);
                    const bool already = resident_.find(key) != resident_.end();
                    if (d > (already ? keepDistance : cfg_.viewDistance)) continue;
                    wanted.push_back({key, lodFor(key, d), d});
                }

        // Nearest first. Everything downstream — the load budget, the order
        // work reaches the thread pool — depends on this, because the tile
        // under the player is the one whose absence is a hole in the floor.
        std::sort(wanted.begin(), wanted.end(),
                  [](const Candidate& a, const Candidate& b) {
                      return a.dist < b.dist;
                  });

        // Anything resident that is no longer wanted gets dropped. Unloads are
        // not budgeted: freeing memory is cheap and holding it is what runs a
        // machine out.
        std::unordered_map<TileKey, TileState, TileKeyHash> next;
        next.reserve(wanted.size());

        int budget = cfg_.maxLoadsPerUpdate;
        for (const Candidate& c : wanted) {
            auto it = resident_.find(c.key);
            if (it == resident_.end()) {
                if (budget <= 0) continue;      // not this frame; try again next
                --budget;
                loads_.push_back({c.key, c.lod});
                next[c.key] = TileState{c.lod, true};
            } else if (it->second.lod != c.lod) {
                if (budget <= 0) {
                    next[c.key] = it->second;   // keep the old level meanwhile
                    continue;
                }
                --budget;
                loads_.push_back({c.key, c.lod});
                next[c.key] = TileState{c.lod, true};
            } else {
                next[c.key] = it->second;
            }
        }

        for (const auto& kv : resident_)
            if (next.find(kv.first) == next.end()) unloads_.push_back(kv.first);

        resident_.swap(next);

        // Visibility is a per-frame mark on resident tiles, not a residency
        // decision. Kept separate so that a tile leaving the frustum costs
        // nothing and re-entering it costs nothing.
        visibleCount_ = 0;
        for (auto& kv : resident_) {
            kv.second.visible = frustum ? frustum->visible(boundsOf(kv.first)) : true;
            if (kv.second.visible) ++visibleCount_;
        }
    }

    struct Load { TileKey key; int lod; };
    const std::vector<Load>& toLoad() const { return loads_; }
    const std::vector<TileKey>& toUnload() const { return unloads_; }

    size_t residentCount() const { return resident_.size(); }
    size_t visibleCount() const { return visibleCount_; }
    bool isResident(const TileKey& k) const {
        return resident_.find(k) != resident_.end();
    }
    int lodOf(const TileKey& k) const {
        auto it = resident_.find(k);
        return it == resident_.end() ? -1000 : it->second.lod;
    }
    bool isVisible(const TileKey& k) const {
        auto it = resident_.find(k);
        return it != resident_.end() && it->second.visible;
    }

    // Where a tile sits in the world, for culling and for the draw's push
    // constant.
    Aabb boundsOf(const TileKey& k) const {
        return Aabb::fromOriginSize(k.x * cfg_.tileSize, k.y * cfg_.tileSize,
                                    k.z * cfg_.tileSize,
                                    cfg_.tileSize, cfg_.tileSize, cfg_.tileSize);
    }

    // The level this tile should be at, given its distance and what it is
    // already at. Hysteresis makes this depend on the current state, which is
    // why it is a method rather than a free function.
    int lodFor(const TileKey& k, double distance) const {
        const int ideal = lodForDistance(distance, cfg_.nearDistance,
                                         cfg_.finestLod, cfg_.coarsestLod);
        auto it = resident_.find(k);
        if (it == resident_.end() || cfg_.hysteresis <= 0.0) return ideal;

        const int current = it->second.lod;
        if (ideal == current) return current;

        // Moving to a finer level requires being decisively nearer than the
        // boundary; coarser requires being decisively further. Between the two
        // thresholds the tile keeps whatever it has.
        if (ideal < current) {
            const double boundary = boundaryFor(current);
            return distance < boundary * (1.0 - cfg_.hysteresis) ? ideal : current;
        }
        const double boundary = boundaryFor(current + 1);
        return distance > boundary * (1.0 + cfg_.hysteresis) ? ideal : current;
    }

private:
    int tileOf(double v) const {
        return int(std::floor(v / cfg_.tileSize));
    }

    // Distance from the eye to the nearest point of the tile, not to its
    // centre. Using the centre makes a large tile the player is standing
    // inside test as if it were half a tile away, and at coarse levels that is
    // enough to pick the wrong LOD for the ground underfoot.
    double distanceTo(const TileKey& k, const Vec3& eye) const {
        const Aabb b = boundsOf(k);
        const double dx = std::max({double(b.minX) - eye.x, 0.0, eye.x - double(b.maxX)});
        const double dy = std::max({double(b.minY) - eye.y, 0.0, eye.y - double(b.maxY)});
        const double dz = std::max({double(b.minZ) - eye.z, 0.0, eye.z - double(b.maxZ)});
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    // The distance at which `lod` begins. Inverse of lodForDistance.
    double boundaryFor(int lod) const {
        const int steps = lod - cfg_.finestLod;
        return steps <= 0 ? 0.0 : cfg_.nearDistance * std::exp2(double(steps));
    }

    StreamerConfig cfg_;
    std::unordered_map<TileKey, TileState, TileKeyHash> resident_;
    std::vector<Load> loads_;
    std::vector<TileKey> unloads_;
    size_t visibleCount_ = 0;
};

}  // namespace ely
