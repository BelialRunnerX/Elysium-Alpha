#include "world/PlanetSurfaceMesher.hpp"

#include "core/Determinism.hpp"
#include "world/Block.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace elysium {
namespace {

struct Bucket {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<std::uint8_t> colors;
    int macroQuads{};
    int microQuads{};
    int aoDarkenedCorners{};
};

Color4u shade(Color4u c, float factor) {
    c.r = static_cast<std::uint8_t>(std::clamp(static_cast<int>(static_cast<float>(c.r)*factor),0,255));
    c.g = static_cast<std::uint8_t>(std::clamp(static_cast<int>(static_cast<float>(c.g)*factor),0,255));
    c.b = static_cast<std::uint8_t>(std::clamp(static_cast<int>(static_cast<float>(c.b)*factor),0,255));
    return c;
}

class Builder {
public:
    void emit(BlockType type, Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3,
              Vec3 desiredNormal, float light, bool micro) {
        if (!blockProperties(type).solid) return;
        const Color4u c=shade(blockProperties(type).color,light);
        emitColoredCorners(type,{c,c,c,c},p0,p1,p2,p3,desiredNormal,micro,0);
    }

    void emitAo(BlockType type, Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3,
                Vec3 desiredNormal, float light, bool micro,
                const std::array<float,4>& ao) {
        if (!blockProperties(type).solid) return;
        std::array<Color4u,4> colors{};
        int dark=0;
        for(std::size_t i=0;i<4;++i) {
            const float a=std::clamp(ao[i],0.45f,1.0f);
            if(a<0.999f) ++dark;
            colors[i]=shade(blockProperties(type).color,light*a);
        }
        emitColoredCorners(type,colors,p0,p1,p2,p3,desiredNormal,micro,dark);
    }

    void emitColored(BlockType materialTag, Color4u c, Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3,
                     Vec3 desiredNormal, bool micro=false) {
        emitColoredCorners(materialTag,{c,c,c,c},p0,p1,p2,p3,desiredNormal,micro,0);
    }

    void emitColoredCorners(BlockType materialTag, std::array<Color4u,4> colors,
                            Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3,
                            Vec3 desiredNormal, bool micro=false, int darkenedCorners=0) {
        if (!blockProperties(materialTag).solid) materialTag=BlockType::Stone;
        Vec3 n = normalize(cross(p1-p0,p2-p0));
        if (lengthSq(n)<0.5f) return;
        if (dot(n,desiredNormal) < 0.0f) {
            std::swap(p1,p3);
            std::swap(colors[1],colors[3]);
            n = normalize(cross(p1-p0,p2-p0));
        }
        const std::array<Vec3,6> verts{p0,p1,p2,p0,p2,p3};
        constexpr std::array<int,6> ci{0,1,2,0,2,3};
        auto& b = buckets_[static_cast<std::size_t>(materialTag)];
        for (std::size_t i=0;i<verts.size();++i) {
            const auto& p=verts[i];
            const auto c=colors[static_cast<std::size_t>(ci[i])];
            b.vertices.insert(b.vertices.end(),{p.x,p.y,p.z});
            b.normals.insert(b.normals.end(),{n.x,n.y,n.z});
            b.colors.insert(b.colors.end(),{c.r,c.g,c.b,c.a});
        }
        if (micro) ++b.microQuads; else ++b.macroQuads;
        b.aoDarkenedCorners += darkenedCorners;
    }

    CpuMeshData finish() {
        CpuMeshData out{};
        for (int raw=1;raw<kBlockTypeCount;++raw) {
            auto& b=buckets_[static_cast<std::size_t>(raw)];
            if (b.vertices.empty()) continue;
            MaterialRange range{};
            range.material=static_cast<BlockType>(raw);
            range.firstVertex=out.vertexCount();
            range.vertexCount=static_cast<int>(b.vertices.size()/3U);
            range.quads=b.macroQuads+b.microQuads;
            out.materialRanges.push_back(range);
            out.vertices.insert(out.vertices.end(),b.vertices.begin(),b.vertices.end());
            out.normals.insert(out.normals.end(),b.normals.begin(),b.normals.end());
            out.colors.insert(out.colors.end(),b.colors.begin(),b.colors.end());
            out.macroQuads += b.macroQuads;
            out.microQuads += b.microQuads;
            out.aoDarkenedCorners += b.aoDarkenedCorners;
        }
        out.quads=out.macroQuads+out.microQuads;
        return out;
    }
private:
    std::array<Bucket,kBlockTypeCount> buckets_{};
};

enum class CellFace : int { UPos, UNeg, VPos, VNeg, RPos, RNeg };

Vec3 avg4(Vec3 a,Vec3 b,Vec3 c,Vec3 d) { return (a+b+c+d)*0.25f; }

int generatedSurfaceAt(const PlanetSurfaceSnapshot& p, CubeFace face, int u, int v) {
    u=std::clamp(u,0,PlanetSurfaceSnapshot::FaceResolution-1);
    v=std::clamp(v,0,PlanetSurfaceSnapshot::FaceResolution-1);
    const int f=static_cast<int>(face);
    const std::size_t idx=static_cast<std::size_t>(u + PlanetSurfaceSnapshot::FaceResolution *
        (v + PlanetSurfaceSnapshot::FaceResolution * f));
    return static_cast<int>(p.generatedSurfaceRadials[idx]);
}

Color4u climateColor(const PlanetSurfaceSnapshot& p, CubeFace face, int u, int v, int surface) {
    const float local=hash01(p.seed,u,v,static_cast<int>(face),0xC11A7EULL);
    const float height=static_cast<float>(surface-PlanetSurfaceSnapshot::ReferenceRadial);
    Color4u c{};
    switch (p.planetClass) {
        case PlanetClass::Temperate:
            if (height < -0.5f) c={33,82,112,255};
            else if (height > 5.0f) c={142,146,132,255};
            else c=local>0.68f?Color4u{112,101,69,255}:Color4u{66,110,73,255};
            break;
        case PlanetClass::Barren:
            c=local>0.72f?Color4u{157,143,128,255}:Color4u{119,112,108,255};
            break;
        case PlanetClass::Scorched:
            c=local>0.76f?Color4u{183,78,42,255}:Color4u{91,66,61,255};
            break;
        case PlanetClass::Frozen:
            c=local>0.70f?Color4u{198,219,224,255}:Color4u{116,144,160,255};
            break;
        case PlanetClass::Toxic:
            c=local>0.70f?Color4u{132,157,76,255}:Color4u{72,92,66,255};
            break;
        case PlanetClass::Irradiated:
            c=local>0.72f?Color4u{126,132,83,255}:Color4u{85,78,72,255};
            break;
        case PlanetClass::Oceanic:
            c=height<0.5f?Color4u{29,84,118,255}:Color4u{72,118,105,255};
            break;
        case PlanetClass::Anomalous:
            c=local>0.5f?Color4u{112,82,145,255}:Color4u{54,107,118,255};
            break;
    }
    const float shadeFactor=std::clamp(0.90f+height*0.018f,0.72f,1.12f);
    return shade(c,shadeFactor);
}

SurfaceCellAddress neighborOf(const PlanetSurfaceSnapshot& p, SurfaceCellAddress a, CellFace face) {
    switch(face) {
        case CellFace::UPos: ++a.u; break;
        case CellFace::UNeg: --a.u; break;
        case CellFace::VPos: ++a.v; break;
        case CellFace::VNeg: --a.v; break;
        case CellFace::RPos: ++a.radial; break;
        case CellFace::RNeg: --a.radial; break;
    }
    if (a.radial>=0 && a.radial<PlanetSurfaceSnapshot::RadialLayers) a=p.normalize(a);
    return a;
}

float faceLight(CellFace face) {
    switch(face) {
        case CellFace::RPos: return 1.0f;
        case CellFace::RNeg: return 0.60f;
        case CellFace::UPos: return 0.80f;
        case CellFace::UNeg: return 0.76f;
        case CellFace::VPos: return 0.74f;
        case CellFace::VNeg: return 0.70f;
    }
    return 0.8f;
}

std::array<Vec3,4> macroFaceCorners(const PlanetSurfaceSnapshot& p, SurfaceCellAddress a, CellFace face) {
    a=p.normalize(a);
    const int u=a.u,v=a.v,r=a.radial;
    switch(face) {
        case CellFace::RPos: return {p.boundaryPosition(a.face,u,v,r+1),p.boundaryPosition(a.face,u+1,v,r+1),p.boundaryPosition(a.face,u+1,v+1,r+1),p.boundaryPosition(a.face,u,v+1,r+1)};
        case CellFace::RNeg: return {p.boundaryPosition(a.face,u,v,r),p.boundaryPosition(a.face,u,v+1,r),p.boundaryPosition(a.face,u+1,v+1,r),p.boundaryPosition(a.face,u+1,v,r)};
        case CellFace::UPos: return {p.boundaryPosition(a.face,u+1,v,r),p.boundaryPosition(a.face,u+1,v,r+1),p.boundaryPosition(a.face,u+1,v+1,r+1),p.boundaryPosition(a.face,u+1,v+1,r)};
        case CellFace::UNeg: return {p.boundaryPosition(a.face,u,v,r),p.boundaryPosition(a.face,u,v+1,r),p.boundaryPosition(a.face,u,v+1,r+1),p.boundaryPosition(a.face,u,v,r+1)};
        case CellFace::VPos: return {p.boundaryPosition(a.face,u,v+1,r),p.boundaryPosition(a.face,u+1,v+1,r),p.boundaryPosition(a.face,u+1,v+1,r+1),p.boundaryPosition(a.face,u,v+1,r+1)};
        case CellFace::VNeg: return {p.boundaryPosition(a.face,u,v,r),p.boundaryPosition(a.face,u,v,r+1),p.boundaryPosition(a.face,u+1,v,r+1),p.boundaryPosition(a.face,u+1,v,r)};
    }
    return {};
}

std::array<Vec3,4> microFaceCorners(const PlanetSurfaceSnapshot& p, const SurfaceMicroAddress& m, CellFace face) {
    const int u=m.u,v=m.v,r=m.radial;
    switch(face) {
        case CellFace::RPos: return {p.microBoundaryPosition(m.cell,u,r+1,v),p.microBoundaryPosition(m.cell,u+1,r+1,v),p.microBoundaryPosition(m.cell,u+1,r+1,v+1),p.microBoundaryPosition(m.cell,u,r+1,v+1)};
        case CellFace::RNeg: return {p.microBoundaryPosition(m.cell,u,r,v),p.microBoundaryPosition(m.cell,u,r,v+1),p.microBoundaryPosition(m.cell,u+1,r,v+1),p.microBoundaryPosition(m.cell,u+1,r,v)};
        case CellFace::UPos: return {p.microBoundaryPosition(m.cell,u+1,r,v),p.microBoundaryPosition(m.cell,u+1,r+1,v),p.microBoundaryPosition(m.cell,u+1,r+1,v+1),p.microBoundaryPosition(m.cell,u+1,r,v+1)};
        case CellFace::UNeg: return {p.microBoundaryPosition(m.cell,u,r,v),p.microBoundaryPosition(m.cell,u,r,v+1),p.microBoundaryPosition(m.cell,u,r+1,v+1),p.microBoundaryPosition(m.cell,u,r+1,v)};
        case CellFace::VPos: return {p.microBoundaryPosition(m.cell,u,r,v+1),p.microBoundaryPosition(m.cell,u+1,r,v+1),p.microBoundaryPosition(m.cell,u+1,r+1,v+1),p.microBoundaryPosition(m.cell,u,r+1,v+1)};
        case CellFace::VNeg: return {p.microBoundaryPosition(m.cell,u,r,v),p.microBoundaryPosition(m.cell,u,r+1,v),p.microBoundaryPosition(m.cell,u+1,r+1,v),p.microBoundaryPosition(m.cell,u+1,r,v)};
    }
    return {};
}

bool outsideSolid(const PlanetSurfaceSnapshot& p, const std::array<Vec3,4>& quad, Vec3 ownerCenter) {
    const Vec3 fc=avg4(quad[0],quad[1],quad[2],quad[3]);
    const Vec3 outward=normalize(fc-ownerCenter);
    return p.solidAt(fc+outward*0.004f);
}

void emitMacroFace(Builder& b,const PlanetSurfaceSnapshot& p,SurfaceCellAddress a,CellFace face,BlockType type) {
    const auto q=macroFaceCorners(p,a,face);
    const Vec3 center=p.cellCenterPosition(a);
    const Vec3 desired=normalize(avg4(q[0],q[1],q[2],q[3])-center);
    b.emit(type,q[0],q[1],q[2],q[3],desired,faceLight(face),false);
}

void emitTiledMacroBoundary(Builder& b,const PlanetSurfaceSnapshot& p,SurfaceCellAddress a,CellFace face,BlockType type) {
    // Only used when the neighbor is refined. Tiling avoids drawing hidden
    // portions of an otherwise homogeneous one-metre face.
    const int N=MicroBrick::Resolution;
    for(int i=0;i<N;++i) for(int j=0;j<N;++j) {
        SurfaceMicroAddress m{a,0,0,0};
        switch(face) {
            case CellFace::RPos: m={a,i,N-1,j}; break;
            case CellFace::RNeg: m={a,i,0,j}; break;
            case CellFace::UPos: m={a,N-1,i,j}; break;
            case CellFace::UNeg: m={a,0,i,j}; break;
            case CellFace::VPos: m={a,i,j,N-1}; break;
            case CellFace::VNeg: m={a,i,j,0}; break;
        }
        const auto q=microFaceCorners(p,m,face);
        const Vec3 mc=p.microCellCenterPosition(m);
        if (!outsideSolid(p,q,mc)) {
            const Vec3 desired=normalize(avg4(q[0],q[1],q[2],q[3])-mc);
            b.emit(type,q[0],q[1],q[2],q[3],desired,faceLight(face),true);
        }
    }
}

void emitRefinedCell(Builder& b,const PlanetSurfaceSnapshot& p,SurfaceCellAddress a) {
    const auto* brick=p.microBrick(a);
    if (!brick) return;
    constexpr std::array<CellFace,6> faces{CellFace::UPos,CellFace::UNeg,CellFace::VPos,CellFace::VNeg,CellFace::RPos,CellFace::RNeg};
    for(int mr=0;mr<MicroBrick::Resolution;++mr) {
        for(int mv=0;mv<MicroBrick::Resolution;++mv) {
            for(int mu=0;mu<MicroBrick::Resolution;++mu) {
                const BlockType type=brick->get(mu,mr,mv);
                if(!blockProperties(type).solid) continue;
                const SurfaceMicroAddress m{a,mu,mr,mv};
                const Vec3 mc=p.microCellCenterPosition(m);
                for(const CellFace face:faces) {
                    const auto q=microFaceCorners(p,m,face);
                    if (outsideSolid(p,q,mc)) continue;
                    const Vec3 desired=normalize(avg4(q[0],q[1],q[2],q[3])-mc);
                    b.emit(type,q[0],q[1],q[2],q[3],desired,faceLight(face),true);
                }
            }
        }
    }
}

} // namespace

CpuMeshData buildPlanetSurfaceChunkMesh(const PlanetSurfaceSnapshot& planet,
                                        const PlanetChunkAddress& chunk) {
    Builder builder;
    if (chunk.radial != 0 || chunk.u < 0 || chunk.u >= PlanetSurface::ChunksPerFaceAxis ||
        chunk.v < 0 || chunk.v >= PlanetSurface::ChunksPerFaceAxis) return builder.finish();

    const int u0 = chunk.u * PlanetSurface::ChunkSize;
    const int v0 = chunk.v * PlanetSurface::ChunkSize;
    const int u1 = std::min(u0 + PlanetSurface::ChunkSize, PlanetSurfaceSnapshot::FaceResolution);
    const int v1 = std::min(v0 + PlanetSurface::ChunkSize, PlanetSurfaceSnapshot::FaceResolution);
    constexpr std::array<CellFace,6> faces{CellFace::UPos,CellFace::UNeg,CellFace::VPos,CellFace::VNeg,CellFace::RPos,CellFace::RNeg};

    for(int v=v0;v<v1;++v) for(int u=u0;u<u1;++u) for(int r=0;r<PlanetSurfaceSnapshot::RadialLayers;++r) {
        const SurfaceCellAddress a{chunk.face,u,v,r};
        const BlockType type=planet.get(a);
        if (!blockProperties(type).solid && !planet.isRefined(a)) continue;
        if (planet.isRefined(a)) {
            emitRefinedCell(builder,planet,a);
            continue;
        }
        if (!blockProperties(type).solid) continue;

        for(const CellFace face:faces) {
            const auto n=neighborOf(planet,a,face);
            if (planet.radialInBounds(n.radial) && planet.isRefined(n)) {
                emitTiledMacroBoundary(builder,planet,a,face,type);
            } else if (!blockProperties(planet.get(n)).solid) {
                emitMacroFace(builder,planet,a,face,type);
            }
        }
    }
    return builder.finish();
}


namespace {

SurfaceCellAddress cachedWorldAddress(const SurfaceChunkData& c,int lu,int lv,int lr) {
    return c.worldAddress(lu,lv,lr);
}

Vec3 cachedBoundaryPosition(const SurfaceChunkData& c, CubeFace face, int uEdge, int vEdge, int radialBoundary) {
    const Vec3 d=faceGridCornerDirection(face,uEdge,vEdge,PlanetSurface::FaceResolution);
    const float radius=c.referenceRadius + static_cast<float>(radialBoundary-PlanetSurface::ReferenceRadial);
    return d*radius;
}

Vec3 cachedCellCenter(const SurfaceChunkData& c, SurfaceCellAddress a) {
    if (a.u<0 || a.u>=PlanetSurface::FaceResolution || a.v<0 || a.v>=PlanetSurface::FaceResolution) {
        const auto wrapped=wrapFaceCell(a.face,a.u,a.v,PlanetSurface::FaceResolution);
        a.face=wrapped.face; a.u=wrapped.u; a.v=wrapped.v;
    }
    const Vec3 d=faceGridCellDirection(a.face,a.u,a.v,PlanetSurface::FaceResolution);
    const float radius=c.referenceRadius + static_cast<float>(a.radial)+0.5f-static_cast<float>(PlanetSurface::ReferenceRadial);
    return d*radius;
}

Vec3 cachedMicroBoundary(const SurfaceChunkData& c, SurfaceCellAddress a,
                         int uEdge,int radialEdge,int vEdge) {
    if (a.u<0 || a.u>=PlanetSurface::FaceResolution || a.v<0 || a.v>=PlanetSurface::FaceResolution) {
        const auto wrapped=wrapFaceCell(a.face,a.u,a.v,PlanetSurface::FaceResolution);
        a.face=wrapped.face; a.u=wrapped.u; a.v=wrapped.v;
    }
    const float fu=static_cast<float>(a.u)+static_cast<float>(std::clamp(uEdge,0,MicroBrick::Resolution))/MicroBrick::Resolution;
    const float fv=static_cast<float>(a.v)+static_cast<float>(std::clamp(vEdge,0,MicroBrick::Resolution))/MicroBrick::Resolution;
    const float u=fu/static_cast<float>(PlanetSurface::FaceResolution)*2.0f-1.0f;
    const float v=fv/static_cast<float>(PlanetSurface::FaceResolution)*2.0f-1.0f;
    const Vec3 d=faceUvToDirection(a.face,u,v);
    const float rr=static_cast<float>(a.radial)+static_cast<float>(std::clamp(radialEdge,0,MicroBrick::Resolution))/MicroBrick::Resolution;
    return d*(c.referenceRadius+rr-static_cast<float>(PlanetSurface::ReferenceRadial));
}

Vec3 cachedMicroCenter(const SurfaceChunkData& c,const SurfaceMicroAddress& m) {
    Vec3 sum{};
    for(int du=0;du<=1;++du) for(int dr=0;dr<=1;++dr) for(int dv=0;dv<=1;++dv)
        sum=sum+cachedMicroBoundary(c,m.cell,m.u+du,m.radial+dr,m.v+dv);
    return sum*(1.0f/8.0f);
}

std::array<Vec3,4> cachedMacroCorners(const SurfaceChunkData& c,SurfaceCellAddress a,CellFace face) {
    const int u=a.u,v=a.v,r=a.radial;
    switch(face) {
        case CellFace::RPos: return {cachedBoundaryPosition(c,a.face,u,v,r+1),cachedBoundaryPosition(c,a.face,u+1,v,r+1),cachedBoundaryPosition(c,a.face,u+1,v+1,r+1),cachedBoundaryPosition(c,a.face,u,v+1,r+1)};
        case CellFace::RNeg: return {cachedBoundaryPosition(c,a.face,u,v,r),cachedBoundaryPosition(c,a.face,u,v+1,r),cachedBoundaryPosition(c,a.face,u+1,v+1,r),cachedBoundaryPosition(c,a.face,u+1,v,r)};
        case CellFace::UPos: return {cachedBoundaryPosition(c,a.face,u+1,v,r),cachedBoundaryPosition(c,a.face,u+1,v,r+1),cachedBoundaryPosition(c,a.face,u+1,v+1,r+1),cachedBoundaryPosition(c,a.face,u+1,v+1,r)};
        case CellFace::UNeg: return {cachedBoundaryPosition(c,a.face,u,v,r),cachedBoundaryPosition(c,a.face,u,v+1,r),cachedBoundaryPosition(c,a.face,u,v+1,r+1),cachedBoundaryPosition(c,a.face,u,v,r+1)};
        case CellFace::VPos: return {cachedBoundaryPosition(c,a.face,u,v+1,r),cachedBoundaryPosition(c,a.face,u+1,v+1,r),cachedBoundaryPosition(c,a.face,u+1,v+1,r+1),cachedBoundaryPosition(c,a.face,u,v+1,r+1)};
        case CellFace::VNeg: return {cachedBoundaryPosition(c,a.face,u,v,r),cachedBoundaryPosition(c,a.face,u,v,r+1),cachedBoundaryPosition(c,a.face,u+1,v,r+1),cachedBoundaryPosition(c,a.face,u+1,v,r)};
    }
    return {};
}

std::array<Vec3,4> cachedMicroCorners(const SurfaceChunkData& c,const SurfaceMicroAddress& m,CellFace face) {
    const int u=m.u,v=m.v,r=m.radial;
    switch(face) {
        case CellFace::RPos: return {cachedMicroBoundary(c,m.cell,u,r+1,v),cachedMicroBoundary(c,m.cell,u+1,r+1,v),cachedMicroBoundary(c,m.cell,u+1,r+1,v+1),cachedMicroBoundary(c,m.cell,u,r+1,v+1)};
        case CellFace::RNeg: return {cachedMicroBoundary(c,m.cell,u,r,v),cachedMicroBoundary(c,m.cell,u,r,v+1),cachedMicroBoundary(c,m.cell,u+1,r,v+1),cachedMicroBoundary(c,m.cell,u+1,r,v)};
        case CellFace::UPos: return {cachedMicroBoundary(c,m.cell,u+1,r,v),cachedMicroBoundary(c,m.cell,u+1,r+1,v),cachedMicroBoundary(c,m.cell,u+1,r+1,v+1),cachedMicroBoundary(c,m.cell,u+1,r,v+1)};
        case CellFace::UNeg: return {cachedMicroBoundary(c,m.cell,u,r,v),cachedMicroBoundary(c,m.cell,u,r,v+1),cachedMicroBoundary(c,m.cell,u,r+1,v+1),cachedMicroBoundary(c,m.cell,u,r+1,v)};
        case CellFace::VPos: return {cachedMicroBoundary(c,m.cell,u,r,v+1),cachedMicroBoundary(c,m.cell,u+1,r,v+1),cachedMicroBoundary(c,m.cell,u+1,r+1,v+1),cachedMicroBoundary(c,m.cell,u,r+1,v+1)};
        case CellFace::VNeg: return {cachedMicroBoundary(c,m.cell,u,r,v),cachedMicroBoundary(c,m.cell,u,r+1,v),cachedMicroBoundary(c,m.cell,u+1,r+1,v),cachedMicroBoundary(c,m.cell,u+1,r,v)};
    }
    return {};
}

void faceDelta(CellFace face,int& du,int& dv,int& dr) {
    du=dv=dr=0;
    switch(face) {
        case CellFace::UPos: du=1; break;
        case CellFace::UNeg: du=-1; break;
        case CellFace::VPos: dv=1; break;
        case CellFace::VNeg: dv=-1; break;
        case CellFace::RPos: dr=1; break;
        case CellFace::RNeg: dr=-1; break;
    }
}

BlockType cachedBoundaryMicro(const SurfaceChunkData& c,int lu,int lv,int lr,
                              int mu,int mr,int mv,CellFace face) {
    int nmu=mu,nmr=mr,nmv=mv;
    int du=0,dv=0,dr=0;
    switch(face) {
        case CellFace::UPos: ++nmu; if(nmu>=MicroBrick::Resolution){nmu=0;du=1;} break;
        case CellFace::UNeg: --nmu; if(nmu<0){nmu=MicroBrick::Resolution-1;du=-1;} break;
        case CellFace::VPos: ++nmv; if(nmv>=MicroBrick::Resolution){nmv=0;dv=1;} break;
        case CellFace::VNeg: --nmv; if(nmv<0){nmv=MicroBrick::Resolution-1;dv=-1;} break;
        case CellFace::RPos: ++nmr; if(nmr>=MicroBrick::Resolution){nmr=0;dr=1;} break;
        case CellFace::RNeg: --nmr; if(nmr<0){nmr=MicroBrick::Resolution-1;dr=-1;} break;
    }
    if(du==0&&dv==0&&dr==0) {
        const auto a=cachedWorldAddress(c,lu,lv,lr);
        if(const auto* b=c.microBrick(a)) return b->get(nmu,nmr,nmv);
        return c.getWithHalo(lu,lv,lr);
    }
    const auto na=cachedWorldAddress(c,lu+du,lv+dv,lr+dr);
    if(const auto* b=c.microBrick(na)) return b->get(nmu,nmr,nmv);
    return c.getWithHalo(lu+du,lv+dv,lr+dr);
}

struct GridDelta { int u{}; int v{}; int r{}; };

std::pair<GridDelta,GridDelta> faceTangents(CellFace face) {
    switch(face) {
        case CellFace::RPos: case CellFace::RNeg: return {{1,0,0},{0,1,0}};
        case CellFace::UPos: case CellFace::UNeg: return {{0,0,1},{0,1,0}};
        case CellFace::VPos: case CellFace::VNeg: return {{1,0,0},{0,0,1}};
    }
    return {};
}

std::array<std::pair<int,int>,4> faceCornerSigns(CellFace face) {
    constexpr std::array<std::pair<int,int>,4> positive{{{-1,-1},{1,-1},{1,1},{-1,1}}};
    constexpr std::array<std::pair<int,int>,4> negative{{{-1,-1},{-1,1},{1,1},{1,-1}}};
    switch(face) {
        case CellFace::RPos: case CellFace::UPos: case CellFace::VPos: return positive;
        case CellFace::RNeg: case CellFace::UNeg: case CellFace::VNeg: return negative;
    }
    return positive;
}

float aoFromOccupancy(int occupied) {
    constexpr std::array<float,4> factors{1.0f,0.86f,0.72f,0.58f};
    return factors[static_cast<std::size_t>(std::clamp(occupied,0,3))];
}

std::array<float,4> cachedMacroAo(const SurfaceChunkData& c,int lu,int lv,int lr,CellFace face) {
    int nu{},nv{},nr{};
    faceDelta(face,nu,nv,nr);
    const auto [ta,tb]=faceTangents(face);
    const auto signs=faceCornerSigns(face);
    std::array<float,4> out{};
    auto solid=[&](int u,int v,int r) { return blockProperties(c.getWithHalo(u,v,r)).solid; };
    for(std::size_t i=0;i<out.size();++i) {
        const auto [sa,sb]=signs[i];
        const int bu=lu+nu,bv=lv+nv,br=lr+nr;
        int occupied=0;
        occupied+=solid(bu+ta.u*sa,bv+ta.v*sa,br+ta.r*sa)?1:0;
        occupied+=solid(bu+tb.u*sb,bv+tb.v*sb,br+tb.r*sb)?1:0;
        occupied+=solid(bu+ta.u*sa+tb.u*sb,bv+ta.v*sa+tb.v*sb,br+ta.r*sa+tb.r*sb)?1:0;
        out[i]=aoFromOccupancy(occupied);
    }
    return out;
}

BlockType cachedMicroOffset(const SurfaceChunkData& c,int lu,int lv,int lr,
                            int mu,int mr,int mv,int du,int dr,int dv) {
    constexpr int N=MicroBrick::Resolution;
    mu+=du; mr+=dr; mv+=dv;
    while(mu<0){mu+=N;--lu;} while(mu>=N){mu-=N;++lu;}
    while(mv<0){mv+=N;--lv;} while(mv>=N){mv-=N;++lv;}
    while(mr<0){mr+=N;--lr;} while(mr>=N){mr-=N;++lr;}
    const auto a=cachedWorldAddress(c,lu,lv,lr);
    if(const auto* b=c.microBrick(a)) return b->get(mu,mr,mv);
    return c.getWithHalo(lu,lv,lr);
}

std::array<float,4> cachedMicroAo(const SurfaceChunkData& c,int lu,int lv,int lr,
                                  int mu,int mr,int mv,CellFace face) {
    int nu{},nv{},nr{};
    faceDelta(face,nu,nv,nr);
    const auto [ta,tb]=faceTangents(face);
    const auto signs=faceCornerSigns(face);
    std::array<float,4> out{};
    auto solid=[&](int du,int dv,int dr) {
        return blockProperties(cachedMicroOffset(c,lu,lv,lr,mu,mr,mv,du,dr,dv)).solid;
    };
    for(std::size_t i=0;i<out.size();++i) {
        const auto [sa,sb]=signs[i];
        int occupied=0;
        occupied+=solid(nu+ta.u*sa,nv+ta.v*sa,nr+ta.r*sa)?1:0;
        occupied+=solid(nu+tb.u*sb,nv+tb.v*sb,nr+tb.r*sb)?1:0;
        occupied+=solid(nu+ta.u*sa+tb.u*sb,nv+ta.v*sa+tb.v*sb,nr+ta.r*sa+tb.r*sb)?1:0;
        out[i]=aoFromOccupancy(occupied);
    }
    return out;
}

void emitCachedMacroFace(Builder& b,const SurfaceChunkData& c,int lu,int lv,int lr,
                         SurfaceCellAddress a,CellFace face,BlockType type) {
    const auto q=cachedMacroCorners(c,a,face);
    const Vec3 center=cachedCellCenter(c,a);
    b.emitAo(type,q[0],q[1],q[2],q[3],normalize(avg4(q[0],q[1],q[2],q[3])-center),
             faceLight(face),false,cachedMacroAo(c,lu,lv,lr,face));
}

void emitCachedTiledBoundary(Builder& b,const SurfaceChunkData& c,int lu,int lv,int lr,
                             SurfaceCellAddress a,CellFace face,BlockType type) {
    constexpr int N=MicroBrick::Resolution;
    for(int i=0;i<N;++i) for(int j=0;j<N;++j) {
        int mu=0,mr=0,mv=0;
        switch(face) {
            case CellFace::RPos: mu=i;mr=N-1;mv=j; break;
            case CellFace::RNeg: mu=i;mr=0;mv=j; break;
            case CellFace::UPos: mu=N-1;mr=i;mv=j; break;
            case CellFace::UNeg: mu=0;mr=i;mv=j; break;
            case CellFace::VPos: mu=i;mr=j;mv=N-1; break;
            case CellFace::VNeg: mu=i;mr=j;mv=0; break;
        }
        if(blockProperties(cachedBoundaryMicro(c,lu,lv,lr,mu,mr,mv,face)).solid) continue;
        SurfaceMicroAddress m{a,mu,mr,mv};
        const auto q=cachedMicroCorners(c,m,face);
        const Vec3 mc=cachedMicroCenter(c,m);
        b.emitAo(type,q[0],q[1],q[2],q[3],normalize(avg4(q[0],q[1],q[2],q[3])-mc),
                 faceLight(face),true,cachedMicroAo(c,lu,lv,lr,mu,mr,mv,face));
    }
}

void emitCachedRefined(Builder& b,const SurfaceChunkData& c,int lu,int lv,int lr,SurfaceCellAddress a) {
    const auto* brick=c.microBrick(a);
    if(!brick) return;
    constexpr std::array<CellFace,6> faces{CellFace::UPos,CellFace::UNeg,CellFace::VPos,CellFace::VNeg,CellFace::RPos,CellFace::RNeg};
    for(int mr=0;mr<MicroBrick::Resolution;++mr) for(int mv=0;mv<MicroBrick::Resolution;++mv) for(int mu=0;mu<MicroBrick::Resolution;++mu) {
        const BlockType type=brick->get(mu,mr,mv);
        if(!blockProperties(type).solid) continue;
        SurfaceMicroAddress m{a,mu,mr,mv};
        const Vec3 mc=cachedMicroCenter(c,m);
        for(const CellFace face:faces) {
            if(blockProperties(cachedBoundaryMicro(c,lu,lv,lr,mu,mr,mv,face)).solid) continue;
            const auto q=cachedMicroCorners(c,m,face);
            b.emitAo(type,q[0],q[1],q[2],q[3],normalize(avg4(q[0],q[1],q[2],q[3])-mc),
                     faceLight(face),true,cachedMicroAo(c,lu,lv,lr,mu,mr,mv,face));
        }
    }
}

} // namespace

CpuMeshData buildPlanetSurfaceChunkMesh(const SurfaceChunkData& chunk) {
    Builder builder;
    if(chunk.address.radial<0 || chunk.address.radial>=PlanetSurface::RadialChunks ||
       chunk.address.u<0 || chunk.address.u>=PlanetSurface::ChunksPerFaceAxis ||
       chunk.address.v<0 || chunk.address.v>=PlanetSurface::ChunksPerFaceAxis) return builder.finish();

    constexpr std::array<CellFace,6> faces{CellFace::UPos,CellFace::UNeg,CellFace::VPos,CellFace::VNeg,CellFace::RPos,CellFace::RNeg};
    for(int lv=0;lv<PlanetSurface::ChunkSize;++lv) for(int lu=0;lu<PlanetSurface::ChunkSize;++lu) for(int lr=0;lr<PlanetSurface::ChunkSize;++lr) {
        const SurfaceCellAddress a=cachedWorldAddress(chunk,lu,lv,lr);
        const BlockType type=chunk.getLocal(lu,lv,lr);
        const bool refined=chunk.isRefined(a);
        if(!blockProperties(type).solid && !refined) continue;
        if(refined) {
            emitCachedRefined(builder,chunk,lu,lv,lr,a);
            continue;
        }
        if(!blockProperties(type).solid) continue;
        for(const CellFace face:faces) {
            int du=0,dv=0,dr=0; faceDelta(face,du,dv,dr);
            const auto neighbor=cachedWorldAddress(chunk,lu+du,lv+dv,lr+dr);
            if(chunk.isRefined(neighbor)) emitCachedTiledBoundary(builder,chunk,lu,lv,lr,a,face,type);
            else if(!blockProperties(chunk.getWithHalo(lu+du,lv+dv,lr+dr)).solid) emitCachedMacroFace(builder,chunk,lu,lv,lr,a,face,type);
        }
    }
    return builder.finish();
}


CpuMeshData buildPlanetSurfaceFieldChunkMesh(const PlanetSurfaceSnapshot& planet,
                                             const PlanetChunkAddress& chunk,
                                             int step,
                                             bool addTransitionSkirts,
                                             float skirtDepth,
                                             int influencedStep) {
    Builder builder;
    if (chunk.radial != 0 || chunk.u < 0 || chunk.u >= PlanetSurface::ChunksPerFaceAxis ||
        chunk.v < 0 || chunk.v >= PlanetSurface::ChunksPerFaceAxis) return builder.finish();

    step = std::clamp(step,1,PlanetSurface::ChunkSize);
    const int u0 = chunk.u * PlanetSurface::ChunkSize;
    const int v0 = chunk.v * PlanetSurface::ChunkSize;
    const int u1 = std::min(u0 + PlanetSurface::ChunkSize, PlanetSurfaceSnapshot::FaceResolution);
    const int v1 = std::min(v0 + PlanetSurface::ChunkSize, PlanetSurfaceSnapshot::FaceResolution);

    auto surfacePoint = [&](int uEdge,int vEdge) {
        const Vec3 d = faceGridCornerDirection(chunk.face,uEdge,vEdge,PlanetSurfaceSnapshot::FaceResolution);
        const FaceUv owner = directionToFaceUv(d);
        const float fu = (owner.u + 1.0f) * 0.5f * static_cast<float>(PlanetSurfaceSnapshot::FaceResolution);
        const float fv = (owner.v + 1.0f) * 0.5f * static_cast<float>(PlanetSurfaceSnapshot::FaceResolution);
        const int u = std::clamp(static_cast<int>(std::floor(fu)),0,PlanetSurfaceSnapshot::FaceResolution-1);
        const int v = std::clamp(static_cast<int>(std::floor(fv)),0,PlanetSurfaceSnapshot::FaceResolution-1);
        const int r = std::max(0,planet.surfaceRadial(owner.face,u,v));
        const float radius = planet.referenceRadius + static_cast<float>(r + 1 - PlanetSurfaceSnapshot::ReferenceRadial);
        return d * radius;
    };

    influencedStep=std::clamp(influencedStep,1,step);
    const auto chunkInfluence=planet.editInfluence(chunk);
    const int significantInfluencedStep=(chunkInfluence.significanceScore()>=24)?1:influencedStep;
    auto emitPatch=[&](int u,int v,int ue,int ve) {
        const int su = std::min(u + (ue-u)/2, PlanetSurfaceSnapshot::FaceResolution-1);
        const int sv = std::min(v + (ve-v)/2, PlanetSurfaceSnapshot::FaceResolution-1);
        const int sr = planet.surfaceRadial(chunk.face,su,sv);
        if (sr < 0) return;
        BlockType material = planet.get(chunk.face,su,sv,sr);
        if (!blockProperties(material).solid) material = BlockType::Stone;

        Vec3 p0=surfacePoint(u,v);
        Vec3 p1=surfacePoint(ue,v);
        Vec3 p2=surfacePoint(ue,ve);
        Vec3 p3=surfacePoint(u,ve);
        const Vec3 desired = normalize(avg4(p0,p1,p2,p3));
        builder.emit(material,p0,p1,p2,p3,desired,1.0f,false);
    };

    for (int v=v0; v<v1; v+=step) {
        const int ve = std::min(v+step,v1);
        for (int u=u0; u<u1; u+=step) {
            const int ue = std::min(u+step,u1);
            // Player edits refine only the affected coarse field tile instead
            // of promoting an entire 32x32 chunk. This preserves towers, pits,
            // and other authored silhouette changes while keeping unrelated
            // far-field area at the cheaper sampling rate.
            if(step>significantInfluencedStep && planet.hasEditInfluence(chunk.face,u,v,ue,ve)) {
                for(int sv=v;sv<ve;sv+=significantInfluencedStep) {
                    const int sve=std::min(sv+significantInfluencedStep,ve);
                    for(int su=u;su<ue;su+=significantInfluencedStep)
                        emitPatch(su,sv,std::min(su+significantInfluencedStep,ue),sve);
                }
            } else {
                emitPatch(u,v,ue,ve);
            }
        }
    }

    // Conservative transition skirts hide cracks between independently sampled
    // field tiers and between a field proxy and an adjacent full-detail chunk.
    // They deliberately trade a little overdraw for topology continuity.
    if (addTransitionSkirts && skirtDepth>0.0f) {
        auto emitSkirt=[&](Vec3 a,Vec3 b) {
            const Vec3 ai=a-normalize(a)*skirtDepth;
            const Vec3 bi=b-normalize(b)*skirtDepth;
            const Vec3 desired=normalize(cross(b-a,ai-a));
            builder.emit(BlockType::Stone,a,b,bi,ai,desired,0.68f,false);
        };
        for (int u=u0;u<u1;u+=step) {
            const int ue=std::min(u+step,u1);
            emitSkirt(surfacePoint(u,v0),surfacePoint(ue,v0));
            emitSkirt(surfacePoint(ue,v1),surfacePoint(u,v1));
        }
        for (int v=v0;v<v1;v+=step) {
            const int ve=std::min(v+step,v1);
            emitSkirt(surfacePoint(u0,ve),surfacePoint(u0,v));
            emitSkirt(surfacePoint(u1,v),surfacePoint(u1,ve));
        }
    }
    return builder.finish();
}


CpuMeshData buildPlanetOrbitalClimateShellMesh(const PlanetSurfaceSnapshot& planet,
                                               int step,
                                               float shellOffset) {
    Builder builder;
    step=std::clamp(step,1,PlanetSurfaceSnapshot::FaceResolution);
    for (int f=0;f<PlanetSurfaceSnapshot::FaceCount;++f) {
        const auto face=static_cast<CubeFace>(f);
        auto point=[&](int uEdge,int vEdge) {
            const Vec3 d=faceGridCornerDirection(face,uEdge,vEdge,PlanetSurfaceSnapshot::FaceResolution);
            const FaceUv owner=directionToFaceUv(d);
            const float fu=(owner.u+1.0f)*0.5f*PlanetSurfaceSnapshot::FaceResolution;
            const float fv=(owner.v+1.0f)*0.5f*PlanetSurfaceSnapshot::FaceResolution;
            const int u=std::clamp(static_cast<int>(std::floor(fu)),0,PlanetSurfaceSnapshot::FaceResolution-1);
            const int v=std::clamp(static_cast<int>(std::floor(fv)),0,PlanetSurfaceSnapshot::FaceResolution-1);
            const int r=generatedSurfaceAt(planet,owner.face,u,v);
            const float radius=planet.referenceRadius + static_cast<float>(r+1-PlanetSurfaceSnapshot::ReferenceRadial) + shellOffset;
            return d*radius;
        };
        for (int v=0;v<PlanetSurfaceSnapshot::FaceResolution;v+=step) {
            const int ve=std::min(v+step,PlanetSurfaceSnapshot::FaceResolution);
            for (int u=0;u<PlanetSurfaceSnapshot::FaceResolution;u+=step) {
                const int ue=std::min(u+step,PlanetSurfaceSnapshot::FaceResolution);
                const int su=std::min(u+(ue-u)/2,PlanetSurfaceSnapshot::FaceResolution-1);
                const int sv=std::min(v+(ve-v)/2,PlanetSurfaceSnapshot::FaceResolution-1);
                const int sr=generatedSurfaceAt(planet,face,su,sv);
                const Color4u c=climateColor(planet,face,su,sv,sr);
                Vec3 p0=point(u,v),p1=point(ue,v),p2=point(ue,ve),p3=point(u,ve);
                builder.emitColored(BlockType::Stone,c,p0,p1,p2,p3,normalize(avg4(p0,p1,p2,p3)));
            }
        }
    }
    return builder.finish();
}

CpuMeshData buildPlanetOrbitalCloudShellMesh(const PlanetSurfaceSnapshot& planet,
                                             int step,
                                             float cloudAltitude) {
    Builder builder;
    step=std::clamp(step,2,PlanetSurfaceSnapshot::FaceResolution);
    const float radius=planet.referenceRadius + cloudAltitude;
    for (int f=0;f<PlanetSurfaceSnapshot::FaceCount;++f) {
        const auto face=static_cast<CubeFace>(f);
        for (int v=0;v<PlanetSurfaceSnapshot::FaceResolution;v+=step) {
            const int ve=std::min(v+step,PlanetSurfaceSnapshot::FaceResolution);
            for (int u=0;u<PlanetSurfaceSnapshot::FaceResolution;u+=step) {
                const int ue=std::min(u+step,PlanetSurfaceSnapshot::FaceResolution);
                const float n=hash01(planet.seed,u/step,v/step,f,0xC10D5ULL);
                const float threshold=planet.planetClass==PlanetClass::Barren?0.92f:0.58f;
                if (n<threshold) continue;
                auto q=[&](int x,int y){return faceGridCornerDirection(face,x,y,PlanetSurfaceSnapshot::FaceResolution)*radius;};
                Vec3 p0=q(u,v),p1=q(ue,v),p2=q(ue,ve),p3=q(u,ve);
                const std::uint8_t alpha=static_cast<std::uint8_t>(std::clamp(90 + static_cast<int>((n-threshold)*280.0f),90,190));
                Color4u c=planet.planetClass==PlanetClass::Scorched?Color4u{174,133,119,alpha}:Color4u{215,224,226,alpha};
                builder.emitColored(BlockType::Stone,c,p0,p1,p2,p3,normalize(avg4(p0,p1,p2,p3)));
            }
        }
    }
    return builder.finish();
}

CpuMeshData buildPlanetSurfaceMesh(const PlanetSurfaceSnapshot& planet) {
    CpuMeshData combined{};
    for(int f=0;f<PlanetSurfaceSnapshot::FaceCount;++f) {
        for(int cv=0;cv<PlanetSurface::ChunksPerFaceAxis;++cv) {
            for(int cu=0;cu<PlanetSurface::ChunksPerFaceAxis;++cu) {
                CpuMeshData part = buildPlanetSurfaceChunkMesh(planet,{static_cast<CubeFace>(f),cu,cv,0});
                const int baseVertex = combined.vertexCount();
                combined.vertices.insert(combined.vertices.end(),part.vertices.begin(),part.vertices.end());
                combined.normals.insert(combined.normals.end(),part.normals.begin(),part.normals.end());
                combined.colors.insert(combined.colors.end(),part.colors.begin(),part.colors.end());
                for (auto range : part.materialRanges) {
                    range.firstVertex += baseVertex;
                    combined.materialRanges.push_back(range);
                }
                combined.quads += part.quads;
                combined.macroQuads += part.macroQuads;
                combined.microQuads += part.microQuads;
                combined.aoDarkenedCorners += part.aoDarkenedCorners;
            }
        }
    }
    return combined;
}

} // namespace elysium
