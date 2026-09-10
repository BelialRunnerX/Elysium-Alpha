// The seed hierarchy. Specification §3.2 and principle S2.
//
// Every level of the world derives its children by a pure hash of its own seed
// and the child's index. Nothing here may consult a clock, a global generator,
// container iteration order, or anything else that can differ between two runs
// on two machines. Violating that invalidates every save in existence, because
// the world is not stored — it is regenerated (S1), and a generator that drifts
// regenerates a *different* world under a player's feet.
//
// The mixer is SplitMix64's finaliser, chosen because the first edition's
// dungeon seeding already used it, it passes the usual avalanche tests, and it
// is four lines that cannot be got subtly wrong.
#pragma once
#include <cstdint>

namespace ely {

constexpr uint64_t kGolden = 0x9E3779B97F4A7C15ull;

inline uint64_t mix(uint64_t z) {
    z += kGolden;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

// Fold a label into a seed. Labels in use, per spec: "layout", "biome", "ore",
// "cave", "flora", "fauna", "poi", "loot", "weather", "name".
inline constexpr uint64_t labelHash(const char* s) {
    uint64_t h = 0xCBF29CE484222325ull;              // FNV-1a offset basis
    while (*s) { h ^= static_cast<uint8_t>(*s++); h *= 0x100000001B3ull; }
    return h;
}

inline uint64_t systemSeed(uint64_t galaxy, uint32_t index) {
    return mix(galaxy ^ mix(uint64_t(index) * kGolden));
}

inline uint64_t planetSeed(uint64_t system, uint32_t planet) {
    return mix(system ^ mix(uint64_t(planet + 1) * 0xD1B54A32D192ED03ull));
}

inline uint64_t hash3(int64_t x, int64_t y, int64_t z) {
    return mix(uint64_t(x) * 0x9E3779B97F4A7C15ull
             ^ mix(uint64_t(y) * 0xC2B2AE3D27D4EB4Full)
             ^ mix(uint64_t(z) * 0x165667B19E3779F9ull));
}

inline uint64_t chunkSeed(uint64_t planet, int64_t cx, int64_t cy, int64_t cz) {
    return mix(planet ^ mix(hash3(cx, cy, cz)));
}

inline uint64_t featureSeed(uint64_t parent, uint64_t label, int64_t a, int64_t b) {
    return mix(parent ^ mix(label) ^ mix(hash3(a, b, 0)));
}

// A tiny deterministic PRNG. Explicitly *not* std::mt19937: the standard
// library's distributions are not specified to produce identical output across
// implementations, which would break S2 the first time somebody built on a
// different platform.
struct Rng {
    uint64_t s;
    explicit Rng(uint64_t seed) : s(seed ? seed : 1) {}

    uint64_t next() {
        s += kGolden;
        uint64_t z = s;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    }
    // Unbiased below 2^32 via Lemire's multiply-shift.
    uint32_t below(uint32_t bound) {
        return uint32_t((next() >> 32) * uint64_t(bound) >> 32);
    }
    float unit() { return float(next() >> 40) * (1.0f / 16777216.0f); }  // [0,1)
    float range(float lo, float hi) { return lo + unit() * (hi - lo); }
    bool chance(float p) { return unit() < p; }
};

}  // namespace ely
