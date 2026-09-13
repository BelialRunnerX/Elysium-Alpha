// Minimal vector maths. Deliberately not a dependency: the engine needs about
// twelve operations and every one of them has to be deterministic in its
// evaluation order (S2), which is easier to guarantee in code we own.
#pragma once
#include <cmath>

namespace ely {

struct Vec3 {
    double x = 0, y = 0, z = 0;

    Vec3() = default;
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(double s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(double s) const { return {x / s, y / s, z / s}; }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }

    double dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
    double length() const { return std::sqrt(dot(*this)); }
    Vec3 normalised() const {
        double l = length();
        return l > 1e-300 ? *this / l : Vec3{0, 1, 0};
    }
};

inline double clampd(double v, double lo, double hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
inline double lerpd(double a, double b, double t) { return a + (b - a) * t; }

// Smootherstep. Used everywhere noise is interpolated; C2 continuous, which
// matters because a C1 kink shows up as a visible crease across terrain.
inline double smooth(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }

}  // namespace ely
