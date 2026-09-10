// The cube-sphere. Specification §4.1.
//
// A planet is six flat voxel grids, one per cube face, projected onto a
// sphere. Local space on each face is an ordinary Cartesian grid, so every
// existing voxel technique — greedy meshing, chunk storage, ray casting,
// building — applies unchanged. Curvature lives in the projection.
//
// The three options were: a flat wrapping world (rejected: destroys the
// fiction the moment you look at the horizon, and makes seamless landing
// impossible), a true spherical voxel grid (rejected: pole singularity, and
// every mining and building operation becomes spherical-coordinate maths), and
// this. The cost of this one is seam handling at the 12 cube edges, which is
// bounded and solvable, and mild area distortion toward face corners, which is
// corrected below.
#pragma once
#include "../core/vec.h"
#include <cstdint>

namespace ely {

enum Face : uint8_t { FACE_PX = 0, FACE_NX, FACE_PY, FACE_NY, FACE_PZ, FACE_NZ };

// Face-local coordinates: (u, v) in [-1, 1], plus an altitude in metres
// measured from sea level. This is the address of a voxel column.
struct FaceCoord {
    Face face;
    double u, v;
};

class CubeSphere {
public:
    // radius: sea-level radius in metres. resolution: voxels along one face
    // edge, derived so that a voxel is about 1 m at the face centre.
    explicit CubeSphere(double radius) : radius_(radius) {
        // A cube face spans a quarter of the great circle. Sizing the grid to
        // that arc length gives ~1 m voxels at the centre of a face; corners
        // stretch by at most 2/sqrt(3) ~= 1.155, which is the standard
        // cube-sphere distortion and is not worth correcting geometrically.
        resolution_ = int((3.14159265358979 * radius_ / 2.0) + 0.5);
    }

    double radius() const { return radius_; }
    int resolution() const { return resolution_; }

    // Face-local (u,v) -> a unit direction from the planet centre.
    //
    // The tangent-adjusted mapping is used rather than the naive one: it
    // trades a little extra maths for a substantially more uniform voxel area
    // across a face, which matters because voxel size varying by 15% across a
    // face is visible as terrain detail changing scale as you walk.
    static Vec3 direction(Face f, double u, double v) {
        const double a = std::tan(u * 0.7853981633974483);   // pi/4
        const double b = std::tan(v * 0.7853981633974483);
        Vec3 d;
        switch (f) {
            case FACE_PX: d = { 1,  b,  -a}; break;
            case FACE_NX: d = {-1,  b,   a}; break;
            case FACE_PY: d = { a,  1,  -b}; break;
            case FACE_NY: d = { a, -1,   b}; break;
            case FACE_PZ: d = { a,  b,   1}; break;
            case FACE_NZ: d = {-a,  b,  -1}; break;
        }
        return d.normalised();
    }

    // The inverse: a direction -> the face that owns it and its coordinates.
    // Ownership at an exact edge is decided by a fixed comparison order, so
    // the mapping is a total function with no ambiguous cases.
    static FaceCoord project(const Vec3& dir) {
        const Vec3 d = dir.normalised();
        const double ax = std::fabs(d.x), ay = std::fabs(d.y), az = std::fabs(d.z);
        Face f;
        double a, b;
        if (ax >= ay && ax >= az) {
            if (d.x > 0) { f = FACE_PX; a = -d.z / ax; b =  d.y / ax; }
            else         { f = FACE_NX; a =  d.z / ax; b =  d.y / ax; }
        } else if (ay >= az) {
            if (d.y > 0) { f = FACE_PY; a =  d.x / ay; b = -d.z / ay; }
            else         { f = FACE_NY; a =  d.x / ay; b =  d.z / ay; }
        } else {
            if (d.z > 0) { f = FACE_PZ; a =  d.x / az; b =  d.y / az; }
            else         { f = FACE_NZ; a = -d.x / az; b =  d.y / az; }
        }
        const double inv = 1.2732395447351628;               // 4/pi
        return {f, std::atan(a) * inv, std::atan(b) * inv};
    }

    // A voxel's world position: direction scaled by (radius + altitude).
    Vec3 worldPos(Face f, double u, double v, double altitude) const {
        return direction(f, u, v) * (radius_ + altitude);
    }

    // Integer voxel index on a face -> face-local (u, v) at the voxel centre.
    double axisToUv(int i) const {
        return (double(i) + 0.5) / double(resolution_) * 2.0 - 1.0;
    }
    int uvToAxis(double t) const {
        int i = int((t * 0.5 + 0.5) * double(resolution_));
        return i < 0 ? 0 : (i >= resolution_ ? resolution_ - 1 : i);
    }

    // The sampling position for all generation fields.
    //
    // THIS is the seam fix. Every climate and density field is sampled at the
    // point's direction from the planet centre, scaled to a nominal sphere —
    // never at face-local (u, v). Two faces meeting at a cube edge ask about
    // the same point in space and therefore receive the same answer, so the
    // fields agree across every seam by construction rather than by stitching.
    //
    // Nothing downstream is permitted to sample a generation field any other
    // way. The seam test walks all 12 edges and fails if this is violated.
    Vec3 fieldPos(Face f, double u, double v) const {
        return direction(f, u, v) * radius_;
    }

private:
    double radius_;
    int resolution_;
};

}  // namespace ely
