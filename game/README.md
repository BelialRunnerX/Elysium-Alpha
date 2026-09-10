# Elysium — Production Client (monorepo `game/`)

v0.25 cinematic open-world pass on the production-compile reconciliation tree.

Canonical native client for the unified Elysium monorepo. Upstream history: `production-build` v0.24.

The Part Two stream preserves the v0.20 playable substrate and v0.22 fortress/civilization surface while v0.23 adds deterministic cross-system workflow planning and indexed autonomy/logistics services. See `FORTRESS_SIMULATION_V022.md` for the broad system catalogue, `FORTRESS_WORKFLOWS_V023.md` for the new integration layer, and `VALIDATION_REPORT.md` for the intentionally compile-only verification level.
## Part Two development checkpoint

This standalone Part Two tree continues full-game development from the frozen v0.13 handoff. The coworker is translating and merging systems into a separate running build; code that has not been merged yet is **still active development**, not deprecated. `COWORKER_INTEGRATION_CONTRACT.md` records the constraints that translated code must satisfy at planetary scale.

The v0.20 focus is a **high-throughput industry + explicit logistics expansion** from Parts 7–9 of the Third Edition. Conveyor, Sorter and Cargo Loader are functional stable machines layered on top of local Network Storage; Crusher, Chemical Vat, Fabricator and Extractor now extend the production graph into ore preprocessing, components, automated mining and defense resupply. Directed logistics uses stable infrastructure IDs plus owner addresses, processing is deterministic and power-aware, Extractor search is bounded/address-keyed, and all new persistent state is headlessly covered. This does **not** replace Network Storage for ordinary crafting—the explicit graph remains the optional high-throughput factory path specified by the design. `INDUSTRY_LOGISTICS.md` and `LOGISTICS_ROUTING.md` are the merge-facing contracts.

A C++20 implementation foundation derived from the **Elysium — Game Design & Architecture Specification, Third Edition**.

> **Production build note (2026-09-09):** the default CMake graph now classifies all 1,612 recovered `.cpp` files. `Elysium.exe` directly links `src/main.cpp` plus all 1,587 non-main runtime translation units; all 22 test and 2 developer-tool `.cpp` files are mandatory separate targets. See `PRODUCTION_BUILD.md` and `.github/workflows/windows-build.yml`.

The project deliberately distinguishes implementation from verification. `VERIFICATION_LOG.md` is the canonical shareable handoff file for every native-client, GPU, platform, stress, save-fixture, or performance check that cannot be completed in the current environment. Deferred verification is not treated as a reason to stop development.

## Technology and architecture

- C++20
- EnTT production ECS isolated behind `EcsWorld`
- raylib 5.5 / OpenGL first client backend behind `IGraphicsBackend`
- deterministic six-face cube-sphere planets
- 1 m macrovoxels with optional 16×16×16 MicroBricks (6.25 cm)
- 32³ logical spherical chunks
- sparse per-touched-chunk player journals
- one bounded engine-wide `JobSystem`
- bounded reconstructed CPU chunk cache with a one-cell cross-chunk/cross-face halo
- `Full / FieldNear / FieldFar / Orbit` rendering hierarchy
- global save schema **v8**, spherical sidecar record format **v8**, generator version **1**, fingerprint **`ELYSPH01`**
- shared `SurfaceWorldReadService` over reconstructed cache packets with deterministic procedural fallback
- derived per-chunk `EditInfluenceSummary` for LOD significance/bounds
- bounded cached spherical A* route service with revision invalidation
- stable Door/Airlock portals + powered interlocked airlock assemblies + bounded persistent room/chamber pressure/oxygen
- powered spherical Sensor Mast / Turret / Shield Pylon / Logic Controller defense machines
- finite persistent automation rules driven by stable machine/portal IDs and ECS-derived hostile contacts
- deterministic stable-ID `SurfaceSiegeDirector` for patrols / 3-wave Marked / 5-wave Hunted Register Actions
- authored Imperial ECS archetypes: Drone / Lictor / Adept / Praetor, including Adept support healing
- stable Furnace / Alloy Crucible / Refinery / Network Storage industrial machines with persistent processing state
- functional Conveyor / Sorter / Cargo Loader routing with stable-ID endpoints, filters, jams, power stalls and bounded throughput
- functional Crusher / Chemical Vat / Fabricator processing with a 27-recipe vertical-slice production graph
- bounded automated Extractor that edits only natural ore, publishes output safely and emits territorial Suspicion telemetry
- local Network Storage → Turret fabricated-ammunition resupply bridge
- infrastructure journal schema **v5** for industrial inventories, recipes, explicit-logistics links/progress and Extractor progress; v1–v4 remain readable

Persistent IDs, `entt::entity`, cube-sphere storage addresses, worker packets, and graphics handles remain separate identity domains. Workers compute immutable results. Authoritative ECS/world mutation, save publication, and GPU publication remain owner-thread operations.

## Build the playable client

Requirements: Git, CMake 3.24+, a C++20 compiler, and network access during first configure so FetchContent can obtain raylib and EnTT.

### Windows

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DELYSIUM_BUILD_CLIENT=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
.\build\Release\Elysium.exe
```

### Linux / macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/elysium
```

## Validate the full recovered source graph

The production graph intentionally does not provide a reduced headless escape hatch: all recovered runtime modules compile with the real client dependency surface. For a dependency-free local API/syntax probe, use the checked-in test stubs directly; CI is the real compile/link authority.

## Primary controls — cube-sphere surface

The client starts in spherical surface mode.

| Input | Action |
|---|---|
| WASD | Move in the local planetary tangent frame |
| Mouse | Look relative to radial up |
| Space | Jump against radial gravity |
| Left Shift | Sprint |
| Left Mouse | Hold to mine; in micro mode chisel a 6.25 cm cell |
| Right Mouse | Place the selected macro block or machine |
| V | Toggle macro tool / 16³ micro-sculpt mode |
| 1–5 | Dirt / Stone / Planks / Steel Plate / Registry Beacon |
| 6–9 | Burner Generator / Battery Bank / Atmosphere Unit / Storage Crate |
| K | Select Airlock Controller machine kit |
| J | Select Sensor Mast machine kit |
| U | Select Turret machine kit |
| Y | Select Shield Pylon machine kit |
| L | Select Logic Controller machine kit |
| G | Select Furnace machine kit |
| H | Select Alloy Crucible machine kit |
| I | Select Refinery machine kit |
| N | Select Network Storage machine kit |
| Q | Select Conveyor machine kit |
| R | Select Sorter machine kit |
| X | Select Cargo Loader machine kit |
| Z | Select Crusher machine kit |
| F1 | Select Chemical Vat machine kit |
| F2 | Select Fabricator machine kit |
| F3 | Select Extractor machine kit |
| [ / ] | Cycle all machine kits |
| B | Select Door placement kit |
| 0 | Select Airlock placement kit |
| E near standalone Door/Airlock | Toggle open / closed |
| E near Airlock Controller | Pair two nearby Airlocks, then cycle chamber interior/exterior |
| E near industrial/logistics machine | Deposit one compatible material or use that machine interaction |
| Shift+E near industrial machine | Withdraw one item from its local inventory |
| Ctrl+E near Conveyor/Sorter/Cargo Loader | Prototype nearest-endpoint route authoring |
| E near Burner | Feed one Coal Ore for +60 seconds |
| E near Turret | Load 12 rounds from one Steel Plate |
| E near Logic Controller | Create the prototype Sensor→Turret/portal defense automation rules |
| E near surface ship | Restore suit reserves |
| C | Crafting / infrastructure menu |
| M | System chart |
| T near surface ship | Travel among the three vertical-slice planets |
| F | Fire at Imperial surface drones |
| O | Orbital climate/topology view |
| P | Toggle legacy planar compatibility patch |
| F5 / F9 | Save / reload |

---




# v0.18 architecture changes

## 1. Territorial Register Action director

`SurfaceSiegeDirector` is a dependency-free simulation service that owns only enforcement state and stable enemy IDs. It does **not** own EnTT entities, renderer handles, dense planet indices, or cube-face seam tables. The existing deterministic dispatch roll still decides whether the Empire reacts; the director decides what that successful roll means at the current Suspicion band.

Prototype attention thresholds are `Noted=25`, `Marked=50`, `Hunted=75`. The 25 activation point is inherited from the specification; the 50/75 split is explicitly tuning. Claimed Marked systems schedule a **3-wave Register Action** and claimed Hunted systems schedule a **5-wave Register Action ending in a Praetor**. Unclaimed systems receive patrols instead because Register Actions are territorial.

Register Actions use the specification's approximately ten-minute warning (`600 s`) before the first wave. Tests inject shorter timing without changing production tuning. Between waves, the director waits a deterministic bounded inter-wave interval.

## 2. Stable spawn requests and ECS ownership boundary

Each wave emits `RegisterActionSpawnRequest` records containing:

- stable action ID;
- stable enemy ID;
- authored Imperial role;
- wave/ordinal;
- deterministic azimuth;
- spawn distance.

The game/authoritative owner thread translates those requests into ECS commands. The director reconciles deaths from stable-ID snapshots, so worker combat systems never need a direct callback into siege state and `entt::entity` never becomes persistent identity.

## 3. Authored Imperial archetypes

The EnTT adapter now distinguishes `Drone`, `Lictor`, `Adept`, and `Praetor`. Archetype data drives health, movement, melee damage/range/period, and optional support behavior. The current Adept performs a deterministic nearby heal against the lowest-health ally during phased AI, with the actual health mutation committed on the owner thread. Presentation is still primitive geometry, but silhouettes are scaled/colored differently for debugging.

This is the first implementation step toward the specification's authored Empire family: Drone mobile grunt, Lictor armored elite, Adept support, and a Praetor climax for Hunted Register Actions.

## 4. Success/failure consequence seam

Destroying/releasing the Registry Beacon fails an active Register Action while leaving world construction intact. Clearing a Register Action currently moves Suspicion toward the structural floor, grants a small prototype Favor/salvage reward, and preserves the claim. These reward numbers remain tuning.

The director has a narrow schema-v1 text state codec for future campaign/system-journal integration. The standalone global manifest does **not yet** embed active siege state; that remains a tracked follow-up rather than hidden as verified persistence.

## 5. Integration constraints preserved

The v0.18 siege path allocates no dense planet-wide column arrays and introduces no seam-rotation cases. Spawn positioning in the standalone client uses direction/tangent-space placement, while the merge contract remains `CubeSphere::project` ownership in the coworker's running build. Existing address-keyed persistence and the coworker's authoritative 0/1/2/4/8 determinism/pathfinder regressions remain unchanged.

---

# v0.17 architecture changes

## 1. Infrastructure journal schema v2

`InfrastructureJournalRecord` now persists explicit `InfrastructureStableRef` dependencies. Each reference carries a record kind, stable object ID, and authoritative `SurfaceCellAddress`. Airlock assemblies therefore name their controller and both portals by stable address, while automation rules name their controller/source/target without relying on a dense global column index. Schema-v1 records remain readable.

## 2. Sparse cross-shard load frontier

`collectMissingInfrastructureDependencies()` accepts a partial record batch plus the already-loaded infrastructure set and returns the exact missing stable references, deduplicated and deterministically ordered. A translated loader can request those owner-address shards directly. Dependencies satisfied by surviving upserts in the same batch are not reported missing.

## 3. Compaction and closure validation

`compactInfrastructureJournal()` implements deterministic latest-write-wins compaction by `(record kind, stable ID)`. Stable ownership is immutable inside one journal stream, and ordinary compaction retains tombstones so previously committed objects cannot be resurrected. `validateInfrastructureSnapshotClosure()` rejects dangling/tombstoned dependencies and owner-address mismatches before mutating live infrastructure.

## 4. Full development stream remains active

The player controller, mining, crafting, drones, raylib backend, chunk cache, persistence prototypes, airlocks, defense machines and automation systems remain valid Part Two work. They have not all been translated into the coworker's running build yet, but development continues here according to the Third Edition specification. Merge notes distinguish portable contracts from prototype-specific implementation details.

---

# v0.15 architecture changes

## 1. Powered base-defense machine family

Part Two now implements four additional spherical machines from the Third Edition defense/command catalogue while preserving the stable numeric meanings of machine types 0–4:

- `SensorMast = 5` — 1.5-unit priority-2 load, 30 m hostile detection;
- `Turret = 6` — 4-unit priority-2 load, persistent ammunition, deterministic 20 m target selection and ECS-facing fire requests;
- `ShieldPylon = 7` — 12-unit priority-2 load, persistent 120-point charge reservoir, 10 m protected radius;
- `LogicController = 8` — 1-unit priority-2 load and owner of persistent automation rules.

`SurfaceInfrastructure::update()` now assigns a deterministic runtime power-network identity to connected machines. Sensors publish contacts only into their own powered network; turrets consume only contacts observed by a sensor on that same network. Turrets do not mutate EnTT directly: they emit stable-ID `SurfaceTurretFireRequest` records, and the owner-thread game layer converts those into ECS command-buffer damage operations.

## 2. ECS command buffer gains deterministic damage publication

`EcsWorld` now supports queued `DamageEnemy` commands in addition to spawn/destroy commands. Turret fire therefore preserves the architecture rule that worker/world systems communicate through stable IDs while owner-thread ECS mutation happens during ordered command publication.

The surface AI commit path also accepts an owner-thread damage-mitigation callback. A powered Shield Pylon may consume persistent charge before drone damage reaches the player Health component. Attempted/applied AI damage is recorded separately in `SurfaceAiTelemetry`.

v0.15 also fixes a duplicate local declaration in `EcsWorld::spawnDrone()` that was present in the Part One-derived source and would have been a native EnTT compilation error. Native EnTT compilation is still deferred because FetchContent cannot reach GitHub in this environment; the fix is explicitly called out in `VERIFICATION_LOG.md` for external confirmation.

## 3. Finite trigger-condition-action automation contract

`SurfaceAutomationRule` is a stable persistent data record owned by a Logic Controller. The initial finite vocabulary intentionally avoids embedding a scripting language:

**Triggers**

- hostile count reported by a Sensor Mast;
- Battery Bank charge below a threshold;
- Atmosphere Unit pressure below a threshold.

**Actions**

- enable a machine;
- disable a machine;
- close a Door/Airlock portal through the authoritative voxel state.

Rules are evaluated in stable-ID order and only when their Logic Controller is enabled and powered. The prototype `E` interaction near a Logic Controller creates the first gameplay rule set: hostile detection enables a nearby Turret and closes the nearest portal. This is a construction/test interface, not the final automation UI.

## 4. Sidecar record format v5

The global campaign manifest remains schema **v8**. Spherical touched-chunk sidecars advance from v4 to **v5** and add:

- persistent Turret ammunition;
- persistent Shield Pylon charge;
- stable automation-rule records owned by the chunk containing their Logic Controller.

Formats v1–v4 remain readable. Runtime-only fields such as power-network ID, sensor counts and turret cooldown are deliberately not persisted.

## 5. Current validation

The dependency-free suite now covers sensor-network detection, automation enable/close actions, turret fire request ordering/ammunition consumption, shield absorption/charge consumption, deterministic power-network identity, and sidecar-v5 defense/automation round trips. `Game.cpp` and `RaylibGraphicsBackend.cpp` pass C++20 syntax checks against a local API-compatible raylib stub. A real raylib/EnTT configure attempt still stops at DNS failure while cloning raylib and is logged as deferred rather than treated as successful native verification.

---

# v0.14 architecture changes

## 1. Part Two development boundary

The v0.13 coworker handoff is treated as **Part One and frozen**. All code in this tree is a separate continuation stream intended to be ported/merged later rather than silently replacing the coworker's migrated build. `PART2_CHANGELOG.md` records only post-v0.13 changes.

## 2. Powered two-door airlock interlock

`SurfaceInfrastructure` now owns stable `SurfaceAirlockAssembly` records linking:

- one `AirlockController` machine;
- one inner Airlock portal;
- one outer Airlock portal;
- one chamber anchor;
- persistent chamber pressure/oxygen;
- a deterministic cycle state.

The controller participates in the existing scalar base-power network at priority 2. A cycle closes both doors before changing chamber pressure. Exterior travel depressurizes before the outer portal may open; interior travel closes the outer portal and repressurizes/oxygenates before the inner portal may open. Brownout is fail-closed and pauses pump progress. Direct manual opening of a portal owned by an assembly is rejected, so the authoritative voxel geometry cannot bypass the interlock.

Current states are `Idle`, `Depressurizing`, `ExteriorOpen`, `Pressurizing`, `InteriorOpen`, and `Fault`. This is intentionally a bounded chamber model, not full gas CFD.

## 3. Atmosphere samples instead of a binary powered flag

`SurfaceInfrastructure::atmosphereAt()` returns local pressure, oxygen, seal/source state, and whether the sample belongs to an airlock chamber. A sealed room that has already been pressurized remains breathable through a short power outage; the Atmosphere Unit no longer needs to be powered every frame for stored air to physically exist.

The spherical player controller now uses partial pressure/oxygen as intermediate protection before full breathable thresholds are reached. Fully breathable air refills suit oxygen; partially supported atmosphere reduces vacuum drain rather than acting as an all-or-nothing switch.

## 4. Sidecar record format v4

The global campaign manifest remains schema **v8**. Spherical touched-chunk sidecars advance from v3 to **v4** and add stable airlock-assembly records: controller ID, inner/outer portal IDs, chamber address, cycle state, pressure, and oxygen. Formats v1-v3 remain readable. The airlock assembly is owned by the chunk containing its chamber anchor.

## 5. Current validation

Headless regression coverage now includes powered controller demand, two-door interlock invariants, depressurize/open and repressurize/open cycles, fail-closed brownout, chamber atmosphere samples, unpowered sealed-room air retention, and sidecar-v4 assembly round trips. Native raylib/EnTT client verification remains deferred and is tracked in `VERIFICATION_LOG.md`.

---

# v0.13 architecture changes

## 1. Explicit serial worker configuration and 0/1/2/4/8 determinism

`JobSystem` now has an explicit `SerialJobs` construction mode. It owns **zero worker threads**, executes submitted work inline, and lets `parallelFor()` take the same deterministic serial path without changing the historical meaning of `JobSystem(0)` (auto-select hardware concurrency).

The headless mesh-capture matrix now compares reconstruction and LOD0 output at **0, 1, 2, 4 and 8 workers**. The same deterministic input currently produces byte-identical captured output across all five configurations.

## 2. Bounded cube-sphere A* Path Request service

`SurfaceNavigationService` replaces the previous one-step left/right avoidance callback with deterministic bounded A* over spherical surface columns. Neighbor expansion uses `PlanetSurface::normalize()`, so routes can cross cube-face seams without a separate seam graph.

The service includes:

- bounded node expansion;
- route caching keyed by start/goal, agent parameters and authoritative world revision;
- deterministic cache eviction;
- conservative cache invalidation on world edits;
- read-only obstacle/height queries through `SurfaceWorldReadService`;
- thread-safe use from concurrent AI Path Request workers.

This is a real route service, but not yet the final kilometre-scale navigation solution. Hierarchical long-distance routing, asynchronous route-request cancellation, dynamic obstacle classes and route reuse across large settlements remain future work.

## 3. Stable Door / Airlock portal objects

The spherical infrastructure layer now owns stable `SurfacePortalObject` records for **Door** and **Airlock**. Closed portals author a solid `DoorPanel`/`AirlockPanel` voxel; opening the portal replaces that voxel with `Air`. Existing collision, sealed-volume, cache invalidation, meshing and navigation therefore react to the same authoritative geometry rather than consulting a parallel fake seal flag.

Portal stable IDs are disjoint from EnTT entities and are saved in the touched chunk that owns their `SurfaceCellAddress`. Sidecar payload format v3 round-trips portal type, address and open state.

Current prototype controls use **B** for Door kits, **0** for Airlock kits, and **E** near a portal to toggle it.

## 4. Bounded persistent room atmosphere state

Each spherical Atmosphere Unit now owns two persistent local scalars:

- `roomPressure` in `[0,1]`;
- `roomOxygen` in `[0,1]`.

Room connectivity remains a bounded `PlanetSurface::sealedVolume()` geometry query. A powered Atmosphere Unit gradually pressurizes and oxygenates a sealed room; an open airlock or structural breach causes the state to vent rapidly. Closing the breach allows the same local state to recover. An intact but unpowered sealed room currently freezes its scalar state rather than simulating leakage.

This intentionally implements the specification's **bounded local pressure/oxygen event** rather than planet-wide fluid dynamics. It does not yet model multiple gases, flow rates through differently sized openings, pressure forces, or a two-door interlocked airlock chamber.

Sidecar payload v3 persists pressure and oxygen with the owning Atmosphere Unit. `roomSealed` is derived from current geometry on update and is not saved.

## 5. Sidecar record format v3

The global campaign manifest remains schema **v8**. The independently versioned spherical touched-chunk payload/transaction format advances to **v3** and adds:

- stable Door/Airlock records;
- persistent Atmosphere Unit pressure/oxygen state.

The reader still accepts sidecar formats v1 and v2; older machine records default pressure/oxygen to zero. Generator version/fingerprint are unchanged because deterministic baseline terrain generation did not change.

---

# v0.12 architecture carried forward

## 1. Shared spherical gameplay world-read substrate

`SurfaceChunkCache` is no longer a renderer-only optimization. `SurfaceWorldReadService` resolves macro and MicroBrick reads through the current immutable reconstructed `SurfaceChunkData` packet whenever the owning chunk is resident and revision-current. Collision, grounded tests, macro/micro raycasts, surface-boundary queries, and AI local steering can therefore consume the same bounded LOD0 residency used by the renderer.

If a requested chunk is absent, still generating, or stale after an edit, the service falls back to deterministic `PlanetSurface` regeneration. This keeps gameplay correct while streaming catches up rather than turning residency misses into holes or false collisions. The service records cache-hit/fallback/stale-query telemetry for headless and future runtime instrumentation.

```text
SurfaceChunkJournal + deterministic fields
                  │
                  ▼
          SurfaceChunkCache
           32³ core + halo
                  │
          ┌───────┼────────┐
          ▼       ▼        ▼
       meshing  collision  raycast / AI steering
                  │
          fallback only while
          packet is absent/stale
```

## 2. Derived `EditInfluenceSummary`

Every touched spherical journal now maintains a compact derived summary: U/V/radial bounds, added/removed solid-cell counts, refined-cell and micro-override counts, maximum outward construction delta, maximum inward excavation depth, and a deterministic significance score. The summary is copied into worker snapshots but is **not** new save-format truth; it is rebuilt from authoritative sparse journal state.

Field LOD uses these summaries to reject unrelated tiles cheaply. More significant edited chunks may refine affected distant tiles to step 1 while ordinary edits continue to use the cheaper influenced step. Returning the chunk fully to deterministic baseline removes both the journal and its summary.

## 3. Surface AI Path Request phase

The surface AI pipeline is now:

```text
OWNER             WORKERS                 WORKERS                  OWNER
EnTT snapshot  →  Sense  →  Think  →  Path Request/steering  →  Commit
```

`Path Request` is currently bounded local obstacle steering rather than a global navigation graph. Workers test read-only spherical world state through `SurfaceWorldReadService`, try the direct tangent candidate, then deterministic left/right avoidance candidates. EnTT/world structural mutation still occurs only during stable-ID owner-thread Commit. The HUD now reports sense/think/path/commit/attack counts separately.

## 4. Narrow deterministic ECS command buffer

`EcsWorld` now has an owner-thread command buffer for spawn/destroy structural operations. Imperial dispatches queue drone spawns and flush them in deterministic enqueue order; phased AI queues dead-enemy destruction and flushes after Commit. Stable IDs are assigned during deterministic command application, remaining independent of transient `entt::entity` values.

This is intentionally the first narrow command-buffer vocabulary, not a claim that every ECS structural operation has migrated yet. Ability/effect/projectile commands and worker-produced command-buffer merging remain future work.

## 5. Spherical recovery remains the default game world

A player failure while using the authoritative cube-sphere now recovers at the spherical ship and remains in spherical gameplay. The planar patch is no longer re-entered as an automatic failure path.

---

# v0.11 architecture carried forward

## 1. Whole-save generation binding — schema v8

v0.10 made each touched spherical chunk independently transactional. v0.11 adds the missing campaign-level identity that binds those sidecars to the global manifest.

Every newly committed save receives a monotonically increasing `save_generation`. The v8 manifest contains that generation, and each spherical chunk transaction stores the same value (introduced in sidecar v2 and retained by current v7). Loading a v8 manifest requires every referenced chunk record to match the manifest generation exactly.

The intended crash model is now:

```text
old committed manifest N
       │
       ├── chunk.current = N
       └── chunk.prev    = N-1

write candidate chunk generation N+1
       │
       ▼
chunk.current = N+1
chunk.prev    = N
       │
       │ crash here? manifest still says N
       │ loader selects chunk.prev generation N
       ▼
atomically publish manifest N+1
       │
       ▼
manifest and referenced current chunks agree on N+1
```

A same-generation retry does **not** rotate away the previous committed sidecar. This prevents repeated attempts at generation N+1 from destroying the generation N fallback before the manifest commits.

Legacy v7 chunk files are represented as `save_generation = 0`, and v1–v7 global saves remain readable through the existing compatibility paths.

### Orphan cleanup

After a successful manifest commit, `pruneSurfaceChunkStore()` keeps sidecars required by both the current and previous recoverable manifests, removes stale temporary files, removes unreferenced current/previous chunk files, and cleans empty planet directories. Cleanup failure is reported but does not retroactively invalidate an already committed save.

A future persistence pass should add an explicit save-generation directory/manifest pointer scheme if profiling or crash-injection shows that the current two-generation file layout is insufficient for large multi-planet saves.

## 2. Localized distant edit influence

Previous field LOD logic promoted an entire edited chunk when any sparse edit existed. v0.11 instead asks `PlanetSurfaceSnapshot::hasEditInfluence()` at the field-patch level.

Normal distant terrain stays at its selected step (`4` for `FieldNear`, `8` for `FieldFar`). Only coarse tiles containing macro or MicroBrick player history subdivide to the configured influenced step (currently `2`). This is a first coarse representation of the Third Edition rule that LOD may reduce detail but must not erase meaningful player construction/excavation.

v0.12 adds a derived `EditInfluenceSummary` with bounds, height/depth and significance, so field LOD can distinguish more meaningful edits without changing save identity. The next step is a still-cheaper coarse distant edit field that can survive far beyond ordinary chunk residency.

## 3. Spherical cached voxel AO

The production cached LOD0 mesher now bakes deterministic corner ambient occlusion for macro and micro geometry. AO samples the outside layer around each face vertex using the reconstructed chunk's one-cell halo, so ordinary chunk boundaries and cube-face seams use the same neighbor information as interior faces.

AO is intentionally a mesh attribute, not a gameplay simulation. It can be tuned or replaced without changing world identity or saves.

## 4. `JobSystem::parallelFor`

The one engine-wide bounded worker pool now exposes a synchronous `parallelFor()` helper for read-heavy deterministic phases. The owner/caller participates, work is partitioned through an atomic index, and nested calls made from an existing worker fall back to serial execution so a bounded pool cannot deadlock waiting on itself.

Headless regression coverage checks deterministic per-index output and nested-worker completion.

## 5. Phased EnTT surface AI

Surface drone updates now have a production-oriented phased path:

```text
OWNER THREAD        WORKERS                 OWNER THREAD
EnTT snapshot  →  Sense  →  Think  →  deterministic Commit
                                            │
                                            ├─ EnTT mutation
                                            ├─ world hover projection
                                            ├─ player damage
                                            └─ entity destruction
```

`EcsWorld::updateSurfaceEnemiesPhased()` copies the required EnTT component data into stable-ID-sorted read records. `Sense` and `Think` operate only on those immutable/plain-data arrays through `JobSystem::parallelFor()`. No worker reads or mutates the EnTT registry. Commit executes in stable-ID order on the owner thread.

v0.12 builds on this boundary with a separate bounded local **Path Request** worker phase backed by shared read-only chunk residency. A global navigation graph remains future work. The HUD exposes per-frame sense/think/path/commit/attack counts.

## 6. Planar failure assumption removed from spherical play

The generic player-failure check no longer treats negative global Y as “fell out of world” while playing on the cube-sphere. A player standing on the lower hemisphere can therefore have negative world Y without triggering the planar recovery path.

---

# Current world / rendering pipeline

`PlanetSurface` stores deterministic generation identity plus sparse per-chunk `SurfaceChunkJournal`s. A fresh planet does not materialize its radial voxel volume. Worker reconstruction produces a disposable `SurfaceChunkData` packet with a 32³ core and a one-cell halo. The packet carries relevant refined MicroBricks and feeds the LOD0 curved mesher directly.

```text
seed + generator contract
          │
          ├──────── deterministic baseline fields
          │
SurfaceChunkJournal (per touched chunk)
          │
          ▼
SurfaceChunkCache
  32³ core + halo
          │
          ▼
Full curved mesh ────────────────────┐
                                     │
Direct field terrain ─ FieldNear/Far ├─ owner-thread GPU publication
                                     │
Climate + cloud orbital shell ──────┘
```

Surface play currently budgets seven nearest logical chunks at full editable detail and uses lower-detail field packets beyond them. The renderer's prefetch candidates are derived from tangential neighbors and wrap through cube-face ownership at seams.

The orbital view uses dedicated deterministic climate/cloud shell meshes and does not request voxel LOD0 packets.

# Current gameplay systems on `PlanetSurface`

The spherical substrate already supports volumetric macro collision, radial gravity, tangent movement, macro and micro raycasts, persistent 16³ micro-sculpting, sparse building/mining edits, stable spherical machines, scalar power networks, batteries, Burner fuel, sealed-room detection, powered atmosphere, crafting, ship resupply/travel, per-system Suspicion, Registry Beacon claims, EnTT surface drones, and Register-Action-style Imperial waves.

The three vertical-slice worlds remain Temperate, Barren/vacuum, and Scorched/hazardous.

# Persistence model

Current writer: **schema v8**.

- global `elysium_save.txt`: player/campaign compatibility state + `save_generation` + stable spherical chunk references;
- `elysium_save.txt.prev`: previous known-good global generation;
- `elysium_save_chunks/planet_N/<key>.chunk`: current touched-chunk transaction;
- `<key>.chunk.prev`: prior recoverable touched-chunk generation.

Chunk records include planet identity, generator version/fingerprint, `save_generation`, stable `PlanetChunkAddress`, macro overrides, placement markers, MicroBrick state, stable surface-machine state (including bounded pressure/oxygen), and stable Door/Airlock portal state. Payload length/checksum, temporary publication, durable flush where supported, and previous-generation fallback are implemented.

Generated baseline chunks are never saved merely because they were visited or cached.

# Validation status

The dependency-free suite covers deterministic generation, cube-sphere seams, sparse journals, MicroBricks, macro/micro collision/raycasting, sealed atmosphere, timed pressurization/vent/repressurization, Door/Airlock seal behavior and persistence, bounded spherical A* seam crossing/cache invalidation/detours, explicit 0/1/2/4/8 worker determinism, base power, cache budgets/cancellation/eviction, halo reconstruction, cached/reference mesh topology, spherical AO presence, localized field edit influence, LOD tiers, orbital shells, fake graphics lifetime, save-generation matching, previous-generation recovery, same-generation retry safety, manifest reference extraction, and orphan cleanup.

Real raylib + EnTT client compilation, graphical save/load, GPU visuals, platform crash injection, and target-hardware performance remain external/deferred checks. **See `VERIFICATION_LOG.md`; do not infer that a deferred item passed.**

# Important remaining construction work

The next highest-value dependencies are:

1. grow the bounded A* service into hierarchical long-distance spherical navigation with asynchronous route-request cancellation and dynamic-obstacle handling;
2. broaden deterministic ECS command-buffer publication to abilities, projectiles, effects, and worker-produced command merges;
3. improve curved LOD stitching beyond conservative skirts and evaluate tessellated/curvature-safe greedy merging rather than flattening large spherical quads;
4. use `EditInfluenceSummary` to drive an explicit persistent/coarse distant edit field for skyline-scale construction and excavation;
5. extend bounded environment simulation with airlock interlocks, pressure differential effects, ducts/filtration and local fluid records without becoming planet-wide CFD;
6. finish removing normal-game dependencies on the planar compatibility `World`;
7. expand the vertical slice with authored POIs, first procedural fauna set, base defense, and the full early metallurgy loop before broad galaxy content.

Broader Third Edition systems—full galaxy distribution, weather/ecology breadth, structural collapse, logistics/automation, complete RPG/passive/gear systems, Court, settlements, vehicles, derelicts, research/economy, and networking—remain planned construction scope rather than fake placeholders.

## External verification

`VERIFICATION_LOG.md` is a first-class release artifact. Share it directly with anyone evaluating the project; it contains IDs, status, reproduction procedures, expected results, and a finding template.
