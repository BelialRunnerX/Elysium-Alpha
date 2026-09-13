// A headless OpenGL context, via EGL's surfaceless platform.
//
// This is what makes the renderer testable. There is no window, no display
// server and no GPU in the environment this was built in — Mesa's llvmpipe
// rasterises on the CPU — and yet every draw goes through the real driver, the
// real GLSL compiler and the real depth test. The pictures it produces are
// produced the same way the game produces them.
//
// That is worth more than it sounds. The first edition's lesson was that the
// one system genuinely verified was the one with a headless renderer; this is
// the same principle (S6) applied to the renderer itself. A shader that does
// not compile, a framebuffer that is incomplete, an attribute format the driver
// rejects, a winding order that culls the world — all of it fails here, in a
// test, in under a second, instead of on someone's desk.
//
// On a machine with a window, the same GlRenderer runs against a context from
// GLFW or SDL instead; this class is only the headless one.
#pragma once
#include <string>

namespace ely {

class HeadlessGl {
public:
    HeadlessGl() = default;
    ~HeadlessGl();

    HeadlessGl(const HeadlessGl&) = delete;
    HeadlessGl& operator=(const HeadlessGl&) = delete;

    // Create and make current an OpenGL core-profile context.
    //
    // Requests `major.minor` core. Returns false with a reason in `error` if
    // EGL is missing, no config matches, or the driver refuses the version —
    // all of which are things a caller should report rather than crash on,
    // because a machine without a GPU is a normal machine to run tests on.
    bool create(int major = 3, int minor = 3, std::string* error = nullptr);
    void destroy();

    bool valid() const { return context_ != nullptr; }

    // Driver identification, for the record. Worth printing in any tool that
    // renders: "llvmpipe" and "NVIDIA RTX" failing differently is common, and
    // knowing which one produced an image saves an hour.
    std::string version() const;
    std::string renderer() const;
    std::string glslVersion() const;

private:
    void* display_ = nullptr;
    void* context_ = nullptr;
};

}  // namespace ely
