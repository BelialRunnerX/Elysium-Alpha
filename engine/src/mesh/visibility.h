// Which voxel faces can actually be seen.
//
// The greedy mesher already emits only faces where a solid voxel meets a
// non-solid one, which throws away the interior of the planet — the great
// majority of all faces. Two further classes of face are provably invisible
// and are removed here and in the mesher's options:
//
//   1. Faces bounding *sealed* cavities. A pocket of air with no path to the
//      outside of the region cannot be looked into. On this generator that is
//      not a rare case: the cave system is a noise mask, and it leaves
//      thousands of isolated bubbles per region that no player will ever
//      reach. Their walls are fully meshed by a naive mesher and never drawn.
//
//   2. Faces pointing away from the camera. Six axis-aligned normals means
//      backface culling is three integer comparisons for a whole sweep, not a
//      per-triangle cross product, and it removes about half of what is left.
//
// This class answers (1) with a flood fill through non-solid voxels from the
// region boundary. Anything it does not reach is sealed.
//
// The caveat, stated plainly because it is the kind of thing that becomes a
// bug six months later: connectivity is computed *per region*. A cavity that
// is sealed within this region but opens into the next one would be culled
// wrongly, and the player would see a hole. The rule that makes it safe is
// that out-of-region neighbours count as reachable, so every cavity touching
// the region boundary is kept. Meshing a region with a one-voxel apron beyond
// the chunk being drawn therefore gives exactly correct results at chunk
// edges; a streaming implementation that wants to cull across whole chunks
// needs a connectivity summary per chunk face instead, which is a bigger
// mechanism than this proof needs.
#pragma once
#include "greedy.h"
#include <cstdint>
#include <vector>

namespace ely {

class VisibilityMask {
public:
    VisibilityMask(int sx, int sy, int sz, const VoxelSampler& sample);

    // True if this voxel is non-solid and connected to the outside.
    //
    // Out-of-range answers true: the world continues past the region, so a
    // face on the region boundary must always be emitted.
    bool exteriorAir(int x, int y, int z) const {
        if (x < 0 || y < 0 || z < 0 || x >= sx_ || y >= sy_ || z >= sz_) return true;
        return reachedBits_[size_t(idx(x, y, z))] != 0;
    }

    long airVoxels() const { return air_; }
    long reachedAir() const { return reached_; }
    long sealedAir() const { return air_ - reached_; }
    long solidVoxels() const { return solid_; }

private:
    int idx(int x, int y, int z) const { return (y * sz_ + z) * sx_ + x; }

    int sx_, sy_, sz_;
    long air_ = 0, reached_ = 0, solid_ = 0;
    std::vector<uint8_t> reachedBits_;
};

}  // namespace ely
