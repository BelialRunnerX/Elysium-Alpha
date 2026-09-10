#include "world/PlanetSurface.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <limits>
#include <unordered_set>

namespace elysium {
namespace {

constexpr float kTau = 6.28318530717958647692f;

int faceIndex(CubeFace face) {
    return static_cast<int>(face);
}

int surfaceColumnIndex(CubeFace face, int u, int v) {
    return u + PlanetSurface::FaceResolution * (v + PlanetSurface::FaceResolution * faceIndex(face));
}

float seededUnit(std::uint64_t seed, std::uint64_t label, int channel) {
    return hash01(seed, channel, static_cast<int>(label & 0x7fffffffULL),
                  static_cast<int>((label >> 31U) & 0x7fffffffULL), 0x53504852ULL);
}

Vec3 seededAxis(std::uint64_t seed, std::uint64_t label, int channel) {
    Vec3 a{
        seededUnit(seed,label,channel*3+0)*2.0f-1.0f,
        seededUnit(seed,label,channel*3+1)*2.0f-1.0f,
        seededUnit(seed,label,channel*3+2)*2.0f-1.0f
    };
    if (lengthSq(a) < 0.05f) a = {0.31f,0.71f,0.43f};
    return normalize(a);
}

// Smooth deterministic field evaluated in shared 3D planetary space. No face
// index participates, so cube ownership changes cannot create a height seam.
float sphericalField(std::uint64_t seed, Vec3 d, std::uint64_t label) {
    float sum = 0.0f;
    float weight = 0.0f;
    constexpr std::array<float,4> frequency{2.4f, 5.1f, 10.7f, 19.3f};
    constexpr std::array<float,4> amplitude{1.0f, 0.52f, 0.25f, 0.12f};
    for (int i=0;i<4;++i) {
        const Vec3 axis = seededAxis(seed,label,i);
        const float phase = seededUnit(seed,label,30+i) * kTau;
        sum += std::sin(dot(d,axis) * frequency[static_cast<std::size_t>(i)] * kTau + phase)
             * amplitude[static_cast<std::size_t>(i)];
        weight += amplitude[static_cast<std::size_t>(i)];
    }
    return weight > 0.0f ? sum / weight : 0.0f;
}


int generatedSurfaceRadialFor(std::uint64_t seed, PlanetClass planetClass, CubeFace face, int u, int v) {
    const Vec3 d = faceGridCellDirection(face,u,v,PlanetSurface::FaceResolution);
    const float continents = sphericalField(seed,d,0x4C41594FULL);
    const float ridges = std::abs(sphericalField(seed,d,0x52494447ULL));
    float height = 0.0f;
    switch (planetClass) {
        case PlanetClass::Temperate: height = continents * 3.8f + ridges * 1.4f; break;
        case PlanetClass::Barren: height = continents * 2.6f + ridges * 2.1f - 0.8f; break;
        case PlanetClass::Scorched: height = continents * 3.2f + ridges * 2.8f - 0.3f; break;
        case PlanetClass::Frozen: height = continents * 3.4f + ridges * 2.0f + 0.2f; break;
        case PlanetClass::Toxic: height = continents * 3.1f + ridges * 1.7f - 0.2f; break;
        case PlanetClass::Irradiated: height = continents * 2.9f + ridges * 2.5f - 0.4f; break;
        case PlanetClass::Oceanic: height = continents * 4.2f + ridges * 1.1f - 2.0f; break;
        case PlanetClass::Anomalous: height = continents * 4.4f + ridges * 3.1f; break;
    }
    return std::clamp(PlanetSurface::ReferenceRadial + static_cast<int>(std::lround(height)), 6, PlanetSurface::RadialLayers-4);
}

BlockType generatedBlockFor(std::uint64_t seed, PlanetClass planetClass, CubeFace face,
                            int u, int v, int radial, int surface) {
    if (radial > surface) return BlockType::Air;
    if (radial <= 1 && planetClass == PlanetClass::Scorched) return BlockType::Magma;

    const int depth = surface-radial;
    if (depth == 0) {
        if (planetClass == PlanetClass::Temperate) return BlockType::Grass;
        if (planetClass == PlanetClass::Barren) return BlockType::Regolith;
        return BlockType::Basalt;
    }
    if (depth <= 2 && planetClass == PlanetClass::Temperate) return BlockType::Dirt;
    if (depth <= 2 && planetClass == PlanetClass::Barren) return BlockType::Regolith;
    if (depth <= 3 && planetClass == PlanetClass::Scorched) return BlockType::Basalt;

    const std::uint64_t label = 0x4F524553ULL ^ static_cast<std::uint64_t>(faceIndex(face));
    const float ore = hash01(seed,u,radial,v,label);
    if (radial < PlanetSurface::ReferenceRadial-5 && ore > 0.986f) return BlockType::IronOre;
    if (radial < PlanetSurface::ReferenceRadial-2 && ore > 0.974f) return BlockType::CopperOre;
    if (radial < PlanetSurface::ReferenceRadial && ore > 0.966f) return BlockType::CoalOre;
    return planetClass == PlanetClass::Scorched ? BlockType::Basalt : BlockType::Stone;
}

int clampCell(float uv, int resolution) {
    const float scaled = (uv + 1.0f) * 0.5f * static_cast<float>(resolution);
    return std::clamp(static_cast<int>(std::floor(scaled)), 0, resolution-1);
}

float uvCellCoord(float uv, int resolution) {
    return (uv + 1.0f) * 0.5f * static_cast<float>(resolution);
}

SurfaceCellAddress normalizeAddress(SurfaceCellAddress address) {
    if (address.u >= 0 && address.u < PlanetSurface::FaceResolution &&
        address.v >= 0 && address.v < PlanetSurface::FaceResolution) return address;

    const float u = ((static_cast<float>(address.u) + 0.5f) /
                     static_cast<float>(PlanetSurface::FaceResolution)) * 2.0f - 1.0f;
    const float v = ((static_cast<float>(address.v) + 0.5f) /
                     static_cast<float>(PlanetSurface::FaceResolution)) * 2.0f - 1.0f;
    const Vec3 direction = faceUvToDirectionUnclamped(address.face,u,v);
    const FaceUv mapped = directionToFaceUv(direction);
    address.face = mapped.face;
    address.u = clampCell(mapped.u, PlanetSurface::FaceResolution);
    address.v = clampCell(mapped.v, PlanetSurface::FaceResolution);
    return address;
}

Vec3 tangentReference(Vec3 up) {
    const Vec3 north{0,1,0};
    Vec3 forward = north - up * dot(north,up);
    if (lengthSq(forward) < 1e-5f) {
        const Vec3 east{0,0,1};
        forward = east - up * dot(east,up);
    }
    return normalize(forward);
}

Vec3 microBoundary(float referenceRadius, SurfaceCellAddress address,
                   int uEdge, int radialEdge, int vEdge) {
    address = normalizeAddress(address);
    const float fu = static_cast<float>(address.u) +
                     static_cast<float>(std::clamp(uEdge,0,MicroBrick::Resolution)) /
                     static_cast<float>(MicroBrick::Resolution);
    const float fv = static_cast<float>(address.v) +
                     static_cast<float>(std::clamp(vEdge,0,MicroBrick::Resolution)) /
                     static_cast<float>(MicroBrick::Resolution);
    const float u = fu / static_cast<float>(PlanetSurface::FaceResolution) * 2.0f - 1.0f;
    const float v = fv / static_cast<float>(PlanetSurface::FaceResolution) * 2.0f - 1.0f;
    const Vec3 d = faceUvToDirection(address.face,u,v);
    const float rr = static_cast<float>(address.radial) +
                     static_cast<float>(std::clamp(radialEdge,0,MicroBrick::Resolution)) /
                     static_cast<float>(MicroBrick::Resolution);
    const float radius = referenceRadius + rr - static_cast<float>(PlanetSurface::ReferenceRadial);
    return d * radius;
}

SurfaceMicroAddress locateMicroCommon(float referenceRadius, Vec3 p) {
    const float radius = length(p);
    const FaceUv uv = directionToFaceUv(p);
    const float uc = uvCellCoord(uv.u,PlanetSurface::FaceResolution);
    const float vc = uvCellCoord(uv.v,PlanetSurface::FaceResolution);
    const int u = std::clamp(static_cast<int>(std::floor(uc)),0,PlanetSurface::FaceResolution-1);
    const int v = std::clamp(static_cast<int>(std::floor(vc)),0,PlanetSurface::FaceResolution-1);
    const float radialCoord = radius - referenceRadius + static_cast<float>(PlanetSurface::ReferenceRadial);
    const int radial = static_cast<int>(std::floor(radialCoord));
    const float uf = std::clamp(uc-static_cast<float>(u),0.0f,0.999999f);
    const float vf = std::clamp(vc-static_cast<float>(v),0.0f,0.999999f);
    const float rf = std::clamp(radialCoord-static_cast<float>(radial),0.0f,0.999999f);
    return {{uv.face,u,v,radial},
            std::clamp(static_cast<int>(std::floor(uf*MicroBrick::Resolution)),0,MicroBrick::Resolution-1),
            std::clamp(static_cast<int>(std::floor(rf*MicroBrick::Resolution)),0,MicroBrick::Resolution-1),
            std::clamp(static_cast<int>(std::floor(vf*MicroBrick::Resolution)),0,MicroBrick::Resolution-1)};
}

} // namespace

std::size_t SurfaceChunkJournal::microOverrideCount() const {
    std::size_t total=0;
    for (const auto& [_,brick] : microBricks) total += brick.overrideCount();
    return total;
}

int PlanetSurfaceSnapshot::flatIndex(CubeFace face, int u, int v, int radial) const {
    return radial + RadialLayers * (u + FaceResolution * (v + FaceResolution * faceIndex(face)));
}

bool PlanetSurfaceSnapshot::radialInBounds(int radial) const {
    return radial >= 0 && radial < RadialLayers;
}

SurfaceCellAddress PlanetSurfaceSnapshot::normalize(SurfaceCellAddress address) const {
    return normalizeAddress(address);
}

BlockType PlanetSurfaceSnapshot::get(CubeFace face, int u, int v, int radial) const {
    if (radial < 0) return BlockType::Stone; // mantle-side continuation is solid
    if (radial >= RadialLayers) return BlockType::Air;
    const auto a = normalize({face,u,v,radial});
    const int idx=flatIndex(a.face,a.u,a.v,a.radial);
    if (const auto it=edits.find(idx); it!=edits.end()) return it->second;
    return baseline(a);
}

BlockType PlanetSurfaceSnapshot::get(SurfaceCellAddress address) const {
    return get(address.face,address.u,address.v,address.radial);
}

BlockType PlanetSurfaceSnapshot::baseline(SurfaceCellAddress address) const {
    if (address.radial < 0) return BlockType::Stone;
    if (address.radial >= RadialLayers) return BlockType::Air;
    address=normalize(address);
    const int surface=static_cast<int>(generatedSurfaceRadials[static_cast<std::size_t>(surfaceColumnIndex(address.face,address.u,address.v))]);
    return generatedBlockFor(seed,planetClass,address.face,address.u,address.v,address.radial,surface);
}

bool PlanetSurfaceSnapshot::isRefined(SurfaceCellAddress address) const {
    if (!radialInBounds(address.radial)) return false;
    address=normalize(address);
    return microBricks.contains(flatIndex(address.face,address.u,address.v,address.radial));
}

const MicroBrick* PlanetSurfaceSnapshot::microBrick(SurfaceCellAddress address) const {
    if (!radialInBounds(address.radial)) return nullptr;
    address=normalize(address);
    const auto it=microBricks.find(flatIndex(address.face,address.u,address.v,address.radial));
    return it==microBricks.end()?nullptr:&it->second;
}

BlockType PlanetSurfaceSnapshot::microGet(SurfaceCellAddress address, int mu, int mr, int mv) const {
    if (!radialInBounds(address.radial)) return address.radial<0?BlockType::Stone:BlockType::Air;
    address=normalize(address);
    const auto* brick=microBrick(address);
    if (!brick) return get(address);
    return brick->get(mu,mr,mv);
}

BlockType PlanetSurfaceSnapshot::microGet(const SurfaceMicroAddress& address) const {
    return microGet(address.cell,address.u,address.radial,address.v);
}

int PlanetSurfaceSnapshot::surfaceRadial(CubeFace face, int u, int v) const {
    const auto a = normalize({face,u,v,0});
    for (int r=RadialLayers-1;r>=0;--r) {
        if (blockProperties(get(a.face,a.u,a.v,r)).solid) return r;
    }
    return -1;
}

EditInfluenceSummary PlanetSurfaceSnapshot::editInfluence(const PlanetChunkAddress& address) const {
    const auto it=editInfluenceSummaries.find(stableChunkKey(address));
    return it==editInfluenceSummaries.end()?EditInfluenceSummary{}:it->second;
}

bool PlanetSurfaceSnapshot::hasEditInfluence(CubeFace face, int uBegin, int vBegin, int uEnd, int vEnd) const {
    uBegin=std::clamp(uBegin,0,FaceResolution);
    vBegin=std::clamp(vBegin,0,FaceResolution);
    uEnd=std::clamp(uEnd,0,FaceResolution);
    vEnd=std::clamp(vEnd,0,FaceResolution);
    if(uBegin>=uEnd || vBegin>=vEnd) return false;

    // v0.12: field LOD first consults compact per-chunk summaries rather than
    // scanning every sparse edit for each coarse tile. The exact edit/micro maps
    // remain present for authoritative reconstruction and compatibility.
    const int cu0=uBegin/PlanetSurface::ChunkSize;
    const int cv0=vBegin/PlanetSurface::ChunkSize;
    const int cu1=(uEnd-1)/PlanetSurface::ChunkSize;
    const int cv1=(vEnd-1)/PlanetSurface::ChunkSize;
    for(int cv=cv0;cv<=cv1;++cv) for(int cu=cu0;cu<=cu1;++cu) {
        for(int cr=0;cr<PlanetSurface::RadialChunks;++cr) {
            const PlanetChunkAddress chunk{face,cu,cv,cr};
            const auto summary=editInfluence(chunk);
            if(summary.intersects(uBegin,vBegin,uEnd,vEnd)) return true;
        }
    }

    // Old/hand-constructed snapshots may not carry summaries. Preserve exact
    // behavior as a deterministic fallback.
    if(!editInfluenceSummaries.empty()) return false;
    const int targetFace=static_cast<int>(face);
    constexpr int cellsPerFace=FaceResolution*FaceResolution*RadialLayers;
    auto inside=[&](int flat) {
        if(flat<0 || flat>=TotalCells) return false;
        const int f=flat/cellsPerFace;
        if(f!=targetFace) return false;
        int rem=flat-f*cellsPerFace;
        const int v=rem/(FaceResolution*RadialLayers);
        rem-=v*FaceResolution*RadialLayers;
        const int u=rem/RadialLayers;
        return u>=uBegin && u<uEnd && v>=vBegin && v<vEnd;
    };
    for(const auto& [flat,_]:edits) if(inside(flat)) return true;
    for(const auto& [flat,brick]:microBricks) if(brick.overrideCount()>0 && inside(flat)) return true;
    return false;
}

Vec3 PlanetSurfaceSnapshot::boundaryPosition(CubeFace face, int uEdge, int vEdge, int radialBoundary) const {
    const Vec3 d = faceGridCornerDirection(face,uEdge,vEdge,FaceResolution);
    const float radius = referenceRadius + static_cast<float>(radialBoundary - ReferenceRadial);
    return d * radius;
}

Vec3 PlanetSurfaceSnapshot::cellCenterPosition(SurfaceCellAddress address) const {
    address = normalize(address);
    const Vec3 d = faceGridCellDirection(address.face,address.u,address.v,FaceResolution);
    const float radius = referenceRadius + (static_cast<float>(address.radial) + 0.5f - static_cast<float>(ReferenceRadial));
    return d * radius;
}

Vec3 PlanetSurfaceSnapshot::microBoundaryPosition(SurfaceCellAddress address, int uEdge, int radialEdge, int vEdge) const {
    return microBoundary(referenceRadius,address,uEdge,radialEdge,vEdge);
}

Vec3 PlanetSurfaceSnapshot::microCellCenterPosition(const SurfaceMicroAddress& address) const {
    return microBoundaryPosition(address.cell,address.u,address.radial,address.v) * 0.125f +
           microBoundaryPosition(address.cell,address.u+1,address.radial,address.v) * 0.125f +
           microBoundaryPosition(address.cell,address.u,address.radial+1,address.v) * 0.125f +
           microBoundaryPosition(address.cell,address.u,address.radial,address.v+1) * 0.125f +
           microBoundaryPosition(address.cell,address.u+1,address.radial+1,address.v) * 0.125f +
           microBoundaryPosition(address.cell,address.u+1,address.radial,address.v+1) * 0.125f +
           microBoundaryPosition(address.cell,address.u,address.radial+1,address.v+1) * 0.125f +
           microBoundaryPosition(address.cell,address.u+1,address.radial+1,address.v+1) * 0.125f;
}

SurfaceMicroAddress PlanetSurfaceSnapshot::locateMicro(Vec3 p) const {
    return locateMicroCommon(referenceRadius,p);
}

bool PlanetSurfaceSnapshot::solidAt(Vec3 p) const {
    const auto m=locateMicro(p);
    if (!radialInBounds(m.cell.radial)) return m.cell.radial<0;
    return blockProperties(microGet(m)).solid;
}

PlanetSurface::PlanetSurface(std::uint64_t seed, PlanetClass planetClass, float referenceRadius)
    : seed_(seed), class_(planetClass), referenceRadius_(referenceRadius) {
    // v0.10: retain only a one-byte height/field sample per surface column. The
    // 32-layer voxel baseline is never materialized; cells are regenerated from
    // this deterministic field + material rules when queried.
    for (int f=0;f<FaceCount;++f) {
        const auto face=static_cast<CubeFace>(f);
        for (int v=0;v<FaceResolution;++v) for (int u=0;u<FaceResolution;++u) {
            generatedSurfaceRadials_[static_cast<std::size_t>(surfaceColumnIndex(face,u,v))] =
                static_cast<std::uint8_t>(generatedSurfaceRadialFor(seed_,class_,face,u,v));
        }
    }
    chunkRevisions_.fill(revision_);
}

SurfaceCellAddress PlanetSurface::normalize(SurfaceCellAddress address) const {
    return normalizeAddress(address);
}

bool PlanetSurface::radialInBounds(int radial) const {
    return radial >= 0 && radial < RadialLayers;
}

int PlanetSurface::flatIndex(SurfaceCellAddress address) const {
    address = normalize(address);
    return address.radial + RadialLayers * (address.u + FaceResolution *
           (address.v + FaceResolution * faceIndex(address.face)));
}

SurfaceCellAddress PlanetSurface::cellFromFlatIndex(int index) const {
    const int cellsPerFace = FaceResolution * FaceResolution * RadialLayers;
    const int f = index / cellsPerFace;
    int rem = index - f * cellsPerFace;
    const int v = rem / (FaceResolution * RadialLayers);
    rem -= v * FaceResolution * RadialLayers;
    const int u = rem / RadialLayers;
    const int radial = rem - u * RadialLayers;
    return {static_cast<CubeFace>(std::clamp(f,0,FaceCount-1)),u,v,radial};
}

SurfaceChunkJournal* PlanetSurface::findJournal(const PlanetChunkAddress& address) {
    const auto it=journals_.find(stableChunkKey(address));
    return it==journals_.end()?nullptr:&it->second;
}

const SurfaceChunkJournal* PlanetSurface::findJournal(const PlanetChunkAddress& address) const {
    const auto it=journals_.find(stableChunkKey(address));
    return it==journals_.end()?nullptr:&it->second;
}

SurfaceChunkJournal& PlanetSurface::ensureJournal(const PlanetChunkAddress& address) {
    const auto key=stableChunkKey(address);
    auto [it,inserted]=journals_.try_emplace(key);
    if (inserted) it->second.address=address;
    return it->second;
}

SurfaceChunkJournal& PlanetSurface::ensureJournal(SurfaceCellAddress address) {
    return ensureJournal(chunkOf(normalize(address)));
}

void PlanetSurface::compactJournal(const PlanetChunkAddress& address) {
    const auto key=stableChunkKey(address);
    const auto it=journals_.find(key);
    if (it!=journals_.end() && it->second.empty()) journals_.erase(it);
}

void PlanetSurface::rebuildJournalInfluence(const PlanetChunkAddress& address) {
    auto* j=findJournal(address);
    if(!j) return;
    EditInfluenceSummary summary{};
    auto include=[&](SurfaceCellAddress a) {
        a=normalize(a);
        if(!summary.any) {
            summary.any=true;
            summary.minU=summary.maxU=a.u;
            summary.minV=summary.maxV=a.v;
            summary.minRadial=summary.maxRadial=a.radial;
        } else {
            summary.minU=std::min(summary.minU,a.u); summary.maxU=std::max(summary.maxU,a.u);
            summary.minV=std::min(summary.minV,a.v); summary.maxV=std::max(summary.maxV,a.v);
            summary.minRadial=std::min(summary.minRadial,a.radial); summary.maxRadial=std::max(summary.maxRadial,a.radial);
        }
    };
    auto classify=[&](SurfaceCellAddress a,bool currentAnySolid,bool currentAllSolid) {
        const bool baselineSolid=blockProperties(baseline(a)).solid;
        if(!baselineSolid && currentAnySolid) ++summary.addedSolidCells;
        if(baselineSolid && !currentAllSolid) ++summary.removedSolidCells;
        const int surface=generatedSurfaceRadial(a.face,a.u,a.v);
        if(currentAnySolid && a.radial>surface)
            summary.maxOutwardDelta=std::max(summary.maxOutwardDelta,a.radial-surface);
        if(baselineSolid && !currentAllSolid && a.radial<=surface)
            summary.maxInwardDepth=std::max(summary.maxInwardDepth,surface-a.radial+1);
    };

    for(const auto& [flat,type]:j->macroEdits) {
        // A refined cell may retain a macro override as the MicroBrick baseline.
        // Classify the final refined occupancy exactly once below rather than
        // counting the same authored cell as both a macro and micro edit.
        if(j->microBricks.contains(flat)) continue;
        const auto a=cellFromFlatIndex(flat);
        include(a);
        const bool solid=blockProperties(type).solid;
        classify(a,solid,solid);
    }
    for(const auto& [flat,brick]:j->microBricks) {
        const auto a=cellFromFlatIndex(flat);
        include(a);
        ++summary.refinedCells;
        summary.microOverrides+=static_cast<int>(brick.overrideCount());
        bool anySolid=false,allSolid=true;
        for(int i=0;i<MicroBrick::CellCount;++i) {
            const bool solid=blockProperties(brick.getIndex(i)).solid;
            anySolid=anySolid||solid;
            allSolid=allSolid&&solid;
        }
        classify(a,anySolid,allSolid);
    }
    j->influence=summary;
}

const SurfaceChunkJournal* PlanetSurface::journal(const PlanetChunkAddress& address) const {
    return findJournal(address);
}

BlockType PlanetSurface::get(SurfaceCellAddress address) const {
    if (address.radial < 0) return BlockType::Stone;
    if (address.radial >= RadialLayers) return BlockType::Air;
    address = normalize(address);
    const int idx=flatIndex(address);
    if (const auto* j=findJournal(chunkOf(address))) {
        if (const auto it=j->macroEdits.find(idx); it!=j->macroEdits.end()) return it->second;
    }
    return baseline(address);
}

BlockType PlanetSurface::get(CubeFace face, int u, int v, int radial) const {
    return get({face,u,v,radial});
}

BlockType PlanetSurface::baseline(SurfaceCellAddress address) const {
    if (!radialInBounds(address.radial)) return address.radial < 0 ? BlockType::Stone : BlockType::Air;
    address = normalize(address);
    const int surface=generatedSurfaceRadial(address.face,address.u,address.v);
    return generatedBlock(address.face,address.u,address.v,address.radial,surface);
}

void PlanetSurface::set(SurfaceCellAddress address, BlockType type, bool placedByPlayer) {
    if (!radialInBounds(address.radial)) return;
    address = normalize(address);
    const int idx = flatIndex(address);
    const auto chunk=chunkOf(address);
    const BlockType current=get(address);
    const auto* existing=findJournal(chunk);
    const bool hadRefinement=existing && existing->microBricks.contains(idx);
    if (current == type && !hadRefinement) return;

    auto& j=ensureJournal(chunk);
    j.microBricks.erase(idx); // macro replacement intentionally discards refined state
    if (type == baseline(address)) j.macroEdits.erase(idx);
    else j.macroEdits[idx] = type;
    if (type == BlockType::Air) j.placedMarkers.erase(idx);
    else if (placedByPlayer) j.placedMarkers.insert(idx);
    rebuildJournalInfluence(chunk);
    compactJournal(chunk);
    ++revision_;
    markDirty(address);
}

void PlanetSurface::applySavedEdit(int index, BlockType type) {
    if (index < 0 || index >= TotalCells) return;
    const SurfaceCellAddress address = cellFromFlatIndex(index);
    const auto chunk=chunkOf(address);
    const auto* existing=findJournal(chunk);
    const bool hadRefinement=existing && existing->microBricks.contains(index);
    if (get(address) == type && !hadRefinement) return;
    auto& j=ensureJournal(chunk);
    j.microBricks.erase(index);
    if (type == baseline(address)) j.macroEdits.erase(index);
    else j.macroEdits[index] = type;
    rebuildJournalInfluence(chunk);
    compactJournal(chunk);
    ++revision_;
    markDirty(address);
}

void PlanetSurface::applySavedPlacedMarker(int index) {
    if (index < 0 || index >= TotalCells) return;
    const auto address=cellFromFlatIndex(index);
    if (blockProperties(get(address)).solid) ensureJournal(address).placedMarkers.insert(index);
}

bool PlanetSurface::playerPlaced(SurfaceCellAddress address) const {
    if (!radialInBounds(address.radial)) return false;
    address = normalize(address);
    const auto* j=findJournal(chunkOf(address));
    return j && j->placedMarkers.contains(flatIndex(address));
}

MicroBrick& PlanetSurface::refineCell(SurfaceCellAddress address) {
    address=normalize(address);
    const int idx=flatIndex(address);
    auto& j=ensureJournal(address);
    auto it=j.microBricks.find(idx);
    if (it!=j.microBricks.end()) return it->second;
    auto [created,_]=j.microBricks.emplace(idx,MicroBrick(get(address)));
    return created->second;
}

bool PlanetSurface::isRefined(SurfaceCellAddress address) const {
    if (!radialInBounds(address.radial)) return false;
    address=normalize(address);
    const auto* j=findJournal(chunkOf(address));
    return j && j->microBricks.contains(flatIndex(address));
}

const MicroBrick* PlanetSurface::microBrick(SurfaceCellAddress address) const {
    if (!radialInBounds(address.radial)) return nullptr;
    address=normalize(address);
    const auto* j=findJournal(chunkOf(address));
    if (!j) return nullptr;
    const auto it=j->microBricks.find(flatIndex(address));
    return it==j->microBricks.end()?nullptr:&it->second;
}

BlockType PlanetSurface::microGet(SurfaceCellAddress address, int mu, int mr, int mv) const {
    if (!radialInBounds(address.radial)) return address.radial<0?BlockType::Stone:BlockType::Air;
    address=normalize(address);
    const auto* brick=microBrick(address);
    if (!brick) return get(address);
    return brick->get(mu,mr,mv);
}

BlockType PlanetSurface::microGet(const SurfaceMicroAddress& address) const {
    return microGet(address.cell,address.u,address.radial,address.v);
}

void PlanetSurface::setMicro(SurfaceCellAddress address, int mu, int mr, int mv, BlockType type) {
    if (!radialInBounds(address.radial) || !MicroBrick::inBounds(mu,mr,mv)) return;
    address=normalize(address);
    const BlockType before=microGet(address,mu,mr,mv);
    if (before==type) return;
    const int idx=flatIndex(address);
    const auto chunk=chunkOf(address);
    MicroBrick& brick=refineCell(address);
    brick.set(mu,mr,mv,type);
    if (brick.overrideCount()==0) {
        if (auto* j=findJournal(chunk)) j->microBricks.erase(idx);
    }
    rebuildJournalInfluence(chunk);
    compactJournal(chunk);
    ++revision_;
    markDirty(address);
}

void PlanetSurface::setMicroIndex(SurfaceCellAddress address, int microIndex, BlockType type) {
    if (microIndex<0 || microIndex>=MicroBrick::CellCount) return;
    const int mu=microIndex%MicroBrick::Resolution;
    const int t=microIndex/MicroBrick::Resolution;
    const int mv=t%MicroBrick::Resolution;
    const int mr=t/MicroBrick::Resolution;
    setMicro(address,mu,mr,mv,type);
}

void PlanetSurface::applySavedMicroEdit(int flatCellIndex, int microIndex, BlockType type) {
    if (flatCellIndex<0 || flatCellIndex>=TotalCells ||
        microIndex<0 || microIndex>=MicroBrick::CellCount) return;
    setMicroIndex(cellFromFlatIndex(flatCellIndex),microIndex,type);
}

std::size_t PlanetSurface::microOverrideCount() const {
    std::size_t count=0;
    for (const auto& [_,journal] : journals_) count += journal.microOverrideCount();
    return count;
}

std::size_t PlanetSurface::macroEditCount() const {
    std::size_t count=0;
    for (const auto& [_,journal] : journals_) count += journal.macroEdits.size();
    return count;
}

std::size_t PlanetSurface::placedMarkerCount() const {
    std::size_t count=0;
    for (const auto& [_,journal] : journals_) count += journal.placedMarkers.size();
    return count;
}

std::size_t PlanetSurface::persistentCellStateCount() const {
    std::size_t count=0;
    for (const auto& [_,journal] : journals_) count += journal.persistentCellStateCount();
    return count;
}

int PlanetSurface::surfaceRadial(CubeFace face, int u, int v) const {
    const auto a = normalize({face,u,v,0});
    for (int r=RadialLayers-1;r>=0;--r) {
        // A refined cell is treated as surface-bearing if any micro material is
        // solid. This keeps radial spawn/ground queries stable after sculpting.
        const SurfaceCellAddress c{a.face,a.u,a.v,r};
        if (const auto* brick=microBrick(c)) {
            bool anySolid=false;
            for (int i=0;i<MicroBrick::CellCount && !anySolid;++i)
                anySolid=blockProperties(brick->getIndex(i)).solid;
            if (anySolid) return r;
        } else if (blockProperties(get(c)).solid) return r;
    }
    return -1;
}

Vec3 PlanetSurface::boundaryPosition(CubeFace face, int uEdge, int vEdge, int radialBoundary) const {
    const Vec3 d = faceGridCornerDirection(face,uEdge,vEdge,FaceResolution);
    const float radius = referenceRadius_ + static_cast<float>(radialBoundary - ReferenceRadial);
    return d * radius;
}

Vec3 PlanetSurface::cellCenterPosition(SurfaceCellAddress address) const {
    address = normalize(address);
    const Vec3 d = faceGridCellDirection(address.face,address.u,address.v,FaceResolution);
    const float radius = referenceRadius_ + (static_cast<float>(address.radial) + 0.5f - static_cast<float>(ReferenceRadial));
    return d * radius;
}

Vec3 PlanetSurface::microBoundaryPosition(SurfaceCellAddress address, int uEdge, int radialEdge, int vEdge) const {
    return microBoundary(referenceRadius_,address,uEdge,radialEdge,vEdge);
}

Vec3 PlanetSurface::microCellCenterPosition(const SurfaceMicroAddress& address) const {
    return microBoundaryPosition(address.cell,address.u,address.radial,address.v) * 0.125f +
           microBoundaryPosition(address.cell,address.u+1,address.radial,address.v) * 0.125f +
           microBoundaryPosition(address.cell,address.u,address.radial+1,address.v) * 0.125f +
           microBoundaryPosition(address.cell,address.u,address.radial,address.v+1) * 0.125f +
           microBoundaryPosition(address.cell,address.u+1,address.radial+1,address.v) * 0.125f +
           microBoundaryPosition(address.cell,address.u+1,address.radial,address.v+1) * 0.125f +
           microBoundaryPosition(address.cell,address.u,address.radial+1,address.v+1) * 0.125f +
           microBoundaryPosition(address.cell,address.u+1,address.radial+1,address.v+1) * 0.125f;
}

SurfaceCellAddress PlanetSurface::locateUnclamped(Vec3 p) const {
    const float radius = length(p);
    const FaceUv uv = directionToFaceUv(p);
    SurfaceCellAddress out{};
    out.face = uv.face;
    out.u = clampCell(uv.u,FaceResolution);
    out.v = clampCell(uv.v,FaceResolution);
    out.radial = static_cast<int>(std::floor(radius - referenceRadius_ + static_cast<float>(ReferenceRadial)));
    return out;
}

SurfaceCellAddress PlanetSurface::locate(Vec3 p) const {
    auto out = locateUnclamped(p);
    out.radial = std::clamp(out.radial,0,RadialLayers-1);
    return out;
}

SurfaceMicroAddress PlanetSurface::locateMicro(Vec3 p) const {
    return locateMicroCommon(referenceRadius_,p);
}

SurfaceRayHit PlanetSurface::raycast(Vec3 origin, Vec3 direction, float maxDistance, float step) const {
    SurfaceRayHit out{};
    direction = ::elysium::normalize(direction);
    if (lengthSq(direction) < 0.5f || maxDistance <= 0.0f) return out;
    step = std::clamp(step,0.025f,0.5f);

    SurfaceCellAddress previous = locateUnclamped(origin);
    bool havePrevious = radialInBounds(previous.radial);
    for (float t=0.0f; t<=maxDistance + step*0.5f; t+=step) {
        const Vec3 point = origin + direction*t;
        const SurfaceCellAddress cell = locateUnclamped(point);
        if (radialInBounds(cell.radial)) {
            if (solidAt(point)) {
                out.hit = true;
                out.cell = normalize(cell);
                out.previous = havePrevious ? normalize(previous) : out.cell;
                out.point = point;
                out.distance = t;
                return out;
            }
            previous = cell;
            havePrevious = true;
        }
    }
    return out;
}

SurfaceMicroRayHit PlanetSurface::raycastMicro(Vec3 origin, Vec3 direction, float maxDistance, float step) const {
    SurfaceMicroRayHit out{};
    direction=::elysium::normalize(direction);
    if (lengthSq(direction)<0.5f || maxDistance<=0.0f) return out;
    step=std::clamp(step,0.008f,0.05f);
    SurfaceMicroAddress last{};
    bool haveLast=false;
    for (float t=0.0f;t<=maxDistance+step*0.5f;t+=step) {
        const Vec3 p=origin+direction*t;
        const auto m=locateMicro(p);
        if (!radialInBounds(m.cell.radial)) continue;
        if (haveLast && m==last) continue;
        last=m; haveLast=true;
        const BlockType type=microGet(m);
        if (blockProperties(type).solid) {
            out.hit=true; out.micro=m; out.point=p; out.distance=t; out.type=type;
            return out;
        }
    }
    return out;
}

bool PlanetSurface::solidAt(Vec3 p) const {
    const auto m=locateMicro(p);
    if (!radialInBounds(m.cell.radial)) return m.cell.radial<0;
    return blockProperties(microGet(m)).solid;
}

bool PlanetSurface::capsuleCollides(Vec3 feet, float radius, float height) const {
    if (lengthSq(feet)<1.0f) return true;
    const auto frame=surfaceFrame(feet);
    const float low=0.03f;
    const float high=std::max(low,height-0.10f);
    const std::array<float,3> levels{low,(low+high)*0.5f,high};
    const std::array<Vec3,9> offsets{{
        {0,0,0},
        frame.right*radius, frame.right*(-radius),
        frame.forward*radius, frame.forward*(-radius),
        ::elysium::normalize(frame.right+frame.forward)*radius,
        ::elysium::normalize(frame.right-frame.forward)*radius,
        ::elysium::normalize(frame.forward-frame.right)*radius,
        ::elysium::normalize((frame.right+frame.forward)*-1.0f)*radius
    }};
    for (float h:levels) {
        const Vec3 center=feet+frame.up*h;
        for (const Vec3 off:offsets) if (solidAt(center+off)) return true;
    }
    return false;
}

bool PlanetSurface::groundedAt(Vec3 feet, float probe) const {
    if (lengthSq(feet)<1.0f) return false;
    const Vec3 up=::elysium::normalize(feet);
    return capsuleCollides(feet-up*std::max(0.01f,probe));
}

float PlanetSurface::surfaceBoundaryRadius(Vec3 direction) const {
    const FaceUv uv = directionToFaceUv(direction);
    const int u = clampCell(uv.u,FaceResolution);
    const int v = clampCell(uv.v,FaceResolution);
    const int surface = surfaceRadial(uv.face,u,v);
    if (surface < 0) return referenceRadius_ - static_cast<float>(ReferenceRadial);
    return referenceRadius_ + static_cast<float>(surface + 1 - ReferenceRadial);
}

SurfaceFrame PlanetSurface::surfaceFrame(Vec3 p) const {
    SurfaceFrame frame{};
    frame.position = p;
    frame.up = ::elysium::normalize(p);
    frame.forward = tangentReference(frame.up);
    frame.right = ::elysium::normalize(cross(frame.forward,frame.up));
    frame.forward = ::elysium::normalize(cross(frame.up,frame.right));
    return frame;
}

Vec3 PlanetSurface::gravityDirectionAt(Vec3 p) const {
    const Vec3 up = ::elysium::normalize(p);
    return up * -1.0f;
}

Vec3 PlanetSurface::cameraRelative(Vec3 p, Vec3 camera) const {
    return p - camera;
}

bool PlanetSurface::gasPassable(SurfaceCellAddress address) const {
    if (address.radial<0) return false;
    if (address.radial>=RadialLayers) return true;
    address=normalize(address);
    if (!blockProperties(get(address)).solid) return true;
    const auto* brick=microBrick(address);
    if (!brick) return false;
    // Conservative v0.6 seal rule: any non-solid microcell makes the macro cell
    // gas-passable. This guarantees chiseled breaches are never falsely sealed;
    // exact micro-pore connectivity can replace it without changing save data.
    for (int i=0;i<MicroBrick::CellCount;++i)
        if (!blockProperties(brick->getIndex(i)).solid) return true;
    return false;
}

SurfaceSealedVolumeQuery PlanetSurface::sealedVolume(SurfaceCellAddress start, int maxCells) const {
    SurfaceSealedVolumeQuery out{};
    maxCells=std::max(1,maxCells);
    if (!radialInBounds(start.radial)) return out;
    start=normalize(start);
    if (!gasPassable(start)) return out;

    std::deque<SurfaceCellAddress> q;
    std::unordered_set<int> visited;
    q.push_back(start);
    visited.insert(flatIndex(start));
    bool leaked=false;

    while(!q.empty()) {
        const auto c=q.front(); q.pop_front();
        out.cells.push_back(flatIndex(c));
        if (static_cast<int>(out.cells.size())>=maxCells) {
            out.truncated=true;
            out.sealed=false;
            return out;
        }
        const std::array<SurfaceCellAddress,6> neighbors{{
            {c.face,c.u+1,c.v,c.radial},{c.face,c.u-1,c.v,c.radial},
            {c.face,c.u,c.v+1,c.radial},{c.face,c.u,c.v-1,c.radial},
            {c.face,c.u,c.v,c.radial+1},{c.face,c.u,c.v,c.radial-1}
        }};
        for (auto n:neighbors) {
            if (n.radial>=RadialLayers) { leaked=true; continue; }
            if (n.radial<0) continue;
            n=normalize(n);
            if (!gasPassable(n)) continue;
            const int idx=flatIndex(n);
            if (visited.insert(idx).second) q.push_back(n);
        }
    }
    out.sealed=!leaked;
    return out;
}

PlanetChunkAddress PlanetSurface::chunkOf(SurfaceCellAddress address) const {
    address = normalize(address);
    return chunkAddress(address.face,address.u,address.v,address.radial,ChunkSize);
}

int PlanetSurface::chunkSlot(const PlanetChunkAddress& address) const {
    const int f = faceIndex(address.face);
    if (address.u < 0 || address.u >= ChunksPerFaceAxis ||
        address.v < 0 || address.v >= ChunksPerFaceAxis ||
        address.radial < 0 || address.radial >= RadialChunks) return -1;
    return address.u + ChunksPerFaceAxis * (address.v + ChunksPerFaceAxis *
           (address.radial + RadialChunks * f));
}

std::uint64_t PlanetSurface::chunkRevision(const PlanetChunkAddress& address) const {
    const int slot = chunkSlot(address);
    return slot >= 0 ? chunkRevisions_[static_cast<std::size_t>(slot)] : 0;
}

void PlanetSurface::markDirty(SurfaceCellAddress address) {
    const auto own = chunkOf(address);
    const int ownSlot = chunkSlot(own);
    if (ownSlot >= 0) chunkRevisions_[static_cast<std::size_t>(ownSlot)] = revision_;

    constexpr std::array<IVec3,4> dirs{{{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}}};
    for (const auto& d : dirs) {
        const auto n = normalize({address.face,address.u+d.x,address.v+d.y,address.radial});
        const auto nc = chunkOf(n);
        if (!(nc == own)) {
            const int slot = chunkSlot(nc);
            if (slot >= 0) chunkRevisions_[static_cast<std::size_t>(slot)] = revision_;
        }
    }
}

int PlanetSurface::generatedSurfaceRadial(CubeFace face, int u, int v) const {
    const auto a=normalize({face,u,v,0});
    return static_cast<int>(generatedSurfaceRadials_[static_cast<std::size_t>(surfaceColumnIndex(a.face,a.u,a.v))]);
}

BlockType PlanetSurface::generatedBlock(CubeFace face, int u, int v, int radial, int surface) const {
    return generatedBlockFor(seed_,class_,face,u,v,radial,surface);
}

int PlanetSurface::chunkEditCount(const PlanetChunkAddress& address) const {
    const auto* j=findJournal(address);
    if (!j) return 0;
    int count=static_cast<int>(j->macroEdits.size());
    for (const auto& [idx,_] : j->microBricks) if (!j->macroEdits.contains(idx)) ++count;
    return count;
}

EditInfluenceSummary PlanetSurface::editInfluenceSummary(const PlanetChunkAddress& address) const {
    const auto* j=findJournal(address);
    return j?j->influence:EditInfluenceSummary{};
}

PlanetSurfaceSnapshot PlanetSurface::snapshot() const {
    PlanetSurfaceSnapshot out{};
    out.seed = seed_;
    out.planetClass = class_;
    out.referenceRadius = referenceRadius_;
    out.generatedSurfaceRadials = generatedSurfaceRadials_;
    for (const auto& [key,journal] : journals_) {
        out.edits.insert(journal.macroEdits.begin(),journal.macroEdits.end());
        out.microBricks.insert(journal.microBricks.begin(),journal.microBricks.end());
        if(journal.influence.any) out.editInfluenceSummaries.emplace(key,journal.influence);
    }
    return out;
}

} // namespace elysium
