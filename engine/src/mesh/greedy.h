// Greedy meshing with per-vertex ambient occlusion. Specification §12.2.
//
// Merging coplanar faces of the same material into the largest possible quads
// cuts triangle count by roughly an order of magnitude on typical terrain.
// At this scale that is not an optimisation, it is the difference between
// shipping and not.
//
// Ambient occlusion is baked per vertex from the three voxels touching each
// corner. It costs one byte per vertex and does more for readability than any
// lighting system — which is why it is here in the mesher rather than left to
// a renderer that does not exist yet.
#pragma once
#include "../world/material.h"
#include <cstdint>
#include <functional>
#include <vector>

namespace ely {

struct Vertex {
    float x, y, z;
    uint8_t mat;       // Material id, not a colour
    uint8_t ao;        // 0 fully occluded .. 3 open
    uint8_t nx;        // face axis 0..5, for shading
};

// A vertex carries the material id rather than a baked RGB triple. Three
// reasons, in order of how much they matter:
//
//   * The GPU wants an index anyway (see gfx/vertex.h): the palette lives in a
//     uniform buffer so that retexturing or biome tinting is a buffer write
//     rather than remeshing everything on screen.
//   * Baking the colour loses information. Recovering "which material is this"
//     from an RGB triple means a reverse lookup that two materials with the
//     same colour would break.
//   * It is smaller: 16 bytes instead of 20 once the compiler has padded.
//
// Consumers call materialInfo(Material(v.mat)) for the colour.

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    size_t quads() const { return indices.size() / 6; }
};

// Sample a voxel in a region, in region-local coordinates. Callers supply a
// sampler that reads one voxel past each boundary so that faces at a chunk
// edge are meshed correctly rather than sealed.
using VoxelSampler = std::function<Material(int x, int y, int z)>;

class VisibilityMask;

// How to mesh a region.
//
// voxelSize and the origin exist so that one mesher serves every level of the
// resolution ladder. The mesher counts in voxels; the mesh comes out in
// metres. At lod -4 voxelSize is 1/16 and a region of 128 voxels is 8 m of
// world; at lod +2 voxelSize is 4 and the same 128 voxels are 512 m. Nothing
// in the algorithm changes, which is the point — micro-voxels and distant
// terrain are the same code path at different scales.
struct MeshOptions {
    double voxelSize = 1.0;
    double ox = 0.0, oy = 0.0, oz = 0.0;   // world position of voxel (0,0,0)

    // Drop faces that bound a sealed cavity (see visibility.h). Optional: pass
    // null and every exposed face is emitted.
    const VisibilityMask* visibility = nullptr;

    // Drop faces whose normal points away from the viewer. Exact for an
    // orthographic camera; for a perspective camera it is exact whenever the
    // region lies entirely in front of the camera along that axis, which is
    // the case for anything past a chunk's distance and is why LOD regions can
    // use it and the chunk under the player's feet should not.
    bool cullBackFaces = false;
    double viewX = 0.0, viewY = 0.0, viewZ = 1.0;   // direction of gaze
};

// What the culling actually removed. Reported rather than assumed: an
// optimisation whose effect is not measured is a claim, not a result (E7).
struct MeshStats {
    long solidVoxels = 0;
    long exposedFaces = 0;      // solid meets non-solid: what a naive mesher emits
    long sealedCulled = 0;      // dropped: the air side is a sealed pocket
    long backfaceCulled = 0;    // dropped: normal points away from the camera
    long emittedFaces = 0;      // survived culling, before merging
    long quads = 0;             // after greedy merging
    double faceReduction() const {
        return quads ? double(exposedFaces) / double(quads) : 0.0;
    }
};

// Mesh an axis-aligned region [0,sx) x [0,sy) x [0,sz).
Mesh greedyMesh(int sx, int sy, int sz, const VoxelSampler& sample,
                const MeshOptions& opts = MeshOptions{},
                MeshStats* stats = nullptr);

}  // namespace ely
