// The GPU vertex format.
//
// Eight bytes, down from the sixteen the mesher's own Vertex occupies, and the
// difference is not cosmetic: at a 512 m view distance this engine has tens of
// millions of vertices resident, so eight bytes against sixteen is gigabytes of
// bandwidth per second at 60 Hz. Three decisions get it there:
//
//   * Positions are unsigned 16-bit integers in *voxel units, chunk-local*, not
//     floats in world metres. A quad corner always lands exactly on a voxel
//     lattice point — the greedy mesher cannot produce anything else — so an
//     integer is not an approximation here, it is the exact value. The vertex
//     shader reconstructs world position as origin + p * voxelSize, with both
//     terms arriving in a push constant. This also fixes a precision problem
//     for free: a float world position 3 km from the planet centre has about
//     0.25 mm of resolution left, which is coarse against 62.5 mm micro-voxels
//     and would show as vertices visibly jittering at the far edge of a chunk.
//
//   * Colour is a material *index*, not an RGB triple. The palette lives in a
//     uniform buffer, so retexturing, biome tinting or a damage overlay is a
//     buffer write rather than a full remesh of everything on screen.
//
//   * AO and face axis share one byte. Both are tiny — two bits and three bits
//     — and neither is worth its own.
#pragma once
#include "../mesh/greedy.h"
#include <cstdint>
#include <vector>

namespace ely {

#pragma pack(push, 1)
struct GpuVertex {
    uint16_t x, y, z;    // corner position in voxel units, chunk-local
    uint8_t  material;   // index into the material palette
    uint8_t  aoAxis;     // bits 0-1: ao 0..3   bits 2-4: face axis 0..5

    static constexpr uint8_t pack(int ao, int axis) {
        return uint8_t((ao & 0x3) | ((axis & 0x7) << 2));
    }
    int ao() const { return aoAxis & 0x3; }
    int axis() const { return (aoAxis >> 2) & 0x7; }
};
#pragma pack(pop)

static_assert(sizeof(GpuVertex) == 8, "the vertex format must stay 8 bytes; "
              "the pipeline's attribute offsets and stride assume it");

// A chunk's geometry, ready to upload.
//
// Indices are 32-bit. A greedy-meshed micro-voxel chunk can exceed 65,535
// vertices — the ladder proof hits 28,304 quads, which is 113,216 vertices — so
// 16-bit indices would silently wrap and draw garbage. Measured rather than
// assumed, and the assert below is what keeps it honest if the chunk size
// changes.
struct GpuMesh {
    std::vector<GpuVertex> vertices;
    std::vector<uint32_t> indices;

    // Where this chunk sits in the world, and how big one of its voxels is.
    // These two go into a push constant and are the whole of the per-draw
    // state, which is why a draw call here needs no descriptor set rebind.
    float originX = 0, originY = 0, originZ = 0;
    float voxelSize = 1.0f;

    size_t triangles() const { return indices.size() / 3; }
    size_t vertexBytes() const { return vertices.size() * sizeof(GpuVertex); }
    size_t indexBytes() const { return indices.size() * sizeof(uint32_t); }
    bool empty() const { return indices.empty(); }
};

// Convert mesher output to GPU form.
//
// The mesher works in voxel units with the origin and scale applied at the end
// (see MeshOptions), so this undoes that: it takes the mesh back to integer
// lattice coordinates and moves the origin and scale into the draw's push
// constant, where the GPU applies them for free.
//
// `originX/Y/Z` and `voxelSize` must be the same values the mesh was built
// with. Passing different ones silently displaces the chunk, which is why they
// are taken from the MeshOptions that produced it rather than re-derived.
inline GpuMesh toGpuMesh(const Mesh& src, double originX, double originY,
                         double originZ, double voxelSize) {
    GpuMesh out;
    out.originX = float(originX);
    out.originY = float(originY);
    out.originZ = float(originZ);
    out.voxelSize = float(voxelSize);
    out.vertices.reserve(src.vertices.size());
    out.indices = src.indices;

    const double inv = 1.0 / voxelSize;
    for (const Vertex& v : src.vertices) {
        // + 0.5 then truncate, rather than a bare cast: the mesher's float
        // positions are exact multiples of voxelSize, but dividing them back
        // can land a hair under the integer, and truncating that gives a
        // corner one voxel out of place.
        const long lx = long((double(v.x) - originX) * inv + 0.5);
        const long ly = long((double(v.y) - originY) * inv + 0.5);
        const long lz = long((double(v.z) - originZ) * inv + 0.5);
        GpuVertex g;
        g.x = uint16_t(lx < 0 ? 0 : (lx > 65535 ? 65535 : lx));
        g.y = uint16_t(ly < 0 ? 0 : (ly > 65535 ? 65535 : ly));
        g.z = uint16_t(lz < 0 ? 0 : (lz > 65535 ? 65535 : lz));
        g.material = v.mat;
        g.aoAxis = GpuVertex::pack(v.ao, v.nx);
        out.vertices.push_back(g);
    }
    return out;
}

}  // namespace ely
