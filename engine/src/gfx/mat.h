// Matrices and cameras, in OpenGL's conventions.
//
// Clip-space conventions differ between graphics APIs, and getting one wrong
// produces a picture that looks *almost* right — upside down, or with the depth
// test backwards so distant terrain draws over near terrain. Both are easy to
// stare past for an hour. This engine renders with OpenGL, so:
//
//   * X is right, Y is UP in normalised device coordinates.
//   * Depth runs -1 at the near plane to +1 at the far plane.
//   * Matrices are column-major and multiply column vectors: clip = P * V * p,
//     which is the layout glUniformMatrix4fv expects with transpose = GL_FALSE.
//
// This was got wrong once already, in the obvious way: the first version used
// Vulkan's conventions (Y down, depth 0..1) and the first GPU frame came out
// perfectly vertically mirrored. It is written down here because the symptom
// looks like a broken camera rather than a wrong constant, and it cost a render
// to find. Porting to Vulkan or D3D means negating m[1][1] and changing the
// depth mapping *and* the near-plane extraction in frustum.h — three places,
// which is why they all say so.
//
// World space is right-handed with Y up, because the terrain generator already
// speaks in altitudes.
#pragma once
#include "../core/vec.h"
#include <cmath>
#include <cstring>

namespace ely {

// Column-major 4x4. m[c][r] — column c, row r — which is the memory order GLSL
// reads a mat4 in, so this struct can go straight into a push constant.
struct Mat4 {
    float m[4][4] = {};

    static Mat4 identity() {
        Mat4 r;
        for (int i = 0; i < 4; ++i) r.m[i][i] = 1.0f;
        return r;
    }

    Mat4 operator*(const Mat4& b) const {
        Mat4 r;
        for (int c = 0; c < 4; ++c)
            for (int row = 0; row < 4; ++row) {
                float s = 0.0f;
                for (int k = 0; k < 4; ++k) s += m[k][row] * b.m[c][k];
                r.m[c][row] = s;
            }
        return r;
    }

    // Transform a point (w = 1), returning the full homogeneous result: the w
    // component is what frustum and near-plane tests need.
    void transform(float x, float y, float z, float out[4]) const {
        for (int row = 0; row < 4; ++row)
            out[row] = m[0][row] * x + m[1][row] * y + m[2][row] * z + m[3][row];
    }
};

// Right-handed look-at. `eye` looks toward `centre`; `up` is the world up hint.
inline Mat4 lookAt(const Vec3& eye, const Vec3& centre, const Vec3& up) {
    const Vec3 f = (centre - eye).normalised();     // forward
    const Vec3 s = f.cross(up).normalised();        // right
    const Vec3 u = s.cross(f);                      // true up

    Mat4 r = Mat4::identity();
    r.m[0][0] = float(s.x); r.m[1][0] = float(s.y); r.m[2][0] = float(s.z);
    r.m[0][1] = float(u.x); r.m[1][1] = float(u.y); r.m[2][1] = float(u.z);
    r.m[0][2] = float(-f.x); r.m[1][2] = float(-f.y); r.m[2][2] = float(-f.z);
    r.m[3][0] = float(-s.dot(eye));
    r.m[3][1] = float(-u.dot(eye));
    r.m[3][2] = float(f.dot(eye));
    return r;
}

// Perspective projection for OpenGL: Y up, depth -1..1.
inline Mat4 perspectiveGl(double fovYRadians, double aspect,
                          double zNear, double zFar) {
    const double f = 1.0 / std::tan(fovYRadians * 0.5);
    Mat4 r;
    r.m[0][0] = float(f / aspect);
    r.m[1][1] = float(f);                                        // Y up
    r.m[2][2] = float((zFar + zNear) / (zNear - zFar));          // depth -1..1
    r.m[2][3] = -1.0f;
    r.m[3][2] = float((2.0 * zFar * zNear) / (zNear - zFar));
    return r;
}

// A fly camera: position plus yaw and pitch, which is all a terrain viewer
// needs and avoids quaternions for something nobody will ever roll.
struct Camera {
    Vec3 position{0, 0, 0};
    double yaw = 0.0;      // radians, 0 looks along -Z
    double pitch = 0.0;    // radians, positive looks up, clamped near vertical
    double fovY = 1.2217304764;   // 70 degrees
    double zNear = 0.08;          // must survive micro-voxels at arm's length
    double zFar = 4000.0;

    Vec3 forward() const {
        const double cp = std::cos(pitch);
        return Vec3{-std::sin(yaw) * cp, std::sin(pitch), -std::cos(yaw) * cp};
    }
    Vec3 right() const {
        return Vec3{std::cos(yaw), 0.0, -std::sin(yaw)};
    }

    // Pitch stops just short of straight up or down. At exactly vertical the
    // forward vector becomes parallel to the up hint and lookAt's cross product
    // collapses, which shows as the view snapping to a random roll.
    void clampPitch() {
        const double lim = 1.5533430343;   // 89 degrees
        if (pitch > lim) pitch = lim;
        if (pitch < -lim) pitch = -lim;
    }

    Mat4 view() const {
        return lookAt(position, position + forward(), Vec3{0, 1, 0});
    }
    Mat4 projection(double aspect) const {
        return perspectiveGl(fovY, aspect, zNear, zFar);
    }
    Mat4 viewProjection(double aspect) const {
        return projection(aspect) * view();
    }
};

}  // namespace ely
