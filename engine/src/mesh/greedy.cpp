#include "greedy.h"
#include "visibility.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace ely {

namespace {

// A meshed face: which material, and the four corner AO values. Two faces
// merge only if all of that matches, which is what keeps AO from being
// smeared across a merged quad.
struct FaceKey {
    Material mat = MAT_AIR;
    uint8_t ao[4] = {3, 3, 3, 3};
    bool operator==(const FaceKey& o) const {
        return mat == o.mat && ao[0] == o.ao[0] && ao[1] == o.ao[1]
            && ao[2] == o.ao[2] && ao[3] == o.ao[3];
    }
    explicit operator bool() const { return mat != MAT_AIR; }
};

// Standard three-sample AO: two edge neighbours and the corner. The special
// case matters — when both edges are solid the corner is fully dark whatever
// the corner voxel is, because light cannot reach it.
inline uint8_t aoValue(bool side1, bool side2, bool corner) {
    if (side1 && side2) return 0;
    return uint8_t(3 - (int(side1) + int(side2) + int(corner)));
}

}  // namespace

Mesh greedyMesh(int sx, int sy, int sz, const VoxelSampler& sample,
                const MeshOptions& opts, MeshStats* stats) {
    Mesh mesh;
    MeshStats local;
    MeshStats& st = stats ? *stats : local;
    const int dims[3] = {sx, sy, sz};

    auto solidAt = [&](int x, int y, int z) {
        return isSolid(sample(x, y, z));
    };

    const double view[3] = {opts.viewX, opts.viewY, opts.viewZ};
    const double vs = opts.voxelSize;
    const double org[3] = {opts.ox, opts.oy, opts.oz};

    // Sweep each of the three axes, both directions.
    for (int d = 0; d < 3; ++d) {
        const int u = (d + 1) % 3;
        const int v = (d + 2) % 3;

        std::vector<FaceKey> mask(size_t(dims[u]) * size_t(dims[v]));

        for (int backFace = 0; backFace < 2; ++backFace) {
            const int dir = backFace ? -1 : 1;
            const int axisIndex = d * 2 + backFace;

            // Backface culling, decided once for a whole axis direction rather
            // than per triangle: all six normals are known up front. A face
            // is visible when its normal opposes the gaze.
            const bool facesAway =
                opts.cullBackFaces && (double(dir) * view[d] > 0.0);

            for (int slice = 0; slice <= dims[d]; ++slice) {
                // Build the mask for this slice.
                for (int j = 0; j < dims[v]; ++j) {
                    for (int i = 0; i < dims[u]; ++i) {
                        int a[3] = {0, 0, 0}, b[3] = {0, 0, 0};
                        a[d] = slice - (backFace ? 0 : 1);
                        b[d] = slice - (backFace ? 1 : 0);
                        a[u] = b[u] = i;
                        a[v] = b[v] = j;

                        // Both sides come from the sampler, including the ones
                        // outside the region: a caller that can answer for the
                        // neighbouring voxel (an apron, or the neighbouring
                        // chunk) gets a seamless join, and a caller that
                        // answers air gets a closed region.
                        //
                        // The one thing the mesher insists on is that the
                        // *solid* side lies inside the region. Otherwise a
                        // region with an apron would emit its neighbour's
                        // inward-facing walls as well as its own, and every
                        // tile boundary would carry two coincident sets of
                        // faces fighting for the same depth.
                        const bool inA = a[d] >= 0 && a[d] < dims[d];
                        const Material ma = sample(a[0], a[1], a[2]);
                        const Material mb = sample(b[0], b[1], b[2]);

                        FaceKey key;
                        // A face exists where a solid voxel meets a non-solid
                        // one. `a` is the solid side for this direction.
                        bool emit = inA && isSolid(ma) && !isSolid(mb);
                        if (emit) {
                            ++st.exposedFaces;
                            if (facesAway) {
                                ++st.backfaceCulled;
                                emit = false;
                            } else if (opts.visibility &&
                                       !opts.visibility->exteriorAir(b[0], b[1], b[2])) {
                                // The air on the far side of this face is a
                                // sealed pocket. Nothing can ever look at it.
                                ++st.sealedCulled;
                                emit = false;
                            }
                        }
                        if (emit) {
                            ++st.emittedFaces;
                            key.mat = ma;
                            // AO sampled in the empty side's plane.
                            int base[3] = {a[0], a[1], a[2]};
                            base[d] += dir;
                            for (int c = 0; c < 4; ++c) {
                                const int du = (c == 1 || c == 2) ? 1 : -1;
                                const int dv = (c >= 2) ? 1 : -1;
                                int s1[3] = {base[0], base[1], base[2]};
                                int s2[3] = {base[0], base[1], base[2]};
                                int cn[3] = {base[0], base[1], base[2]};
                                s1[u] += du;
                                s2[v] += dv;
                                cn[u] += du; cn[v] += dv;
                                key.ao[c] = aoValue(solidAt(s1[0], s1[1], s1[2]),
                                                    solidAt(s2[0], s2[1], s2[2]),
                                                    solidAt(cn[0], cn[1], cn[2]));
                            }
                        }
                        mask[size_t(j) * size_t(dims[u]) + size_t(i)] = key;
                    }
                }

                // Greedily merge rectangles out of the mask.
                for (int j = 0; j < dims[v]; ++j) {
                    for (int i = 0; i < dims[u];) {
                        const FaceKey key = mask[size_t(j) * dims[u] + i];
                        if (!key) { ++i; continue; }

                        int w = 1;
                        while (i + w < dims[u] &&
                               mask[size_t(j) * dims[u] + i + w] == key) ++w;

                        int h = 1;
                        bool grow = true;
                        while (j + h < dims[v] && grow) {
                            for (int k = 0; k < w; ++k)
                                if (!(mask[size_t(j + h) * dims[u] + i + k] == key)) {
                                    grow = false; break;
                                }
                            if (grow) ++h;
                        }

                        // Emit the quad.
                        int origin[3] = {0, 0, 0};
                        origin[d] = slice;
                        origin[u] = i;
                        origin[v] = j;
                        int du[3] = {0, 0, 0}, dv[3] = {0, 0, 0};
                        du[u] = w;
                        dv[v] = h;

                        const uint32_t base = uint32_t(mesh.vertices.size());
                        // Voxel coordinates become world metres here, and
                        // nowhere else: this single multiply is what lets the
                        // same mesher serve micro-voxels and kilometre-scale
                        // distant terrain.
                        auto push = [&](int cx, int cy, int cz, uint8_t ao) {
                            mesh.vertices.push_back(Vertex{
                                float(org[0] + cx * vs),
                                float(org[1] + cy * vs),
                                float(org[2] + cz * vs),
                                uint8_t(key.mat), ao, uint8_t(axisIndex)});
                        };
                        ++st.quads;
                        // Corner order decides the winding, and the winding
                        // decides whether the GPU's back-face cull removes the
                        // hidden half of the world or the visible half.
                        //
                        // This was wrong: every quad was emitted in the same
                        // corner order regardless of which way it faced, so
                        // faces along +d and along -d wound oppositely in
                        // screen space and no single glFrontFace setting was
                        // right for both. The symptom was not a blank screen —
                        // it was a picture of the *inside* of the terrain, with
                        // roughly half the surface missing, which reads as a
                        // shading bug rather than a state bug. Both GL_CCW and
                        // GL_CW measured about 19% wrong against the software
                        // reference; with the reversal below, and culling on,
                        // it is 0.4%, which is triangle-edge disagreement and
                        // nothing else.
                        //
                        // Reversing the two middle corners for back faces makes
                        // every quad counter-clockwise seen from outside the
                        // solid voxel.
                        const int corner[2][4] = {{0, 1, 2, 3}, {0, 3, 2, 1}};
                        const int* co = corner[backFace];
                        const int cx[4] = {origin[0], origin[0] + du[0],
                                           origin[0] + du[0] + dv[0], origin[0] + dv[0]};
                        const int cy[4] = {origin[1], origin[1] + du[1],
                                           origin[1] + du[1] + dv[1], origin[1] + dv[1]};
                        const int cz[4] = {origin[2], origin[2] + du[2],
                                           origin[2] + du[2] + dv[2], origin[2] + dv[2]};
                        for (int c = 0; c < 4; ++c)
                            push(cx[co[c]], cy[co[c]], cz[co[c]], key.ao[co[c]]);

                        // Flip the triangulation when the AO gradient runs the
                        // other way, or the quad shows a diagonal seam. The
                        // reversal above swaps corners 1 and 3, which leaves
                        // both diagonal sums unchanged, so this test is the
                        // same for front and back faces.
                        const bool flip = (key.ao[0] + key.ao[2]) < (key.ao[1] + key.ao[3]);
                        const uint32_t order[2][6] = {{0,1,2, 0,2,3}, {1,2,3, 1,3,0}};
                        const uint32_t* o = order[flip ? 1 : 0];
                        for (int t = 0; t < 6; ++t) mesh.indices.push_back(base + o[t]);

                        // Clear the consumed rectangle.
                        for (int jj = 0; jj < h; ++jj)
                            for (int ii = 0; ii < w; ++ii)
                                mask[size_t(j + jj) * dims[u] + i + ii] = FaceKey{};
                        i += w;
                    }
                }
            }
        }
    }
    return mesh;
}

}  // namespace ely
