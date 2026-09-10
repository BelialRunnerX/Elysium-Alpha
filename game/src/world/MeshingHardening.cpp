#include "world/MeshingHardening.hpp"
#include <cmath>
namespace elysium::renderhardening {
namespace { std::uint64_t mix(std::uint64_t h,std::uint64_t v){ h^=v+0x9e3779b97f4a7c15ULL+(h<<6)+(h>>2); return h; } }
FaceDecision decideVisibleFace(const FaceQuery& q,const SampleFn& sample,MeshingCounters* c){
    FaceDecision out{}; if(c) ++c->facesConsidered;
    const auto self=sample(q.x,q.y,q.z); if(!self.known || !self.solid) return out;
    const auto n=sample(q.x+q.nx,q.y+q.ny,q.z+q.nz); out.material=self.material;
    if(!n.known){ out.emit=true; out.haloSample=true; if(c){++c->haloSamples;++c->facesEmitted;} return out; }
    if(n.solid) return out;
    if(n.refined){ out.refinedBoundary=true; if(c) ++c->refinedBoundaryFaces; return out; }
    if(!n.exteriorAir){ out.sealedCavityRejected=true; if(c) ++c->sealedCavityRejected; return out; }
    out.emit=true; if(c) ++c->facesEmitted; return out;
}
float cornerAo(const std::array<bool,4>& o,MeshingCounters* c) noexcept { int n=0; for(bool v:o) n+=v?1:0; if(c&&n>0) ++c->aoDarkenedCorners; return std::clamp(1.0f-0.11f*float(n),0.56f,1.0f); }
bool publicationStillCurrent(PublicationStamp s,std::uint64_t r,std::uint64_t g) noexcept { return s.sourceRevision==r && s.generation==g; }
std::uint64_t meshFingerprint(std::uint64_t seed,const MeshingCounters& c) noexcept { auto h=seed; h=mix(h,c.facesConsidered); h=mix(h,c.facesEmitted); h=mix(h,c.haloSamples); h=mix(h,c.sealedCavityRejected); h=mix(h,c.refinedBoundaryFaces); h=mix(h,c.aoDarkenedCorners); return h; }
}
