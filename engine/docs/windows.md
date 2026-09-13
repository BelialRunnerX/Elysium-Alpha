# Building on Windows

Everything here has been compiled and tested on Linux. The Windows path is
written and its two platform-specific pieces are real code — the GL entry-point
loader and the CMake branches — but **it has never been built on Windows**,
because the machine it was developed on has no Windows toolchain and no shell on
yours. Expect the first build to produce errors. Paste them back and they get
fixed; that loop is the plan, not a fallback.

## What you need

1. **Visual Studio 2022** with the *Desktop development with C++* workload.
   The free Community edition is fine. Build Tools alone also works.
2. **vcpkg**, for zlib and GLFW:

   ```powershell
   git clone https://github.com/microsoft/vcpkg C:\vcpkg
   C:\vcpkg\bootstrap-vcpkg.bat
   [Environment]::SetEnvironmentVariable('VCPKG_ROOT', 'C:\vcpkg', 'User')
   ```

   Open a **new** terminal after that so `VCPKG_ROOT` is set.

   `vcpkg.json` in the project root lists the dependencies, so vcpkg installs
   them automatically during configure. There is nothing to `vcpkg install` by
   hand.

## Build

From the project root:

```powershell
cmake --preset windows
cmake --build --preset windows
```

The first configure takes a few minutes while vcpkg builds zlib and GLFW. After
that it is seconds.

The binaries land in `build\Release\`.

## Run

```powershell
.\build\Release\elysium.exe 23
```

WASD to move, mouse to look, space and left-control for up and down, shift to
sprint, Esc to quit. The window title shows frame rate, tile counts, triangles
and streaming cost.

The seed is the planet. `23` is a Temperate world; `systemsurvey.exe` lists
others:

```powershell
.\build\Release\systemsurvey.exe 1234
.\build\Release\planetmap.exe 23 out      # maps and a shaded globe as PNGs
.\build\Release\genbench.exe 23           # where generation time goes
```

## What does not build on Windows, and why

`glview` and `flythrough` are the headless rendering tools. They create their
OpenGL context through **EGL's surfaceless platform**, which does not exist on
Windows — there is no way to get a GL context there without a window. CMake
skips them automatically.

Nothing is lost from the engine: the windowed application runs the identical
`WorldView`, and every generator tool (`planetmap`, `crosssection`, `voxelview`,
`lodview`, `systemsurvey`, `genbench`) builds and runs normally. The unit tests
build and run too:

```powershell
ctest --test-dir build -C Release
```

## The one piece written specifically for Windows

`opengl32.dll` exports OpenGL 1.1 and nothing later. Every call this renderer
makes — `glCreateShader`, `glGenVertexArrays`, `glBufferData`, all of it — has to
be fetched at run time, after a context is current. Worse, `wglGetProcAddress`
returns null for the 1.1 functions and `GetProcAddress` on the DLL returns null
for everything newer, so a correct loader has to try both.

`src/gl/glcore.h` declares the entry points as function pointers on Windows and
as ordinary functions elsewhere, from a single list, and `src/gl/glloader.cpp`
fills them in using a getter the caller supplies. The application passes
`glfwGetProcAddress`, which already handles the two-source lookup. On Linux the
loader is a no-op that returns true, so there is no platform branch in the
calling code.

If the context comes up but nothing renders, that loader is the first place to
look — it names the first entry point it could not resolve.

## Likely first-build problems

Written down because they are predictable, not because they have been seen:

- **`VCPKG_ROOT` not set** — configure fails to find the toolchain file. Open a
  new terminal after setting it.
- **`std::max` / `min` macros** — `NOMINMAX` is already defined in
  `CMakeLists.txt`; if something still trips over it, that is where to look.
- **`M_PI` undefined** — MSVC does not define it without `_USE_MATH_DEFINES`.
  The engine defines its own `kPi` constants and should not hit this, but a
  tool might.
- **Warnings as noise** — MSVC is set to `/W3`, not `/W4`, deliberately. If
  something warns loudly it is worth reading rather than silencing.
- **The window opens black** — almost certainly the GL loader or the shader
  compile. Both print a specific message to the console; run from a terminal
  rather than by double-clicking so you can see it.
