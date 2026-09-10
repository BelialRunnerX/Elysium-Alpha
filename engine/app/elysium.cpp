// The windowed application: fly around a planet.
//
// This is the thinnest file in the engine on purpose. Everything it does —
// streaming, LOD, building, culling, drawing — is WorldView, which the
// flythrough tool drives headlessly and which is therefore tested. All that is
// here is a window, a context and a keyboard.
//
// NOTE ON STATUS. This is the one file in the project that has never been
// compiled: the environment it was written in has no GLFW headers and no
// display, so there was no way to build it, let alone open a window. Everything
// it calls is exercised by tools/flythrough.cpp against a real OpenGL driver.
// Treat the twenty lines of GLFW below as a careful first draft; the rest has
// been run.
//
// Controls: WASD to move, mouse to look, space/ctrl for up and down, shift to
// sprint, F to toggle wireframe-ish face culling, Esc to quit.
#include "gl/glcore.h"
#include "gl/glrenderer.h"
#include "engine/worldview.h"
#include "world/terrain.h"

// GLFW must not drag in a system GL header.
//
// Without this, <GLFW/glfw3.h> includes <GL/gl.h>, which on Windows declares
// glGetString and friends as *functions* exported by opengl32 — while
// gl/glcore.h declares the same names as function pointers, because opengl32
// exports OpenGL 1.1 and nothing this renderer uses. The two collide and
// nothing compiles. GLFW_INCLUDE_NONE tells GLFW to define its own types and
// leave GL alone, which is what a project with its own loader wants.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>
#include <cmath>
#include <cstdio>
#include <cstdlib>

using namespace ely;

namespace {

constexpr double kPi = 3.14159265358979;

WorldView* gView = nullptr;
double gLastX = 0, gLastY = 0;
bool gFirstMouse = true;

void onMouse(GLFWwindow*, double x, double y) {
    if (!gView) return;
    if (gFirstMouse) { gLastX = x; gLastY = y; gFirstMouse = false; }
    const double dx = x - gLastX, dy = y - gLastY;
    gLastX = x; gLastY = y;
    // Yaw increases to the left because the camera looks down -Z at yaw 0 and
    // the right vector is +X; moving the mouse right should turn right.
    gView->camera.yaw -= dx * 0.0025;
    gView->camera.pitch -= dy * 0.0025;
    gView->camera.clampPitch();
}

}  // namespace

int main(int argc, char** argv) {
    const uint64_t seed = argc > 1 ? std::strtoull(argv[1], nullptr, 0) : 23;

    if (!glfwInit()) {
        std::fprintf(stderr, "glfwInit failed\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(1600, 900, "Elysium", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "could not create a window with an OpenGL 3.3 core "
                             "context\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Resolve the GL entry points. On Windows this is mandatory — opengl32.dll
    // exports OpenGL 1.1 and nothing this renderer uses — and it must happen
    // after the context is current. Elsewhere it is a no-op that returns true,
    // so there is no platform test here.
    std::string loadError;
    if (!loadGlFunctions(reinterpret_cast<void* (*)(const char*)>(
                             glfwGetProcAddress),
                         &loadError)) {
        std::fprintf(stderr, "%s\n", loadError.c_str());
        glfwTerminate();
        return 1;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, onMouse);

    GlRenderer renderer;
    std::string error;
    if (!renderer.init(&error)) {
        std::fprintf(stderr, "renderer init failed:\n%s\n", error.c_str());
        return 1;
    }

    PlanetParams params = generatePlanetFromSeed(seed);
    Terrain terrain(params);
    const double surface = terrain.surfaceAltitude(FACE_PZ, 0.0, 0.0);

    WorldViewConfig cfg;
    cfg.face = FACE_PZ;
    // One worker per core, less the main thread and a little headroom. Tile
    // building is pure and embarrassingly parallel, so this scales until the
    // upload budget or memory bandwidth becomes the limit.
    cfg.workers = std::max(2, int(std::thread::hardware_concurrency()) - 2);
    cfg.maxUploadsPerFrame = 8;
    cfg.streamer.tileSize = 32.0;
    cfg.streamer.viewDistance = 384.0;
    cfg.streamer.nearDistance = 48.0;
    cfg.streamer.finestLod = 0;
    cfg.streamer.coarsestLod = 5;
    cfg.streamer.verticalTiles = 1;
    cfg.streamer.maxLoadsPerUpdate = 64;

    WorldView view;
    view.init(terrain, renderer, cfg);
    view.camera.position = Vec3{0.0, surface + 3.0, 0.0};
    view.camera.yaw = kPi;
    view.camera.zNear = 0.15;
    view.camera.zFar = 1600.0;
    gView = &view;

    std::printf("planet %llu: %s, radius %.0f m, gravity %.2f g\n",
                (unsigned long long)seed, planetClassName(params.cls),
                params.radius, params.gravity);
    std::printf("WASD to move, mouse to look, space/ctrl for up and down,"
                " shift to sprint, Esc to quit\n");

    const float clear[3] = {18 / 255.0f, 20 / 255.0f, 28 / 255.0f};
    auto last = std::chrono::steady_clock::now();
    double statusTimer = 0.0;

    while (!glfwWindowShouldClose(window)) {
        const auto now = std::chrono::steady_clock::now();
        const double dt = std::chrono::duration<double>(now - last).count();
        last = now;

        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, 1);

        const double speed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
                                  ? 60.0 : 12.0) * dt;
        const Vec3 fwd = view.camera.forward();
        const Vec3 right = view.camera.right();
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            view.camera.position += fwd * speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            view.camera.position += fwd * -speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            view.camera.position += right * speed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            view.camera.position += right * -speed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            view.camera.position += Vec3{0, speed, 0};
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            view.camera.position += Vec3{0, -speed, 0};

        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        if (width == 0 || height == 0) continue;   // minimised

        view.update();
        view.render(width, height, clear);
        glfwSwapBuffers(window);

        statusTimer += dt;
        if (statusTimer > 1.0) {
            statusTimer = 0.0;
            const WorldViewStats& s = view.stats();
            char title[256];
            std::snprintf(title, sizeof(title),
                          "Elysium  |  %.0f fps  |  %zu/%zu tiles  |  %llu tris"
                          "  |  %.0f MB  |  stream %.1f ms",
                          dt > 0 ? 1.0 / dt : 0.0, s.tilesVisible,
                          s.tilesResident,
                          (unsigned long long)s.trianglesDrawn,
                          s.bufferBytes / (1024.0 * 1024.0), s.updateMs);
            glfwSetWindowTitle(window, title);
        }
    }

    gView = nullptr;
    view.shutdown();
    renderer.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
