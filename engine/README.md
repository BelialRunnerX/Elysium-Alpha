# Elysium — engine core

A C++17 implementation of the world layer and renderer from the Elysium game
specification (2nd edition): a No Man's Sky-scale galaxy where every planet is a
fully voxel, Minecraft-style world.

Rendering is OpenGL 3.3 core. Nothing here *needs* a GPU to be verified: the
graphics pipeline runs headlessly through EGL's surfaceless platform against
Mesa's software rasteriser, so the shaders compile, the framebuffer binds and
the depth test runs in a test that finishes in two seconds on a machine with no
display. That is principle **S6** of the spec pointed at the renderer — *a
generator you can only inspect by flying to it is a generator you will not
inspect* — and it is why the GPU path found three real bugs before any hardware
was involved.

## Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release -DELY_FETCH_DEPENDENCIES=ON
cmake --build build -j
ctest --test-dir build    # eight suites, including a real OpenGL render
```

**Windows / Visual Studio:** double-click `open-in-visual-studio.bat`, or open
the folder in VS 2022 — it reads `CMakePresets.json` natively. See
[`docs/visual-studio.md`](docs/visual-studio.md).

**CI:** `.github/workflows/build.yml` builds and tests on Linux and Windows on
every push, and uploads the rendered proof images as artefacts. The Linux job
runs the *graphics* tests too, on a machine with no GPU and no display, because
Mesa's llvmpipe gives a real OpenGL 3.3 core context through EGL.

Requires CMake ≥ 3.16, a **C++20** compiler and zlib.

Everything else is optional and degrades rather than failing:

| | needed for | found via |
| --- | --- | --- |
| **EnTT** | the ECS, the engine loop, the application | a package (vcpkg), `third_party/entt/`, `../single_include/`, `-DENTT_INCLUDE_DIR=`, or `-DELY_FETCH_DEPENDENCIES=ON` |
| **OpenGL + EGL** | the renderer and the headless graphics tools | the system |
| **GLFW** | the windowed application only | a package |

Without EnTT the build drops three targets, says so, and builds the other
twelve. **The network is a last resort, never a default:** `ELY_FETCH_DEPENDENCIES`
is off unless you ask, because a build that phones home is a build that fails on
a train — and this project was written somewhere with no route to GitHub at all.

## Layout

Modules are real CMake targets whose dependencies point one way only, and each
test executable links **one module**. A header that reaches sideways or upwards
fails to link rather than quietly becoming load-bearing — the layering is
checked by the build, not by discipline.

```
ely_core ──> ely_world ──> ely_mesh ──> ely_gfx ──> ely_gl ──> ely_engine
    ├──────> ely_jobs ───────────────────┤  ^         ^            ^
    └──────> ely_save ───────────────────┘  │         │            │
ely_image ──────────────────────────────────┴ ely_ecs ┴────────────┘
```

| Path | What it is |
| --- | --- |
| `src/core/` | Deterministic seeding (SplitMix64), vectors, Perlin/fbm/ridged noise |
| `src/world/cubesphere.h` | Cube-sphere mapping — and the seam fix |
| `src/world/planet.*` | Planet classes, elements, hazards, per-seed parameters |
| `src/world/material.*` | 43 materials, 22 ores, depth bands, class abundance |
| `src/world/terrain.*` | Climate → height → density → caves → biome → material |
| `src/world/resolution.h` | The resolution ladder: blocks, micro-voxels, chunks |
| `src/world/volume.h` | `PalettedVolume<N>` — palette-compressed voxel cube |
| `src/world/chunk.h` | 32³ blocks plus **sparse** 16³ micro-voxel detail |
| `src/world/lod.h` | LOD selection and sampling the generator at any scale |
| `src/mesh/greedy.*` | Greedy meshing, per-vertex AO, LOD scaling, culling |
| `src/mesh/visibility.*` | Exterior-air flood fill — sealed cavities are not meshed |
| `src/image/image.*` | zlib PNG writer and a depth-buffered software rasteriser |
| `src/gfx/mat.h` | Matrices and a fly camera, in OpenGL's clip conventions |
| `src/gfx/frustum.h` | Frustum plane extraction and AABB culling |
| `src/gfx/vertex.h` | The 8-byte GPU vertex |
| `src/gfx/shading.h` | The shading model — one definition, C++ and GLSL |
| `src/gfx/streamer.h` | LOD residency for a moving camera, with hysteresis |
| `src/gfx/tilebuild.*` | Generate → cull → mesh → pack, one tile |
| `src/jobs/jobsystem.*` | The thread pool, `dispatch` and `parallelFor` |
| `src/save/editstore.*` | The per-chunk edit journal — what the player changed |
| `src/ecs/components.h` | Component types — data only, no behaviour |
| `src/ecs/systems.*` | Residency, dispatch, culling, draw ordering, motion |
| `src/engine/worldview.*` | The engine loop: streaming, threaded building, drawing |
| `src/gl/glcore.h` | Minimal OpenGL 3.3 declarations, and the Windows loader |
| `src/gl/glcontext.*` | Headless EGL context — how the renderer is tested |
| `src/gl/glrenderer.*` | Program, buffers, palette, offscreen target, draw |
| `app/elysium.cpp` | The windowed application (GLFW) |
| `tools/` | Headless inspection: planetmap, crosssection, voxelview, lodview, systemsurvey, glview, flythrough |

## Tools

```sh
./build/systemsurvey  1234          # galaxy → systems → planets
./build/planetmap     23 out        # height, biome, climate maps + a shaded globe
./build/crosssection  23 out        # 1 px/m vertical slice: caves, ore bands, aquifers
./build/voxelview     23 out 96     # generate → pack → mesh → render a 96 m region
./build/lodview       23 out        # the resolution ladder, distance LOD, culling stats
./build/glview        23 out        # a real GPU render, checked against the software one
./build/flythrough    23 out 90     # fly a camera across a planet, headless, 90 frames
./build/elysium       23            # the windowed application, if GLFW was found
```

`glview` and `flythrough` need no GPU and no display; they run against Mesa's
llvmpipe. `ctest` runs both the unit tests and `glview`.

## The three load-bearing decisions

**Every field is sampled in direction space.** The spec flags the cube-sphere
seam as the project's highest technical risk. It is not fixed by stitching; it
is fixed by never introducing it. Two faces meeting at a cube edge ask the
generator about the *same 3D direction*, so they get the same answer by
construction. Verified: 936 samples across all 12 cube edges, 0.000000 m of
height disagreement and zero material mismatches.

**Climate is a shell, volume is 3D.** Climate is sampled at `dir * radius` — a
map, where temperature does not vary with depth. Density, caves and ore are
sampled at `dir * (radius + altitude)`. Conflating the two was a real bug here:
with volume fields on the shell, every 3D noise term was constant down a column,
which silently disabled caves, overhangs and the whole ore system while leaving
the surface looking perfectly plausible.

**The generator is a pure function of a continuous position.** This is what
makes micro-voxels affordable. It was never reading a grid, so asking for a
sample every 62.5 mm instead of every metre costs only the samples. Sub-block
detail therefore needs *no storage at all* until a player edits it — storage
tracks what the player did, not what the world is.

## Resolution, LOD and culling

One block is one metre. Below it, a block subdivides into 16×16×16 micro-voxels
of 62.5 mm (4096 subunits per block). Levels are powers of two in metres, from
lod −4 (micro) upward; one LOD step per doubling of viewing distance, which is
double the voxel edge and one eighth of the voxel count.

Measured on planet 23, a 6×16 grid of 32 m tiles over half a kilometre:

```
voxels generated  1,061,376   vs 9,437,184 at uniform lod 0    8.9x less
faces   exposed     136,623
        backface     42,761   31.3%  normal away from the camera
        drawn         51,424   2.7x fewer than exposed faces after merging
```

Four culls compose, in order of what they actually save:

1. **Interior faces** — only solid-meets-non-solid is meshed at all. This throws
   away the entire inside of the planet and is worth more than everything else
   combined.
2. **LOD** — 8.9× fewer voxels across a 512 m view on this scene.
3. **Greedy merging** — 2–3× fewer quads on terrain, up to 56× on smooth
   micro-voxel ground.
4. **Backface culling** — ~31%, decided once per axis direction rather than per
   triangle, since all six normals are known.
5. **Sealed cavities** — a flood fill from the region boundary; anything it does
   not reach cannot be looked into. *Honest result: on this generator the caves
   form one connected network, so this earns almost nothing underground.* It is
   kept because it costs one pass and will matter for sealed player rooms and
   structure interiors — but it is not what makes the frame budget.

## Rendering

One draw call per tile, and nothing bound per draw but a vertex array and two
uniforms. The chunk's origin and voxel size are a `vec4`; the view-projection is
set once per frame. There is no per-chunk descriptor state at all.

**The vertex is eight bytes.** Positions are unsigned 16-bit integers in voxel
units, chunk-local — a greedy quad corner always lands exactly on the voxel
lattice, so an integer is the exact value rather than an approximation. The
shader reconstructs world position as `origin + p * voxelSize`. That also fixes
a precision problem for free: a float world position 3 km from the planet centre
has about 0.25 mm of resolution left, which is coarse against 62.5 mm
micro-voxels and would show as vertices shimmering at the far edge of a chunk.
Colour is a material *index*, not RGB, so re-tinting the world is a 1 KB uniform
write instead of remeshing everything on screen.

Measured on a 512 m strip: 295,112 vertices and 147,556 triangles in 3.9 MB,
which is 14 bytes per vertex including its share of the 32-bit indices.

**Streaming.** `Streamer` decides residency; `WorldView` runs the loop. Tiles
build on worker threads — `buildTile` is a pure function of (terrain, spec), so
the threading needs no lock around the world at all. Over 90 frames of
continuous motion: 1.85 ms average main-thread streaming cost, residency flat at
~1000 tiles and 15 MB, no growth.

**What the headless GPU path caught.** All three would have been hours of
staring at a screen:

1. *The projection was Vulkan's.* The first GPU frame came out perfectly
   vertically mirrored, which reads as a broken camera rather than a wrong sign.
2. *The mesher wound front and back faces identically*, so no `glFrontFace`
   setting was right for both and back-face culling removed roughly half the
   *visible* world. The symptom was a picture of the inside of the terrain — a
   shading bug, apparently. Both windings measured ~19% wrong against the
   software reference; with the fix, culling on matches culling off to 0.43% of
   pixels, and the wrong winding is decisively 37%.
3. *Tiles that meshed to nothing were never recorded as built*, so every empty
   sky tile was re-queued every frame forever. The workers spent all their time
   regenerating air while visible tiles waited behind them, and the world slowly
   drained away ahead of a walking camera.

The GPU render now agrees with the software reference to **0.43% of pixels**
(mean absolute difference 0.52/255). The residue is triangle-edge disagreement —
two rasterisers deciding which pixel a boundary belongs to — which is every
silhouette in the image.

## Entities and systems

Tile state lives in an [EnTT](https://github.com/skypjack/entt) registry. The
motivation was a bug this engine actually had: residency, build state and GPU
geometry were three separate containers that had to agree, and when they stopped
agreeing — a tile that meshed to nothing was never recorded as built — the
workers spent every frame regenerating empty sky while the visible world drained
away ahead of a walking camera. As entities with tags there is one population,
the questions are views, and that class of disagreement cannot be expressed.

Components are data with no behaviour; systems are free functions over views, so
every one runs against a registry populated by hand with no planet, no GPU and
no threads. The frame is:

```
residency  → which tiles should exist, at what level, from the camera
dispatch   → hand what needs building to the workers, nearest first
ingest     → take back what finished, upload it, record what it was
cull       → mark what the frustum can see
drawList   → order what is visible, nearest first
```

The streamer stays outside the ECS deliberately: it is the policy — hysteresis,
budgets, ordering — and keeping it separate is what lets that policy be tested
without a registry at all. The ECS turns its answer into entities.

Making `residencySystem` idempotent (re-running it must change nothing) was
worth more than expected: the build queue's peak depth over a 200 m walk fell
from 718 tiles to 121.

## Saving

**The world is never saved. Only what somebody did to it is.**

Every voxel of every planet in the galaxy is recomputable from a 64-bit seed, so
storing terrain would be storing the answer to a question the generator already
answers for free. What cannot be recomputed is a player's work, and that is the
whole of what goes on disk. Visit a planet, cut a trench into a hillside, fly to
another system, come back — the trench is there, and the untouched rest of that
world cost nothing, because it was never written.

```
1. as generated          662 triangles, save file 0 bytes
2. after digging         826 triangles, 150 dug out, 36 built
3. saved, reloaded       826 triangles, 1165 bytes on disk

  the digging changed        1449 pixels
  the save round trip changed 0 pixels

  1165 bytes of save for a 9 km planet. Everything else is the seed.
```

`savedemo` produces that, and it is a **test**: it exits non-zero unless the
world it left and the world it came back to are pixel-identical.

The format is an **append-only journal per planet**, keyed by a global chunk
address — the cube-sphere face plus the chunk's integer position in that face's
own metre grid, which is a property of the planet rather than of wherever a
session happened to start. Appending is the whole design: a player mining is a
stream of small edits, and a format that had to seek, read-modify-write and
re-index for each one would either stall the game or need a write-behind cache
with its own consistency problems. Loading replays the journal with later
records winning; compaction rewrites the merged state when the journal grows
past twice the live edit count, via a temporary file and a rename so a crash
mid-compaction leaves the original intact.

Three things it defends against, each with a test:

- **A crash mid-write.** Every record carries a CRC32 and replay stops at the
  first failure. A short read is easy to detect; a *plausible* torn record is
  not, and that is the one that would corrupt a world silently. The player loses
  the last edit rather than the planet.
- **A journal from another planet.** Refused outright — applying it would
  scatter one world's edits across another.
- **A generator that has moved.** The header carries a generator fingerprint.
  Edits are deltas against generated terrain, so if the noise changes they
  become perfectly good data describing a world that no longer exists — a
  doorway now hanging in mid-air. It is loaded anyway and flagged loudly:
  discarding the player's work silently would be worse, and so would applying it
  silently (principle E7).

One edit is 6 bytes. A chunk with no edits has no record; a planet with no edits
has no file.

**Known gap:** edits are applied at one metre and finer. A tile drawn at LOD 2
or coarser cannot represent a one-metre change, so a trench is invisible from
300 m away and appears as you approach. Downsampling edits into coarse levels is
the fix and is not implemented.

## Threading

One pool, and everything parallel goes through it. Tile building used to own its
own threads inside `WorldView`; nothing else could use them, and anything else
that wanted parallelism would have started its own and oversubscribed the
machine. `ely_jobs` inverts that: `dispatch` for individual work, `parallelFor`
for loops, sized once.

Four decisions, each of which is a thing that goes wrong otherwise:

- **Not `std::async`.** Its launch policy is implementation-defined — libstdc++
  will run it deferred, on the calling thread, at the moment you wait for it —
  there is no pooling, and no way to cap concurrency.
- **`parallelFor` runs on the calling thread too**, rather than handing
  everything to workers and blocking. With zero workers it is exactly a serial
  loop, so single-threaded is a configuration rather than a second code path.
- **Nested `parallelFor` runs serially.** Tile building is already parallel
  across tiles; if a tile's columns also fanned out, every worker would wait for
  capacity every other worker was holding — a deadlock that only appears under
  load.
- **Results never depend on scheduling.** Every index writes its own slot and
  nothing is accumulated across jobs.

That last one is asserted, not asserted-to: `test_gfx` builds the same tile with
0, 1, 2, 4 and 8 workers and requires the vertex and index buffers to be
byte-identical, and `genbench` repeats the check while it measures.

```
parallel tile build (32 x 96 x 32 m at 1 m), 2 cores available
   0 workers     83.7 ms   1.00x  identical output
   1 workers     56.4 ms   1.48x  identical output
   2 workers     51.0 ms   1.64x  identical output
   4 workers     52.5 ms   1.59x  identical output
   8 workers     51.7 ms   1.62x  identical output
```

1.6× is close to the ceiling on the two-core machine this was measured on;
adding workers past the core count buys nothing, as the flat tail shows. On a
machine with more cores it should go further, and `genbench` prints the core
count so the number stays interpretable.

There is a lifetime trap in `parallelFor` worth knowing about, because the
obvious implementation has it: if the latch counts *chunks*, the last chunk's
countdown releases the caller, which returns and destroys the stack frame while
the worker that ran that chunk is still going round its loop to read the shared
counter. The latch counts *runners* instead, and the caller finishes with a real
`wait()` rather than a spin on the counter — because the counter hits zero
before the notifying thread has finished touching the latch.

## Generator performance

The engine's one real bottleneck. Every optimisation below is **bit-identical** —
golden world fingerprints for five seeds are asserted in the world tests, and
every change had to reproduce them exactly.

| | before | after |
| --- | --- | --- |
| per column (climate + surface) | 20.65 µs | 5.29 µs |
| a 32 × 96 × 32 m tile at 1 m | 100.4 ms | 51.3 ms |
| a voxel in open sky | 0.265 µs | 0.031 µs |
| initial world load (860 tiles) | 5.8 s | 2.6 s |
| tiles drawn while walking | 51 of 116 | 91 of 116 |

Four changes, in order of what they bought:

1. **The surface scan starts near the ground.** It descended from the top of the
   planet's relief range in 4 m steps — for a column near sea level, hundreds of
   metres of empty sky, each step a full density evaluation. It was 96% of the
   cost of a column. Rock can only exist within `lift × squash` of the terrain
   height, and `genbench` measures the worst `lift` over 400,000 samples at 2.24,
   so 7.2 m is the true ceiling and the scan starts 16 m up. The start is
   *snapped back onto the old 4 m grid*, so the sampled sequence is a suffix of
   the original one and the result is identical.
2. **Per-ore constants are cached.** `oreAt` loops 22 ores per stone voxel and
   every iteration recomputed a class-abundance lookup and a cube root. Cached as
   values, not folded products, so the arithmetic associates exactly as before.
3. **A lattice-cell gradient cache.** The spatial hash was 39% of all generation
   time (measured by stubbing it out). Walking a column samples at 1 m steps
   while the cave fields have a 78 m wavelength, so ~78 consecutive voxels share
   a lattice cell and recomputed the same eight hashes.
4. **The 3D density terms are guarded, not multiplied by zero.** Beyond 40 m from
   the terrain height they contribute nothing, and evaluating them anyway was
   five noise samples per voxel for most of a tile.

Still ~25× over the specification's 2 ms budget for a 32³ chunk. The remaining
cost is genuinely the noise; SIMD and fewer octaves at coarse LOD are the next
levers, and the second of those would change the world.

## Known gaps

- **LOD cracks.** Tiles at different levels do not share vertices, so hairline
  seams appear where two levels meet. Skirts at tile edges are the standard fix
  and are not implemented.
- **Generation is still the bottleneck**, though half of it is gone. Under a
  walking camera the near field is always complete — the player never walks into
  a hole — but the far distance shrinks and fills back in when the camera stops:
  91 tiles drawn while moving against 116 once settled, up from 51. That is the
  generator losing a race, not the scheduler failing.
- **`app/elysium.cpp` has never been compiled**, nor has the Windows GL loader
  in `src/gl/glloader.cpp`. They are the only untested code in the project: the
  environment had no GLFW headers, no display and no Windows toolchain.
  Everything they call is exercised by `tools/flythrough.cpp` against a real
  driver. See `docs/windows.md`.
- **Biome balance.** Temperate worlds run tundra-heavy (the latitude temperature
  term is probably too strong) and Frozen worlds show only two biomes.
- **Quantisation offset.** Fine sampling puts the ground ~0.47 m higher than
  coarse sampling — half a coarse voxel, the expected consequence of testing
  solidity at voxel centres. It is pinned by a test so it cannot grow into a
  visible step, but it does mean ground rises slightly as LOD refines.
