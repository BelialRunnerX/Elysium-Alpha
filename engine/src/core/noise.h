// Noise, sampled in 3D.
//
// Everything in this engine samples noise in *three* dimensions, in the
// planet's own object space, even for fields that are conceptually 2D maps
// like temperature. That is not decoration — it is the entire solution to the
// cube-sphere seam problem (spec §4.1).
//
// A 2D field defined per cube face has to be stitched at the 12 cube edges,
// where two faces meet at an angle and their grids do not align. A 3D field
// sampled at a point's *direction from the planet centre* has no seam at all,
// because both faces are asking about the same point in space and therefore
// get the same answer. The seam stops being a generation problem and becomes
// only a meshing problem, which is far smaller.
//
// This costs one extra dimension of noise evaluation. It is worth it.
#pragma once
#include "seed.h"
#include "vec.h"

namespace ely {

// One lattice cell's eight corner hashes, remembered.
//
// Walking a column samples the noise fields at one-metre steps while the
// coarsest cave field has a 78 m wavelength — so roughly 78 consecutive voxels
// fall inside the same lattice cell and recompute the same eight hashes. The
// spatial hash was measured at 39% of all generation time (65 ms per tile
// against 40 ms with the hash stubbed out), and almost all of it was that
// repetition.
//
// The cache stores hash3 itself rather than the finished gradient, because
// hash3 does not depend on the field's seed — only the XOR at the point of use
// does. Keyed on the cell coordinates alone, so it stays correct across
// planets, and a miss simply costs what the old code always cost.
//
// Callers own these. That is deliberate: Noise stays const and stateless, which
// is what lets four worker threads sample the same terrain with no locking.
struct NoiseCell {
    int64_t ix = INT64_MIN, iy = INT64_MIN, iz = INT64_MIN;
    uint64_t h[8] = {};
};

// Gradient noise. Perlin's construction with a hashed gradient per lattice
// point; deterministic in (seed, position) and nothing else.
class Noise {
public:
    explicit Noise(uint64_t seed) : seed_(seed) {}

    double sample(double x, double y, double z) const {
        const double fx = std::floor(x), fy = std::floor(y), fz = std::floor(z);
        const int64_t ix = int64_t(fx), iy = int64_t(fy), iz = int64_t(fz);
        const double tx = x - fx, ty = y - fy, tz = z - fz;
        const double ux = smooth(tx), uy = smooth(ty), uz = smooth(tz);

        // Corner contributions, in a fixed order. The order is load-bearing:
        // floating-point addition is not associative, so a parallel reduction
        // or a reordered sum produces a different world (S2).
        const double c000 = grad(ix,     iy,     iz,     tx,       ty,       tz);
        const double c100 = grad(ix + 1, iy,     iz,     tx - 1,   ty,       tz);
        const double c010 = grad(ix,     iy + 1, iz,     tx,       ty - 1,   tz);
        const double c110 = grad(ix + 1, iy + 1, iz,     tx - 1,   ty - 1,   tz);
        const double c001 = grad(ix,     iy,     iz + 1, tx,       ty,       tz - 1);
        const double c101 = grad(ix + 1, iy,     iz + 1, tx - 1,   ty,       tz - 1);
        const double c011 = grad(ix,     iy + 1, iz + 1, tx,       ty - 1,   tz - 1);
        const double c111 = grad(ix + 1, iy + 1, iz + 1, tx - 1,   ty - 1,   tz - 1);

        const double x00 = lerpd(c000, c100, ux);
        const double x10 = lerpd(c010, c110, ux);
        const double x01 = lerpd(c001, c101, ux);
        const double x11 = lerpd(c011, c111, ux);
        return lerpd(lerpd(x00, x10, uy), lerpd(x01, x11, uy), uz);
    }

    double sample(const Vec3& p) const { return sample(p.x, p.y, p.z); }

    // The same value, reusing `cell` when this point lands in the lattice cell
    // it already holds. Bit-identical to sample(): the cached hashes are the
    // same numbers hash3 would return.
    double sample(double x, double y, double z, NoiseCell& cell) const {
        const double fx = std::floor(x), fy = std::floor(y), fz = std::floor(z);
        const int64_t ix = int64_t(fx), iy = int64_t(fy), iz = int64_t(fz);
        if (cell.ix != ix || cell.iy != iy || cell.iz != iz) {
            cell.ix = ix; cell.iy = iy; cell.iz = iz;
            cell.h[0] = hash3(ix,     iy,     iz);
            cell.h[1] = hash3(ix + 1, iy,     iz);
            cell.h[2] = hash3(ix,     iy + 1, iz);
            cell.h[3] = hash3(ix + 1, iy + 1, iz);
            cell.h[4] = hash3(ix,     iy,     iz + 1);
            cell.h[5] = hash3(ix + 1, iy,     iz + 1);
            cell.h[6] = hash3(ix,     iy + 1, iz + 1);
            cell.h[7] = hash3(ix + 1, iy + 1, iz + 1);
        }
        const double tx = x - fx, ty = y - fy, tz = z - fz;
        const double ux = smooth(tx), uy = smooth(ty), uz = smooth(tz);

        // Same fixed summation order as sample(). Reordering these would be a
        // different world (S2), cache or no cache.
        const double c000 = gradFrom(cell.h[0], tx,     ty,     tz);
        const double c100 = gradFrom(cell.h[1], tx - 1, ty,     tz);
        const double c010 = gradFrom(cell.h[2], tx,     ty - 1, tz);
        const double c110 = gradFrom(cell.h[3], tx - 1, ty - 1, tz);
        const double c001 = gradFrom(cell.h[4], tx,     ty,     tz - 1);
        const double c101 = gradFrom(cell.h[5], tx - 1, ty,     tz - 1);
        const double c011 = gradFrom(cell.h[6], tx,     ty - 1, tz - 1);
        const double c111 = gradFrom(cell.h[7], tx - 1, ty - 1, tz - 1);

        const double x00 = lerpd(c000, c100, ux);
        const double x10 = lerpd(c010, c110, ux);
        const double x01 = lerpd(c001, c101, ux);
        const double x11 = lerpd(c011, c111, ux);
        return lerpd(lerpd(x00, x10, uy), lerpd(x01, x11, uy), uz);
    }

    // Fractal Brownian motion: octaves of doubling frequency, halving weight.
    double fbm(const Vec3& p, int octaves, double lacunarity = 2.0,
               double gain = 0.5) const {
        double sum = 0, amp = 1, norm = 0, freq = 1;
        for (int i = 0; i < octaves; ++i) {
            sum += sample(p * freq) * amp;
            norm += amp;
            amp *= gain;
            freq *= lacunarity;
        }
        return norm > 0 ? sum / norm : 0;
    }

    // fbm with one cached lattice cell per octave. `cells` must have at least
    // `octaves` entries and must belong to this field — each octave samples at
    // its own frequency, so they land in different cells and cannot share one.
    double fbm(const Vec3& p, int octaves, NoiseCell* cells,
               double lacunarity = 2.0, double gain = 0.5) const {
        double sum = 0, amp = 1, norm = 0, freq = 1;
        for (int i = 0; i < octaves; ++i) {
            sum += sample(p.x * freq, p.y * freq, p.z * freq, cells[i]) * amp;
            norm += amp;
            amp *= gain;
            freq *= lacunarity;
        }
        return norm > 0 ? sum / norm : 0;
    }

    // Ridged noise: folded absolute value, which produces creases rather than
    // blobs. This is what makes mountain chains and valley networks read as
    // chains and networks instead of as lumps (spec §4.2, "Ridges").
    double ridged(const Vec3& p, int octaves, double lacunarity = 2.0,
                  double gain = 0.5) const {
        double sum = 0, amp = 1, norm = 0, freq = 1;
        for (int i = 0; i < octaves; ++i) {
            double n = 1.0 - std::fabs(sample(p * freq));
            sum += n * n * amp;
            norm += amp;
            amp *= gain;
            freq *= lacunarity;
        }
        return norm > 0 ? (sum / norm) * 2.0 - 1.0 : 0;
    }

private:
    double grad(int64_t ix, int64_t iy, int64_t iz,
                double dx, double dy, double dz) const {
        return gradFrom(hash3(ix, iy, iz), dx, dy, dz);
    }

    // The gradient for a corner whose spatial hash is already known. The seed
    // is mixed in here rather than inside hash3, which is what lets the cache
    // hold values that are valid for every field.
    double gradFrom(uint64_t spatial, double dx, double dy, double dz) const {
        // A hashed unit-ish gradient from the 12 edge midpoints of a cube,
        // Perlin's improved set. Cheap and free of axis-aligned artefacts.
        const uint64_t h = spatial ^ seed_;
        switch (h & 15u) {
            case  0: return  dx + dy;  case  1: return -dx + dy;
            case  2: return  dx - dy;  case  3: return -dx - dy;
            case  4: return  dx + dz;  case  5: return -dx + dz;
            case  6: return  dx - dz;  case  7: return -dx - dz;
            case  8: return  dy + dz;  case  9: return -dy + dz;
            case 10: return  dy - dz;  case 11: return -dy - dz;
            case 12: return  dx + dy;  case 13: return -dy + dz;
            case 14: return -dx + dy;  default: return -dy - dz;
        }
    }
    uint64_t seed_;
};

}  // namespace ely
