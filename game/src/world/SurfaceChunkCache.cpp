#include "world/SurfaceChunkCache.hpp"

#include <algorithm>
#include <chrono>
#include <limits>

namespace elysium {
namespace {

SurfaceCellAddress normalizeCell(SurfaceCellAddress a) {
    if (a.u >= 0 && a.u < PlanetSurface::FaceResolution &&
        a.v >= 0 && a.v < PlanetSurface::FaceResolution) return a;
    const auto wrapped=wrapFaceCell(a.face,a.u,a.v,PlanetSurface::FaceResolution);
    a.face=wrapped.face;
    a.u=wrapped.u;
    a.v=wrapped.v;
    return a;
}

} // namespace

int SurfaceChunkData::haloIndex(int u, int v, int radial) {
    const int hu=u+Halo, hv=v+Halo, hr=radial+Halo;
    if (hu<0 || hu>=HaloSize || hv<0 || hv>=HaloSize || hr<0 || hr>=HaloSize) return -1;
    return hr + HaloSize * (hu + HaloSize * hv);
}

BlockType SurfaceChunkData::getLocal(int u, int v, int radial) const {
    if (u < 0 || u >= CoreSize || v < 0 || v >= CoreSize || radial < 0 || radial >= CoreSize)
        return BlockType::Air;
    return getWithHalo(u,v,radial);
}

BlockType SurfaceChunkData::getWithHalo(int u, int v, int radial) const {
    const int idx=haloIndex(u,v,radial);
    if (idx<0 || static_cast<std::size_t>(idx)>=blocks.size()) return BlockType::Air;
    return blocks[static_cast<std::size_t>(idx)];
}

SurfaceCellAddress SurfaceChunkData::worldAddress(int localU, int localV, int localRadial) const {
    SurfaceCellAddress a{address.face,
        address.u*CoreSize + localU,
        address.v*CoreSize + localV,
        address.radial*CoreSize + localRadial};
    return normalizeCell(a);
}

int SurfaceChunkData::flatCellIndex(SurfaceCellAddress a) {
    a=normalizeCell(a);
    return a.radial + PlanetSurface::RadialLayers * (a.u + PlanetSurface::FaceResolution *
        (a.v + PlanetSurface::FaceResolution * static_cast<int>(a.face)));
}

bool SurfaceChunkData::isRefined(SurfaceCellAddress a) const {
    if (a.radial<0 || a.radial>=PlanetSurface::RadialLayers) return false;
    return microBricks.contains(flatCellIndex(a));
}

const MicroBrick* SurfaceChunkData::microBrick(SurfaceCellAddress a) const {
    if (a.radial<0 || a.radial>=PlanetSurface::RadialLayers) return nullptr;
    const auto it=microBricks.find(flatCellIndex(a));
    return it==microBricks.end()?nullptr:&it->second;
}

BlockType SurfaceChunkData::microGet(SurfaceCellAddress a, int mu, int mr, int mv) const {
    if (a.radial<0) return BlockType::Stone;
    if (a.radial>=PlanetSurface::RadialLayers) return BlockType::Air;
    a=normalizeCell(a);
    if (const auto* brick=microBrick(a)) return brick->get(mu,mr,mv);

    // Non-refined cells should only be queried from the core or one-cell halo.
    // Resolve them through local coordinates where possible.
    const int u0=address.u*CoreSize;
    const int v0=address.v*CoreSize;
    const int r0=address.radial*CoreSize;
    if (a.face==address.face) {
        const int lu=a.u-u0, lv=a.v-v0, lr=a.radial-r0;
        if (lu>=-Halo && lu<=CoreSize && lv>=-Halo && lv<=CoreSize && lr>=-Halo && lr<=CoreSize)
            return getWithHalo(lu,lv,lr);
    }
    // Across a cube-face seam the wrapped address no longer has a simple local
    // delta. Such non-refined lookup is only needed for immediate halo cells,
    // which were already sampled into getWithHalo() by the mesher's local path.
    return BlockType::Air;
}

std::size_t SurfaceChunkData::estimatedBytes() const {
    std::size_t bytes=blocks.capacity()*sizeof(BlockType);
    for (const auto& [_,brick] : microBricks) {
        bytes += sizeof(int) + sizeof(MicroBrick) + brick.overrideCount() * (sizeof(int)+sizeof(BlockType));
    }
    return bytes;
}

SurfaceChunkCache::SurfaceChunkCache(JobSystem& jobs, std::size_t maxResidentChunks,
                                     std::size_t maxResidentBytes, std::size_t maxPending)
    : jobs_(jobs),
      maxResidentChunks_(std::max<std::size_t>(1,maxResidentChunks)),
      maxResidentBytes_(std::max<std::size_t>(SurfaceChunkData::HaloSize*SurfaceChunkData::HaloSize*SurfaceChunkData::HaloSize,maxResidentBytes)),
      maxPending_(std::max<std::size_t>(1,maxPending)) {}

void SurfaceChunkCache::beginFrame() {
    ++clock_;
    for (auto& [_,entry] : entries_) {
        entry.wanted=false;
        entry.priority=SurfaceChunkPriority::Prefetch;
    }
}

std::size_t SurfaceChunkCache::pendingCount() const {
    std::size_t count=0;
    for (const auto& [_,entry] : entries_) if (entry.pending) ++count;
    return count;
}

std::shared_ptr<SurfaceChunkData> SurfaceChunkCache::buildChunk(const PlanetSurfaceSnapshot& snapshot,
                                                                const PlanetChunkAddress& address,
                                                                std::uint64_t revision) {
    auto out=std::make_shared<SurfaceChunkData>();
    out->address=address;
    out->revision=revision;
    out->referenceRadius=snapshot.referenceRadius;
    out->blocks.resize(static_cast<std::size_t>(SurfaceChunkData::HaloSize*SurfaceChunkData::HaloSize*SurfaceChunkData::HaloSize),BlockType::Air);

    const int u0=address.u*PlanetSurface::ChunkSize;
    const int v0=address.v*PlanetSurface::ChunkSize;
    const int r0=address.radial*PlanetSurface::ChunkSize;
    for (int lv=-SurfaceChunkData::Halo;lv<=PlanetSurface::ChunkSize;++lv) {
        for (int lu=-SurfaceChunkData::Halo;lu<=PlanetSurface::ChunkSize;++lu) {
            for (int lr=-SurfaceChunkData::Halo;lr<=PlanetSurface::ChunkSize;++lr) {
                SurfaceCellAddress world{address.face,u0+lu,v0+lv,r0+lr};
                if (world.radial>=0 && world.radial<PlanetSurface::RadialLayers) world=snapshot.normalize(world);
                const int hi=(lr+SurfaceChunkData::Halo) + SurfaceChunkData::HaloSize *
                    ((lu+SurfaceChunkData::Halo) + SurfaceChunkData::HaloSize*(lv+SurfaceChunkData::Halo));
                out->blocks[static_cast<std::size_t>(hi)]=snapshot.get(world);
                if (world.radial>=0 && world.radial<PlanetSurface::RadialLayers) {
                    if (const auto* brick=snapshot.microBrick(world))
                        out->microBricks.try_emplace(snapshot.flatIndex(world.face,world.u,world.v,world.radial),*brick);
                }
            }
        }
    }
    return out;
}

bool SurfaceChunkCache::request(const PlanetSurface& planet, const PlanetChunkAddress& address,
                                SurfaceChunkPriority priority) {
    const auto key=stableChunkKey(address);
    auto [it,inserted]=entries_.try_emplace(key);
    Entry& entry=it->second;
    if (inserted) entry.address=address;
    entry.wanted=true;
    entry.lastUse=clock_;
    if (static_cast<int>(priority)<static_cast<int>(entry.priority) || inserted) entry.priority=priority;

    const auto revision=planet.chunkRevision(address);
    if (entry.data && entry.revision==revision) return true;
    if (entry.pending && entry.revision==revision) return true;

    // maxPending_ is a hard owner-thread scheduling frontier. All request
    // classes must respect it; callers retry wanted Visible/EditRemesh work on
    // subsequent frames, while speculative Prefetch requests are simply dropped.
    // This prevents a camera turn or edit burst from flooding the shared worker
    // pool with one job per visible chunk.
    if (!entry.pending && pendingCount()>=maxPending_) {
        if (priority==SurfaceChunkPriority::Prefetch) ++cumulative_.droppedPrefetch;
        return false;
    }

    if (entry.pending) {
        entry.pending.reset();
        ++entry.token;
        ++cumulative_.cancellations;
    }
    entry.data.reset();
    entry.revision=revision;
    const auto token=++entry.token;
    auto snapshot=std::make_shared<PlanetSurfaceSnapshot>(planet.snapshot());
    ++cumulative_.generationRequests;
    entry.pending=jobs_.submit([snapshot,address,revision,token]() {
        BuildResult result{};
        result.address=address;
        result.revision=revision;
        result.token=token;
        result.data=buildChunk(*snapshot,address,revision);
        return result;
    });
    return true;
}

void SurfaceChunkCache::cancelUnwanted() {
    for (auto it=entries_.begin();it!=entries_.end();) {
        auto& entry=it->second;
        if (!entry.wanted && entry.pending) {
            entry.pending.reset();
            ++entry.token;
            ++cumulative_.cancellations;
        }
        if (!entry.wanted && !entry.pending && !entry.data) it=entries_.erase(it);
        else ++it;
    }
}

void SurfaceChunkCache::processCompleted(const PlanetSurface& planet) {
    using namespace std::chrono_literals;
    for (auto& [_,entry] : entries_) {
        if (!entry.pending || entry.pending->wait_for(0ms)!=std::future_status::ready) continue;
        auto result=entry.pending->get();
        entry.pending.reset();
        if (result.token!=entry.token || result.revision!=planet.chunkRevision(result.address)) {
            ++cumulative_.staleResults;
            continue;
        }
        entry.data=std::move(result.data);
        entry.revision=result.revision;
        entry.lastUse=clock_;
        ++cumulative_.published;
    }
}

void SurfaceChunkCache::evictToBudget() {
    auto totals=[this]() {
        std::pair<std::size_t,std::size_t> t{};
        for (const auto& [_,entry] : entries_) if (entry.data) {
            ++t.first;
            t.second+=entry.data->estimatedBytes();
        }
        return t;
    };

    for (;;) {
        auto [count,bytes]=totals();
        if (count<=maxResidentChunks_ && bytes<=maxResidentBytes_) break;
        auto victim=entries_.end();
        std::uint64_t oldest=std::numeric_limits<std::uint64_t>::max();
        for (auto it=entries_.begin();it!=entries_.end();++it) {
            const auto& e=it->second;
            if (!e.data || e.wanted || e.pending) continue;
            if (e.lastUse<oldest) { oldest=e.lastUse; victim=it; }
        }
        if (victim==entries_.end()) break; // all over-budget data is currently requested
        victim->second.data.reset();
        ++cumulative_.evictions;
        if (!victim->second.pending && !victim->second.wanted) entries_.erase(victim);
    }
}

void SurfaceChunkCache::sync(const PlanetSurface& planet) {
    processCompleted(planet);
    evictToBudget();
}

void SurfaceChunkCache::clear() {
    for (auto& [_,entry] : entries_) if (entry.pending) {
        entry.pending.reset();
        ++cumulative_.cancellations;
    }
    entries_.clear();
}

std::shared_ptr<const SurfaceChunkData> SurfaceChunkCache::find(const PlanetChunkAddress& address) const {
    const auto it=entries_.find(stableChunkKey(address));
    if (it==entries_.end()) return {};
    return it->second.data;
}

SurfaceChunkCacheStats SurfaceChunkCache::stats() const {
    auto s=cumulative_;
    for (const auto& [_,entry] : entries_) {
        if (entry.data) {
            ++s.resident;
            s.residentBytes+=entry.data->estimatedBytes();
        }
        if (entry.pending) ++s.pending;
        if (entry.wanted) ++s.wanted;
    }
    return s;
}

} // namespace elysium
