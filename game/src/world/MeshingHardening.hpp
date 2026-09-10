#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <limits>

namespace elysium::renderhardening {
using MaterialId = std::uint64_t;
struct CellSample { bool known{}; bool solid{}; bool refined{}; bool exteriorAir{}; MaterialId material{}; };
using SampleFn = std::function<CellSample(std::int64_t,std::int64_t,std::int64_t)>;
struct FaceQuery { std::int64_t x{},y{},z{}; int nx{},ny{},nz{}; };
struct FaceDecision { bool emit{}; bool haloSample{}; bool sealedCavityRejected{}; bool refinedBoundary{}; MaterialId material{}; };
struct MeshingCounters { std::uint64_t facesConsidered{}, facesEmitted{}, haloSamples{}, sealedCavityRejected{}, refinedBoundaryFaces{}, aoDarkenedCorners{}; };
struct PublicationStamp { std::uint64_t sourceRevision{}, generation{}, fingerprint{}; };
FaceDecision decideVisibleFace(const FaceQuery&, const SampleFn&, MeshingCounters* = nullptr);
float cornerAo(const std::array<bool,4>& occupied, MeshingCounters* = nullptr) noexcept;
bool publicationStillCurrent(PublicationStamp candidate, std::uint64_t sourceRevision, std::uint64_t generation) noexcept;
std::uint64_t meshFingerprint(std::uint64_t seed, const MeshingCounters&) noexcept;
}
