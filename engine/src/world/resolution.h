// The resolution ladder: what a "voxel" is at each level.
//
// One *block* is one metre, as in the specification. Unlike Minecraft, a block
// is not the finest thing the world is defined at: it subdivides into
// 16 x 16 x 16 *micro-voxels* of 62.5 mm, so a metre of terrain can hold real
// sub-block shape — a chamfered boulder, a chipped rim, a hand-carved step —
// rather than a staircase.
//
// This is affordable for one reason, and it is worth stating plainly because
// every design decision below follows from it: the generator is a pure function
// of a *continuous* position (principle S1). It was never reading a stored
// grid, so asking it for a sample every 62.5 mm instead of every metre costs
// nothing but the samples. Micro-detail therefore needs **no storage at all**
// until a player edits it. Storage is for deviations from the generator, and
// the generator is infinitely detailed for free.
//
// Levels are a power of two in metres:
//
//     lod -4  =  1/16 m   micro-voxels, the finest the world is defined at
//     lod -1  =  1/2 m
//     lod  0  =  1 m      one block
//     lod +2  =  4 m      ... and coarser
//
// These constants live in their own header because both the storage layer
// (volume.h, chunk.h) and the sampling layer (lod.h) need them, and neither
// should have to include the other.
#pragma once

namespace ely {

constexpr int kMicroBits = 4;                            // 16 = 2^4
constexpr int kMicro = 1 << kMicroBits;                  // per block edge
constexpr int kMicroVolume = kMicro * kMicro * kMicro;   // 4096 per block
constexpr int kFinestLod = -kMicroBits;                  // -4
constexpr double kMicroSize = 1.0 / double(kMicro);      // 0.0625 m

constexpr int kChunkSize = 32;                           // blocks per chunk edge
constexpr int kChunkVolume = kChunkSize * kChunkSize * kChunkSize;

}  // namespace ely
