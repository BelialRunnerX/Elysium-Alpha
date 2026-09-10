// View frustum culling.
//
// This is the cull that runs per *chunk*, before anything is uploaded or drawn,
// and it is the cheapest of all of them: six plane tests against a box, against
// the alternative of submitting a draw call for terrain that is behind the
// player. Everything else in this engine culls faces; this one culls whole
// regions, and on a 512 m view distance most of the world is behind you.
//
// Planes come straight out of the combined view-projection matrix (the
// Gribb-Hartmann extraction). The reason to take them from the matrix rather
// than build them from the camera is that the matrix is the single source of
// truth for what will actually be rasterised — if the projection changes, the
// cull changes with it, and a cull that disagrees with the projection either
// pops geometry in at the screen edge or wastes the work it was meant to save.
#pragma once
#include "mat.h"
#include <cmath>

namespace ely {

// An axis-aligned box in world metres.
struct Aabb {
    float minX = 0, minY = 0, minZ = 0;
    float maxX = 0, maxY = 0, maxZ = 0;

    static Aabb fromOriginSize(double ox, double oy, double oz,
                               double sx, double sy, double sz) {
        Aabb b;
        b.minX = float(ox);      b.minY = float(oy);      b.minZ = float(oz);
        b.maxX = float(ox + sx); b.maxY = float(oy + sy); b.maxZ = float(oz + sz);
        return b;
    }
    double centreX() const { return (minX + maxX) * 0.5; }
    double centreY() const { return (minY + maxY) * 0.5; }
    double centreZ() const { return (minZ + maxZ) * 0.5; }
};

class Frustum {
public:
    Frustum() = default;
    explicit Frustum(const Mat4& viewProj) { set(viewProj); }

    void set(const Mat4& vp) {
        // Row i of the matrix, in this column-major layout, is m[c][i].
        auto row = [&](int i, double out[4]) {
            for (int c = 0; c < 4; ++c) out[c] = vp.m[c][i];
        };
        double r0[4], r1[4], r2[4], r3[4];
        row(0, r0); row(1, r1); row(2, r2); row(3, r3);

        // Each plane is w plus or minus the matching row. The near plane is
        // w + row 2 because this projection puts the near plane at depth -1
        // (OpenGL); an API whose near plane is at 0, such as Vulkan, needs row
        // 2 alone here. That one line is the second of the three places a port
        // has to touch — see the note in mat.h.
        setPlane(0, r3[0] + r0[0], r3[1] + r0[1], r3[2] + r0[2], r3[3] + r0[3]);
        setPlane(1, r3[0] - r0[0], r3[1] - r0[1], r3[2] - r0[2], r3[3] - r0[3]);
        setPlane(2, r3[0] + r1[0], r3[1] + r1[1], r3[2] + r1[2], r3[3] + r1[3]);
        setPlane(3, r3[0] - r1[0], r3[1] - r1[1], r3[2] - r1[2], r3[3] - r1[3]);
        setPlane(4, r3[0] + r2[0], r3[1] + r2[1], r3[2] + r2[2], r3[3] + r2[3]);
        setPlane(5, r3[0] - r2[0], r3[1] - r2[1], r3[2] - r2[2], r3[3] - r2[3]);
    }

    // Conservative: a box that straddles a plane is kept. False means the box
    // is entirely outside at least one plane, which is a proof of invisibility;
    // true means "possibly visible", which is all a cull needs to promise.
    bool visible(const Aabb& b) const {
        for (int i = 0; i < 6; ++i) {
            const double* p = plane_[i];
            // The box corner furthest along the plane normal. If even that is
            // behind the plane, every corner is.
            const double x = p[0] >= 0 ? b.maxX : b.minX;
            const double y = p[1] >= 0 ? b.maxY : b.minY;
            const double z = p[2] >= 0 ? b.maxZ : b.minZ;
            if (p[0] * x + p[1] * y + p[2] * z + p[3] < 0.0) return false;
        }
        return true;
    }

    bool containsPoint(double x, double y, double z) const {
        for (int i = 0; i < 6; ++i) {
            const double* p = plane_[i];
            if (p[0] * x + p[1] * y + p[2] * z + p[3] < 0.0) return false;
        }
        return true;
    }

    const double* plane(int i) const { return plane_[i]; }

private:
    void setPlane(int i, double a, double b, double c, double d) {
        // Normalising matters: without it the plane distance is scaled
        // arbitrarily per plane, which is harmless for a sign test but wrong
        // the moment anything asks how far outside a box is.
        const double len = std::sqrt(a * a + b * b + c * c);
        const double inv = len > 1e-12 ? 1.0 / len : 0.0;
        plane_[i][0] = a * inv; plane_[i][1] = b * inv;
        plane_[i][2] = c * inv; plane_[i][3] = d * inv;
    }

    double plane_[6][4] = {};
};

}  // namespace ely
