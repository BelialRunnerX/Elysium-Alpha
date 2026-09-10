# Elysium Production Build Graph — 2026-09-09

This tree is the production-compile reconciliation of the recovered Elysium merge archive.

## Source coverage invariant

The repository contains **1,612 recovered `.cpp` translation units**:

- `src/`: 1,588 files total — `src/main.cpp` plus **1,587 runtime modules**.
- `tests/`: **22** files — every file is compiled; main-bearing tests become individual CTest executables and support files become `elysium_test_support`.
- `tools/`: **2** files — each is compiled as a separate developer executable.

The final `Elysium.exe` directly receives all 1,587 non-main runtime object files plus `src/main.cpp`. They are not hidden behind a static-library dead-strip boundary, so duplicate definitions and unresolved production symbols are link failures. Test/tool entry points cannot legally be linked into the same executable because they have their own `main()` functions; they are nevertheless mandatory members of the same default build graph.

`cmake/ProductionSources.cmake` is the explicit authoritative inventory. Configure fails if a `.cpp` is added, removed, or left unclassified. Regenerate it with:

```text
python tools/generate_production_sources.py
```

## Windows production build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DELYSIUM_BUILD_CLIENT=ON `
  -DELYSIUM_BUILD_TESTS=ON `
  -DELYSIUM_BUILD_TOOLS=ON `
  -DELYSIUM_FETCH_DEPENDENCIES=ON `
  -DELYSIUM_ENFORCE_SOURCE_COVERAGE=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
cmake --install build --config Release --prefix dist/Elysium
```

The dependency pins are Raylib **5.5** and EnTT **3.13.2**, fetched by CMake for the real client. The installed layout puts `Elysium.exe`, `assets/`, `config/`, and `licenses/` together so the portable artifact does not require the source repository.

## Reconciliation changes made for compile readiness

The production merge repaired generated reference/member syntax across the staged modules, reconciled old/new renderer interfaces (draw parameters, textures, batched instance fallback), restored missing diagnostic compatibility accessors without changing cube-sphere ownership semantics, appended `MachineType::ArcSmelter` without renumbering persisted machine IDs, appended `BlockType::Rubble` without renumbering existing block IDs, and completed the recovered eight-value `PlanetClass` presentation/environment switches.

The authoritative planet rules remain address-keyed, use direction-space cube-sphere ownership, preserve bounded sealed-volume behavior, and do not serialize transient ECS or graphics handles as persistent identity.

## Validation state

A dependency-stub syntax sweep was run over all recovered runtime translation units in this environment and compile defects found by that sweep were reconciled. The real Windows MSVC compile/link, real Raylib/EnTT integration, CTest execution, package construction, and checksum are deliberately enforced by `.github/workflows/windows-build.yml`; those are the authoritative external build proof.

The CI workflow fails if source coverage changes, configuration fails, any production translation unit fails to compile, the full runtime fails to link, tests fail, the EXE is not installed, required canonical asset packs are absent, or artifact creation fails.

## Git repository asset-pack policy

The compile-ready Git tree enforces all 1,612 recovered C++ translation units.
The two large canonical asset ZIPs are release/runtime payloads, not compilation
inputs, and are intentionally excluded from the bootstrap materialization used
to seed `production-build`. Their expected names are documented under
`assets/packs/README.md`. A Windows CI compile can therefore prove configure,
compile, link, CTest, and portable EXE installation independently of those large
content archives. A **final distributable** still requires the canonical packs
(or their unpacked production successors) to be attached/copied and checksum
verified before release sign-off.

Canonical production-merge asset ZIP checksums:

- `93563e38b58de2963d4f6f396b795c5409f4ed471a9700faee9bfd72efc13011`  `Elysium_GameReady_AssetLibrary_Complete_03-05_2026-09-09.zip`
- `16ce5338ead050cf105d90e21ce7061b9ec7fd101c403f236ea4435cfe2cf536`  `Elysium_VisualImplementation_MasterPack_ConceptWorkflow_2026-09-09.zip`
