// The component types.
//
// Every component here is data and nothing else — no methods, no invariants, no
// constructors that do work. That is the discipline an ECS is worth having for:
// behaviour lives in systems, which are plain functions over views, and any
// system can be run, tested or replaced without touching the data it operates
// on.
//
// The engine uses entities for two quite different populations, and it is worth
// being explicit that this is deliberate rather than accidental:
//
//   TERRAIN TILES. Streaming used to be an unordered_map from tile id to a
//   small struct, plus a second map inside the renderer, plus a set of ids in
//   flight. Three containers that had to agree, and when they stopped agreeing
//   the symptom was terrain draining away ahead of a walking camera. As
//   entities they are one population with tags, and "every tile that is
//   resident but has no geometry yet" stops being a manual cross-reference and
//   becomes a view.
//
//   GAME OBJECTS. The player, creatures, dropped items, projectiles — the
//   things the specification's mechanics act on. None of them exist yet; the
//   components for position and motion are here because the streaming work
//   needs a camera anyway, and a camera is the first game object.
#pragma once
#include "../core/vec.h"
#include "../gfx/frustum.h"
#include "../gfx/streamer.h"

#include <cstdint>

namespace ely::ecs {

// --- Spatial ---------------------------------------------------------------

struct Transform {
    Vec3 position{0, 0, 0};
    double yaw = 0.0;     // radians; 0 looks along -Z
    double pitch = 0.0;   // radians; positive looks up
};

struct Velocity {
    Vec3 linear{0, 0, 0};   // metres per second
};

// --- Camera ----------------------------------------------------------------

struct CameraLens {
    double fovY = 1.2217304764;   // 70 degrees
    double zNear = 0.15;
    double zFar = 1600.0;
};

// Tag: the camera the world streams around and renders from. Exactly one
// entity should carry it; systems take the first they find, so a second is a
// bug that shows as the world loading around the wrong place.
struct ActiveCamera {};

// --- Terrain tiles ---------------------------------------------------------

// Which piece of world this entity is.
struct Tile {
    TileKey key;
    uint64_t id = 0;   // the renderer's chunk handle, packed from the key
};

// The level this tile has actually been built at. Absent until it has been
// built even once, which is what distinguishes "asked for" from "delivered".
struct TileLevel {
    int lod = 0;
};

struct TileBounds {
    Aabb box;
};

// Geometry that exists on the GPU. Absent means nothing to draw — either the
// tile has not been built, or it was built and turned out to be empty, and
// those two are told apart by whether TileLevel is present.
struct TileGeometry {
    uint32_t triangles = 0;
    uint32_t bytes = 0;
};

// Tag: built, and it contained nothing. Most tiles in a resident set are this —
// open sky above the ground, solid rock below it with no exposed faces — and
// recording the fact is what stops them being rebuilt every frame forever.
struct TileEmpty {};

// Tag: handed to a worker thread; a result is coming.
struct TileBuilding {};

// The level this tile *should* be at, when that differs from TileLevel. Its
// presence is the queue: a system picks these up, dispatches them and removes
// them.
struct TileWanted {
    int lod = 0;
};

// Tag: survived the frustum test this frame. Rewritten every frame, so it is
// never stale; the alternative — a visibility flag that persists — is how a
// chunk ends up drawn from a camera position that no longer exists.
struct TileVisible {};

}  // namespace ely::ecs
