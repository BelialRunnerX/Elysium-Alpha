// The shading model — one definition, two consumers.
//
// The software renderer in src/render/ and the GLSL in src/gfx/shaders/ must
// produce the same picture, or the headless proof images stop being proof.
// Rather than trust that two hand-written copies stay in step, the constants
// live here, the C++ path calls this function, and the GLSL is a literal
// transcription with the same constants written in the same order. A test pins
// the numbers so a change here fails loudly instead of silently making the
// proof images a picture of something the GPU does not draw.
//
// If you change anything in this file, change shaders/terrain.frag to match and
// update the expectations in the shading test. That is the whole contract.
#pragma once
#include "../core/vec.h"
#include "../world/material.h"
#include <cstdint>

namespace ely {

// Per-axis face brightness. Six axis-aligned normals means lighting can be a
// lookup rather than a dot product, and hand-chosen values read better than a
// physical model at this scale: the point is that form is legible when every
// material is flat-coloured.
//
// Order matches Vertex::nx: +X, -X, +Y, -Y, +Z, -Z.
inline constexpr float kFaceLight[6] = {
    0.86f,   // +X
    0.74f,   // -X
    1.00f,   // +Y  sky-facing, the brightest
    0.55f,   // -Y  undersides, the darkest
    0.92f,   // +Z
    0.68f    // -Z
};

// Ambient occlusion response. ao is 0 (fully occluded) to 3 (open), and maps to
// a multiplier of 0.52 at the darkest corner rising to 1.00 at the lightest.
inline constexpr float kAoBase = 0.52f;
inline constexpr float kAoStep = 0.16f;

// Distance fog. Linear in depth, saturating at kFogFull metres, and never quite
// reaching full strength so that distant terrain keeps a silhouette instead of
// dissolving into the background.
inline constexpr float kFogFull = 520.0f;
inline constexpr float kFogMax = 0.82f;
inline constexpr float kFogColour[3] = {78.0f / 255.0f, 92.0f / 255.0f,
                                        118.0f / 255.0f};

inline float faceLight(int axis) {
    return kFaceLight[axis < 0 ? 0 : (axis > 5 ? 5 : axis)];
}

inline float aoLight(int ao) {
    return kAoBase + kAoStep * float(ao < 0 ? 0 : (ao > 3 ? 3 : ao));
}

// The complete model, in linear 0..1 colour.
//
//   base     the material's colour
//   axis     which of the six faces this is
//   ao       0..3 from the mesher
//   depth    distance from the eye in metres
inline void shadeVoxelFace(const float base[3], int axis, int ao, float depth,
                           float out[3]) {
    const float l = faceLight(axis) * aoLight(ao);
    float fog = depth / kFogFull;
    if (fog < 0.0f) fog = 0.0f;
    if (fog > kFogMax) fog = kFogMax;
    for (int c = 0; c < 3; ++c) {
        const float lit = base[c] * l;
        out[c] = lit * (1.0f - fog) + kFogColour[c] * fog;
    }
}

// The material palette the GPU gets as a uniform array, and the software
// renderer reads directly. Built from the material table so that there is still
// only one place a colour is defined (principle E5) — the palette is a
// transport format, not a second source of truth.
struct MaterialPalette {
    float rgb[MAT_COUNT][4] = {};   // vec4 per entry: GLSL std140 wants 16-byte
                                    // alignment, and a vec3 array does not have
                                    // it. The fourth component is unused.
    MaterialPalette() {
        for (int i = 0; i < MAT_COUNT; ++i) {
            const MaterialInfo& info = materialInfo(Material(i));
            rgb[i][0] = info.r / 255.0f;
            rgb[i][1] = info.g / 255.0f;
            rgb[i][2] = info.b / 255.0f;
            rgb[i][3] = 1.0f;
        }
    }
    static constexpr size_t bytes() { return sizeof(float) * 4 * MAT_COUNT; }
};

}  // namespace ely
