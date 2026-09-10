cmake_minimum_required(VERSION 3.16)
project(elysium CXX)

# ---------------------------------------------------------------------------
# Module layout.
#
#   ely_core ──> ely_world ──> ely_mesh ──> ely_gfx ──> ely_gl ──> ely_engine
#                                             ^           ^
#   ely_image ────────────────────────────────┴───────────┘
#
# Each module is a real target that declares what it depends on, and the
# dependencies only ever point one way. That is not bookkeeping: the test
# executables link *one module each*, so a header that reaches sideways or
# upwards fails to link rather than quietly becoming load-bearing. The layering
# is checked by the build rather than by discipline.
#
#   ely_core    seeding, vectors, noise. Depends on nothing.
#   ely_jobs    the thread pool and parallelFor. Depends only on core.
#   ely_save    the per-chunk edit journal: what the player changed, and only
#               that. The world itself is never stored — it is regenerated.
#   ely_world   planets, materials, terrain, chunks, LOD. The generator.
#   ely_mesh    greedy meshing, ambient occlusion, visibility culling.
#   ely_image   PNG writing and a software rasteriser. Depends only on zlib.
#   ely_gfx     camera, frustum, GPU vertex format, shading, LOD streaming,
#               tile building. Knows about the world; knows nothing about a
#               graphics API.
#   ely_gl      the OpenGL backend. The only module that touches GL.
#   ely_engine  the loop: streaming, worker threads, draw submission.
# ---------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()
if(MSVC)
  # /Zc:__cplusplus  — without it MSVC reports __cplusplus as 199711 forever,
  #                    and libraries that feature-test on it (EnTT among them)
  #                    silently take their oldest code path.
  # /bigobj          — the ECS translation units instantiate a lot of templates
  #                    and overflow the default section limit.
  # /permissive-     — two-phase lookup, so a template error is an error here
  #                    rather than a surprise on the other compiler.
  add_compile_options(/W3 /permissive- /Zc:__cplusplus /bigobj /MP)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
else()
  add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

option(ELY_FETCH_DEPENDENCIES
       "Download missing dependencies from the internet as a last resort" OFF)

find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

# --- ely_core ---------------------------------------------------------------
add_library(ely_core INTERFACE)
target_include_directories(ely_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- ely_world --------------------------------------------------------------
add_library(ely_world
  src/world/planet.cpp
  src/world/material.cpp
  src/world/terrain.cpp
)
target_link_libraries(ely_world PUBLIC ely_core)

# --- ely_mesh ---------------------------------------------------------------
add_library(ely_mesh
  src/mesh/greedy.cpp
  src/mesh/visibility.cpp
)
target_link_libraries(ely_mesh PUBLIC ely_world)

# --- ely_image --------------------------------------------------------------
add_library(ely_image src/image/image.cpp)
target_link_libraries(ely_image PUBLIC ely_core ZLIB::ZLIB)

# --- ely_gfx ----------------------------------------------------------------
add_library(ely_gfx src/gfx/tilebuild.cpp)
target_link_libraries(ely_gfx PUBLIC ely_mesh ely_jobs ely_save)

# --- ely_save ---------------------------------------------------------------
#
# zlib is here for crc32, not compression: every journal record is checksummed
# so that a torn write from a crash is detectable rather than plausible.
add_library(ely_save src/save/editstore.cpp)
target_link_libraries(ely_save PUBLIC ely_world ZLIB::ZLIB)

# --- ely_jobs ---------------------------------------------------------------
add_library(ely_jobs src/jobs/jobsystem.cpp)
target_link_libraries(ely_jobs PUBLIC ely_core Threads::Threads)

# --- ely_ecs ----------------------------------------------------------------
#
# EnTT is a single header. It is looked for in several places rather than
# fetched, because the machine this was written on has no network access to
# GitHub and vendoring a 3.6 MB header into the tree is worse than finding the
# one already on disk.
#
#   1. -DENTT_INCLUDE_DIR=<dir containing entt/entt.hpp>
#   2. third_party/entt/entt.hpp, vendored beside the source
#   3. ../single_include/entt/entt.hpp, which is the layout of a cloned repo
#      when this project sits inside it
#   4. an installed EnTT package
# An installed package first — this is what vcpkg, Conan and a system install
# all provide, and it is what CI uses.
find_package(EnTT CONFIG QUIET)

if(NOT EnTT_FOUND)
  # Then a header already on disk. Both of these matter for working without a
  # network: `third_party/` for a vendored copy, and `../single_include/` for
  # the layout of a cloned EnTT repo with this project sitting inside it.
  find_path(ELY_ENTT_INCLUDE entt/entt.hpp
    HINTS ${ENTT_INCLUDE_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}/third_party
          ${CMAKE_CURRENT_SOURCE_DIR}/../single_include
          ${CMAKE_CURRENT_SOURCE_DIR}/../../single_include
    NO_DEFAULT_PATH)
  if(NOT ELY_ENTT_INCLUDE)
    find_path(ELY_ENTT_INCLUDE entt/entt.hpp)
  endif()
endif()

# And only then the network. Last resort on purpose: a build that reaches out
# to the internet is a build that fails on a train, and this project was
# written somewhere with no route to GitHub at all.
if(NOT EnTT_FOUND AND NOT ELY_ENTT_INCLUDE AND ELY_FETCH_DEPENDENCIES)
  include(FetchContent)
  # Tracking master rather than a tag, deliberately and temporarily. The
  # version this engine was written and tested against is EnTT 4.0.0, which at
  # the time of writing lives on master and has no release tag to pin to. An
  # unpinned dependency is a real cost — a build that worked yesterday can break
  # today — so this should become a tag the moment there is one. It is here
  # rather than silently pinned to a v3 tag because guessing at a version that
  # has never been compiled against would move the failure from "configure
  # step" to "somebody else's machine".
  FetchContent_Declare(EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG master
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(EnTT)
  set(EnTT_FOUND TRUE)
  message(STATUS "EnTT fetched from GitHub")
endif()

if(EnTT_FOUND OR ELY_ENTT_INCLUDE)
  add_library(ely_ecs src/ecs/systems.cpp)
  if(EnTT_FOUND)
    target_link_libraries(ely_ecs PUBLIC ely_gfx EnTT::EnTT)
    message(STATUS "EnTT: package")
  else()
    target_include_directories(ely_ecs PUBLIC ${ELY_ENTT_INCLUDE})
    target_link_libraries(ely_ecs PUBLIC ely_gfx)
    message(STATUS "EnTT: header at ${ELY_ENTT_INCLUDE}")
  endif()
  set(ELY_HAVE_ECS TRUE)
else()
  message(STATUS "EnTT not found: the engine and the application will not be"
                 " built. Install it, pass -DENTT_INCLUDE_DIR=<dir containing"
                 " entt/entt.hpp>, or configure with"
                 " -DELY_FETCH_DEPENDENCIES=ON. The generator, mesher and"
                 " renderer modules do not need it.")
endif()

# --- ely_gl and ely_engine --------------------------------------------------
#
# On Linux these link the versioned sonames directly. The usual
# find_package(OpenGL) wants the libGL.so/libEGL.so development symlinks, which
# a runtime-only Mesa install does not ship; the .so.1 files are what actually
# exist and are what the loader resolves at run time anyway.
if(WIN32)
  set(ELY_GL_LIBS opengl32)
  set(ELY_HAVE_GL TRUE)
else()
  # The plain names come from a -dev package; the versioned sonames are what a
  # runtime-only Mesa install actually ships, and are what the loader resolves
  # at run time anyway. Looking for both means the build works on a developer
  # machine and on a bare container.
  find_library(ELY_GL_LIB NAMES GL libGL.so.1
               HINTS /usr/lib/x86_64-linux-gnu /usr/lib64 /usr/lib
                     /usr/lib/aarch64-linux-gnu)
  find_library(ELY_EGL_LIB NAMES EGL libEGL.so.1
               HINTS /usr/lib/x86_64-linux-gnu /usr/lib64 /usr/lib
                     /usr/lib/aarch64-linux-gnu)
  if(ELY_GL_LIB AND ELY_EGL_LIB)
    set(ELY_GL_LIBS ${ELY_GL_LIB} ${ELY_EGL_LIB})
    set(ELY_HAVE_GL TRUE)
  endif()
endif()

if(ELY_HAVE_GL)
  # glcontext.cpp is the headless EGL context and is POSIX-only: Windows has
  # no EGL, and a window there comes from GLFW instead. The renderer itself is
  # identical on both — it never creates a context, it is handed one.
  set(ELY_GL_SOURCES src/gl/glrenderer.cpp src/gl/glloader.cpp)
  if(NOT WIN32)
    list(APPEND ELY_GL_SOURCES src/gl/glcontext.cpp)
  endif()
  add_library(ely_gl ${ELY_GL_SOURCES})
  target_link_libraries(ely_gl PUBLIC ely_gfx ely_image ${ELY_GL_LIBS})

  if(ELY_HAVE_ECS)
    add_library(ely_engine src/engine/worldview.cpp)
    target_link_libraries(ely_engine PUBLIC ely_gl ely_ecs Threads::Threads)
    set(ELY_HAVE_ENGINE TRUE)
  endif()
  message(STATUS "OpenGL backend enabled")
else()
  message(STATUS "OpenGL backend skipped: no GL library found")
endif()

# ---------------------------------------------------------------------------
# Tools — headless inspection of every stage (principle S6).
# ---------------------------------------------------------------------------
foreach(tool planetmap crosssection systemsurvey)
  add_executable(${tool} tools/${tool}.cpp)
  target_link_libraries(${tool} PRIVATE ely_world ely_image)
endforeach()

foreach(tool voxelview lodview)
  add_executable(${tool} tools/${tool}.cpp)
  target_link_libraries(${tool} PRIVATE ely_mesh ely_image)
endforeach()

# genbench builds whole tiles, so it needs the tile builder and the pool.
add_executable(genbench tools/genbench.cpp)
target_link_libraries(genbench PRIVATE ely_gfx ely_image)

if(ELY_HAVE_GL)
  # The headless tools need the EGL context, so they are POSIX-only. On
  # Windows the windowed application is the way to see the renderer; the
  # generator tools above all still build and run.
  if(NOT WIN32)
    add_executable(glview tools/glview.cpp)
    target_link_libraries(glview PRIVATE ely_gl)

    if(ELY_HAVE_ENGINE)
      add_executable(flythrough tools/flythrough.cpp)
      target_link_libraries(flythrough PRIVATE ely_engine)
    endif()

    add_executable(savedemo tools/savedemo.cpp)
    target_link_libraries(savedemo PRIVATE ely_gl)
  endif()

  # --- The windowed application ---------------------------------------------
  #
  # Optional, because the engine is fully exercisable without a window: the
  # flythrough tool drives the same WorldView against a real driver, headless.
  # If GLFW is not installed this simply is not built and nothing else changes.
  #
  #   Linux:   apt install libglfw3-dev
  #   Windows: vcpkg install glfw3   (see docs/windows.md)
  #   macOS:   brew install glfw
  find_package(glfw3 QUIET)
  if(glfw3_FOUND AND ELY_HAVE_ENGINE)
    add_executable(elysium app/elysium.cpp)
    target_link_libraries(elysium PRIVATE ely_engine glfw)
    message(STATUS "Windowed application enabled")
  else()
    message(STATUS "Windowed application skipped: GLFW not found"
                   " (the headless tools do not need it)")
  endif()
endif()

# ---------------------------------------------------------------------------
# Visual Studio ergonomics.
#
# The generated solution is what somebody double-clicks, and by default it opens
# on an arbitrary target with every file in one flat list. These three lines are
# the difference between that and a solution that behaves like a project.
# ---------------------------------------------------------------------------
if(MSVC)
  # Files appear in the solution laid out the way they are on disk, by module,
  # rather than as one flat list of sixty.
  get_property(ELY_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
  foreach(target ${ELY_TARGETS})
    get_target_property(type ${target} TYPE)
    if(NOT type STREQUAL "INTERFACE_LIBRARY")
      get_target_property(sources ${target} SOURCES)
      if(sources)
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})
      endif()
      # Libraries and tools into folders, so the solution tree is readable.
      if(type STREQUAL "EXECUTABLE")
        set_target_properties(${target} PROPERTIES FOLDER "tools")
      else()
        set_target_properties(${target} PROPERTIES FOLDER "modules")
      endif()
    endif()
  endforeach()
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # F5 runs the game, from the source directory, with a seed — rather than
  # running whichever target CMake happened to define first, from the build
  # directory, with no arguments.
  if(TARGET elysium)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT elysium)
    set_target_properties(elysium PROPERTIES
      VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      VS_DEBUGGER_COMMAND_ARGUMENTS "23")
  elseif(TARGET planetmap)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT planetmap)
  endif()

  # The tools write PNGs into ./out relative to the working directory.
  foreach(target planetmap crosssection voxelview lodview genbench savedemo)
    if(TARGET ${target})
      set_target_properties(${target} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        VS_DEBUGGER_COMMAND_ARGUMENTS "23 out")
    endif()
  endforeach()
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)

# ---------------------------------------------------------------------------
# Tests.
#
# One executable per module, linking only that module. If ely_gfx ever needs
# something from ely_gl, test_gfx stops linking — which is the point.
# ---------------------------------------------------------------------------
enable_testing()
add_library(ely_testharness tests/harness.cpp)
target_include_directories(ely_testharness PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_executable(test_world tests/test_world.cpp)
target_link_libraries(test_world PRIVATE ely_world ely_testharness)
add_test(NAME world COMMAND test_world)

add_executable(test_mesh tests/test_mesh.cpp)
target_link_libraries(test_mesh PRIVATE ely_mesh ely_testharness)
add_test(NAME mesh COMMAND test_mesh)

add_executable(test_save tests/test_save.cpp)
target_link_libraries(test_save PRIVATE ely_save ely_testharness)
add_test(NAME save COMMAND test_save)

add_executable(test_jobs tests/test_jobs.cpp)
target_link_libraries(test_jobs PRIVATE ely_jobs ely_testharness)
add_test(NAME jobs COMMAND test_jobs)

add_executable(test_gfx tests/test_gfx.cpp)
target_link_libraries(test_gfx PRIVATE ely_gfx ely_testharness)
add_test(NAME gfx COMMAND test_gfx)

if(ELY_HAVE_ECS)
  add_executable(test_ecs tests/test_ecs.cpp)
  target_link_libraries(test_ecs PRIVATE ely_ecs ely_testharness)
  add_test(NAME ecs COMMAND test_ecs)
endif()

if(ELY_HAVE_GL AND NOT WIN32)
  # glview is a test as much as a tool: it brings up a real driver, compiles the
  # real shaders, draws the real scene and checks the result against the
  # software reference. It exits non-zero if any stage fails.
  add_test(NAME opengl COMMAND glview 23 ${CMAKE_CURRENT_BINARY_DIR})
  set_tests_properties(opengl PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1")

  # savedemo is a test too: it exits non-zero unless the world it left and the
  # world it came back to are pixel-identical.
  add_test(NAME saveroundtrip COMMAND savedemo 23 ${CMAKE_CURRENT_BINARY_DIR})
  set_tests_properties(saveroundtrip PROPERTIES
                       ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1")
endif()
