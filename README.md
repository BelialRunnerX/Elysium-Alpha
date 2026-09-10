# Elysium

**Unified monorepo** for the Elysium open-world game — Sleeping Empire universe.

No Man's Sky × Minecraft × Dwarf Fortress × Factorio × Diablo.

## What's in this repo

| Path | Source | Role |
|------|--------|------|
| [`game/`](game/) | `production-build` | **Canonical C++20 native client** (EnTT + raylib, cube-sphere planets, industry, siege, save v8+) |
| [`engine/`](engine/) | `elysium-game` | Lower-level OpenGL engine core (headless-testable meshing / world layer) |
| [`engine-prototype/`](engine-prototype/) | `Elysium-game-dev` | Active prototype baseline (v0.20 vertical slice contracts) |
| [`play/`](play/) | *new* | High-fidelity **browser open-world** client (WebGL) — playable now |
| [`mods/mono/`](mods/mono/) | `Elysium-mono` | Minecraft NeoForge mod series (lib + core + dungeons + mobs + npcs + trinkets) |
| [`mods/core/`](mods/core/) | `elysium-core-mod` | Core content mod (races, classes, runes, gear) |
| [`mods/neoforge/`](mods/neoforge/) | `Elysium-Neoforge` | Earlier Forge progression skeleton |
| [`docs/lore/`](docs/lore/) | `the-sleeping-empire` | World bible / lore |
| [`docs/architecture/`](docs/architecture/) | `elysium` | Quartz design vault |
| [`docs/design/`](docs/design/) | game-dev docs | Design locks & edition specs |
| [`archives/`](archives/) | `elysium-source`, `elysium-menu-repo` | Historical pointers |

See [`SOURCES.md`](SOURCES.md) for provenance of every merged repository.

## Play now (browser)

```bash
cd play && python3 -m http.server 8080
# open http://localhost:8080
```

WASD move · mouse look · Space jump · LMB mine · RMB place · 1–8 hotbar · T travel · M map · Esc menu

## Build the native client

```bash
cd game
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/Elysium
```

Windows (VS 2022+): see [`game/PRODUCTION_BUILD.md`](game/PRODUCTION_BUILD.md).

## Design locks

- Synthesis: NMS × Minecraft × **DF depth** × **Factorio logistics** × Diablo combat fantasy
- Cube-sphere planets, sparse journals, StableIds ≠ `entt::entity`
- Art: weathered anime operatives, white/black/cyan Imperial accents
- Lore: Sleeping Empire / Absolutionism / Nullspire

## Status

v0.25 Alpha — consolidated monorepo + cinematic open-world presentation pass on the playable cube-sphere vertical slice (native) and a full browser open-world client.
