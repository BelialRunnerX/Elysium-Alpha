// Level of detail: choosing which resolution to materialise, and sampling the
// generator at it.
//
// The ladder itself is in resolution.h. What is here is the policy — how far
// away a level is good enough for — and the sampler that answers the generator
// at an arbitrary voxel size.
//
// Each step up doubles the voxel edge, so it holds a quarter of the surface
// samples and an eighth of the volume, which is the "half the voxels at twice
// the distance" rule expressed in the axis that actually costs money.
#pragma once
#include "resolution.h"
#include "terrain.h"
#include <cmath>

namespace ely {

inline double lodVoxelSize(int lod) { return std::exp2(double(lod)); }

// Voxels per unit volume, relative to level 0. The design asks for "half the
// voxels at twice the distance"; in three dimensions one LOD step is a factor
// of 8 in voxel count, which is the number that actually decides whether a
// frame fits.
inline double lodVoxelDensity(int lod) { return std::exp2(-3.0 * double(lod)); }

// Level of detail for a viewing distance.
//
// The rule the design asks for: at twice the distance, half the linear voxel
// density. That is exactly one LOD step per doubling of distance, which makes
// the selection a base-2 logarithm and nothing more.
//
//   nearDistance is where the finest level applies. Beyond it, every doubling
//   of distance steps one level coarser, clamped to a maximum.
inline int lodForDistance(double distance, double nearDistance = 24.0,
                          int finest = kFinestLod, int coarsest = 6) {
    if (distance <= nearDistance) return finest;
    const int step = int(std::floor(std::log2(distance / nearDistance)));
    const int lod = finest + step;
    return lod < finest ? finest : (lod > coarsest ? coarsest : lod);
}

// The screen-space size of one voxel at a distance, for checking that a LOD
// choice actually holds a pixel budget rather than merely sounding right.
inline double voxelPixels(int lod, double distance, double fovPixels) {
    return lodVoxelSize(lod) / distance * fovPixels;
}

// Sample the world at an arbitrary resolution.
//
// The generator does not know about LOD: it answers about a point. This picks
// the point at the centre of the voxel being asked about, which is the correct
// thing for a "does this voxel contain rock" question and keeps every level
// consistent with every other — a coarse voxel is the material at its middle,
// not an average, so LOD transitions cannot introduce materials that are not
// there at full detail.
class LodSampler {
public:
    LodSampler(const Terrain& t, Face face, double uCentre, double vCentre,
               double altCentre, int lod)
        : t_(t), face_(face), lod_(lod), size_(lodVoxelSize(lod)),
          u0_(uCentre), v0_(vCentre), alt0_(altCentre),
          metresPerUv_(t.planet().radius * 3.14159265358979 / 4.0) {}

    double voxelSize() const { return size_; }
    int lod() const { return lod_; }

    // Voxel indices are relative to the centre, in units of this level's voxel
    // size, so index 0 is the same world point at every level.
    Material at(int x, int y, int z) const {
        const double u = u0_ + (double(x) + 0.5) * size_ / metresPerUv_;
        const double v = v0_ + (double(z) + 0.5) * size_ / metresPerUv_;
        return t_.materialAt(face_, u, v, alt0_ + (double(y) + 0.5) * size_);
    }

    // Column-cached walk, for filling a region. Same result as at(), one to
    // two orders of magnitude faster (see Terrain::columnAt).
    template <typename Fn>
    void forEachColumn(int x0, int x1, int z0, int z1, int y0, int y1,
                       Fn&& emit) const {
        for (int z = z0; z < z1; ++z) {
            for (int x = x0; x < x1; ++x) {
                const double u = u0_ + (double(x) + 0.5) * size_ / metresPerUv_;
                const double v = v0_ + (double(z) + 0.5) * size_ / metresPerUv_;
                const Terrain::Column col = t_.columnAt(face_, u, v);
                for (int y = y0; y < y1; ++y)
                    emit(x, y, z,
                         t_.materialInColumn(col, alt0_ + (double(y) + 0.5) * size_,
                                             size_));
            }
        }
    }

private:
    const Terrain& t_;
    Face face_;
    int lod_;
    double size_, u0_, v0_, alt0_, metresPerUv_;
};

}  // namespace ely
