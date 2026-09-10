# Opening this in Visual Studio

Two ways. The first is less setup; the second gives you a `.sln`.

## 1. Open the folder (recommended)

Visual Studio 2022 understands CMake natively and reads `CMakePresets.json`
from this folder.

**File → Open → Folder…** and pick the project root.

VS finds the `windows` preset, configures, and gives you the full IDE — IntelliSense,
breakpoints, the Test Explorer, the lot. Pick the `windows` configuration from
the toolbar dropdown if it does not select it for you.

## 2. Generate a solution

Double-click **`open-in-visual-studio.bat`**. It configures with the `windows`
preset (which uses the Visual Studio 2022 generator) and opens the resulting
`build\elysium.sln`.

Equivalently, by hand:

```powershell
cmake --preset windows
start build\elysium.sln
```

## Before either one: vcpkg

zlib, GLFW and EnTT come from vcpkg. Once, ever:

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
setx VCPKG_ROOT C:\vcpkg
```

Then **open a new terminal** so `VCPKG_ROOT` is set, and configure. `vcpkg.json`
lists the dependencies, so they install themselves during the first configure —
which takes a few minutes, and seconds after that.

If you would rather not use vcpkg, point CMake at an EnTT header you already
have:

```powershell
cmake --preset windows -DENTT_INCLUDE_DIR="C:\path\to\entt\single_include"
```

## What you get

The solution is laid out to be usable rather than merely to exist:

- **Files are grouped by module**, mirroring the folders on disk, instead of
  sixty files in one flat list.
- **Targets are in folders** — `modules` for the libraries, `tools` for the
  executables.
- **F5 runs the game.** `elysium` is the startup project, its working directory
  is the source root, and it is already passing seed `23`.
- **The tools have working directories set too**, so `planetmap`, `lodview`,
  `savedemo` and friends write their PNGs into `out\` where you can find them.
- **Test Explorer** picks up all eight suites through CTest.

## If EnTT is missing

The build does not fail — it drops `ely_ecs`, `ely_engine` and the application,
tells you so, and builds everything else. The generator, the mesher, the
renderer and their tools do not depend on it. That is deliberate: a missing
optional dependency should cost you the part that needs it, not the afternoon.

## Things that will probably go wrong first

Written down because they are predictable, not because they have been seen —
**this project has never been compiled on Windows.**

- **`VCPKG_ROOT` not set** — configure cannot find the toolchain file. Open a
  new terminal after `setx`.
- **A GL symbol redefinition** — something included a system GL header.
  `src/gl/glcore.h` declares the modern entry points as function pointers,
  because `opengl32.dll` exports only OpenGL 1.1; anything that also pulls in
  `<GL/gl.h>` collides with it. `app/elysium.cpp` defines `GLFW_INCLUDE_NONE`
  for exactly this reason.
- **The window opens black** — almost certainly the GL loader or the shader
  compile. Both print a specific message; run from a terminal rather than
  double-clicking so you can read it.
- **EnTT template errors** — the version from vcpkg may be 3.x while this was
  written against 4.0. The ECS code sticks to long-stable API, but if this is
  where it breaks, the message will say so clearly and it is a small surface to
  fix.
