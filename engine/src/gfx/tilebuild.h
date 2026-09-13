// Turning a piece of world into something drawable.
//
// This is the bridge between the two halves of the engine. Below it, the
// generator answers questions about points and knows nothing about rendering;
// above it, the renderer draws buffers and knows nothing about planets. This is
// the one file that has to know both, and it is the work item the streamer's
// load queue produces: "build tile (x, y, z) at level n".
//
// The whole pipeline for one tile, in order:
//
//   generate (column-cached, with a one-voxel apron)
//     -> flood fill for sealed cavities
//     -> greedy mesh with per-vertex AO
//     -> pack to the 8-byte GPU vertex
//
// The apron is the part that is easy to leave out and expensive to debug. A
// tile meshed in isolation treats everything outside itself as air and emits a
// wall on all six sides; on a tiled landscape that is a solid seam standing
// along every tile boundary, plus two coincident walls z-fighting between each
// pair of neighbours. One voxel of real generated world beyond each face is the
// entire fix.
#pragma once
#include "../jobs/jobsystem.h"
#include "../save/editstore.h"
#include "../mesh/greedy.h"
#include "../mesh/visibility.h"
#include "../world/lod.h"
#include "../world/terrain.h"
#include "frustum.h"
#include "vertex.h"

#include <vector>

namespace ely {

// Where a tile sits, in the planet's face-local frame. Tiles are cubes of world
// measured in metres; `lod` decides how many voxels that cube is divided into.
struct TileSpec {
    Face face = FACE_PZ;
    double uCentre = 0.0, vCentre = 0.0;  // the face coordinate the frame is
                                          // built around
    double ox = 0.0, oy = 0.0, oz = 0.0;  // tile corner, metres from that centre
    double sizeX = 32.0, sizeY = 32.0, sizeZ = 32.0;   // tile extent, metres
    int lod = 0;

    // The player's edits, applied after generation. Null means the pristine
    // generated world, which is what every tool that is not demonstrating saves
    // wants.
    const EditStore* edits = nullptr;

    // Clip the tile to a scene boundary. Inside a scene a tile joins its
    // neighbour through the apron; at the scene's own outer edge there is no
    // neighbour, so the apron must be suppressed or the viewer looks straight
    // into the inside of the ground through a wall that was never drawn.
    bool clipToScene = false;
    double sceneMinX = 0, sceneMaxX = 0;
    double sceneMinY = 0, sceneMaxY = 0;
    double sceneMinZ = 0, sceneMaxZ = 0;
};

struct BuiltTile {
    GpuMesh gpu;
    Aabb bounds;
    int lod = 0;
    long voxelsGenerated = 0;
    MeshStats mesh;
    double generateMs = 0, meshMs = 0;
    bool skippedAsSky = false;   // proved empty without generating it

    bool empty() const { return gpu.empty(); }
};

// Build one tile. Pure with respect to the terrain: the same spec always gives
// the same mesh, which is what lets tiles be built on worker threads without
// any coordination beyond handing the result back.
//
// `jobs` optionally parallelises the column walk *within* this tile. It is
// worth passing when a caller is building one tile and waiting for it — a tool,
// or the first tile under a player who has just spawned — and it costs nothing
// when the caller is itself a worker, because parallelFor detects that and runs
// serially rather than having every worker try to occupy every other worker.
BuiltTile buildTile(const Terrain& terrain, const TileSpec& spec,
                    JobSystem* jobs = nullptr);

}  // namespace ely
