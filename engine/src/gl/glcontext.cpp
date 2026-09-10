#include "glcontext.h"
#include "glcore.h"

#include <cstdint>

// EGL, declared here for the same reason glcore.h declares GL: the runtime
// library is present, the headers are not. EGL's surface is small enough that
// eight functions and a dozen constants cover the whole of what a headless
// context needs.
extern "C" {
typedef void* EGLDisplay;
typedef void* EGLConfig;
typedef void* EGLContext;
typedef void* EGLSurface;
typedef unsigned int EGLenum;
typedef int EGLint;
typedef unsigned int EGLBoolean;

EGLDisplay eglGetPlatformDisplay(EGLenum platform, void* native,
                                 const intptr_t* attribList);
EGLBoolean eglInitialize(EGLDisplay dpy, EGLint* major, EGLint* minor);
EGLBoolean eglTerminate(EGLDisplay dpy);
EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint* attribList,
                           EGLConfig* configs, EGLint configSize, EGLint* numConfig);
EGLBoolean eglBindAPI(EGLenum api);
EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config,
                            EGLContext share, const EGLint* attribList);
EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read,
                          EGLContext ctx);
EGLint eglGetError(void);
void (*eglGetProcAddress(const char* name))(void);
}

namespace {

constexpr EGLenum kPlatformSurfaceless = 0x31DD;   // EGL_PLATFORM_SURFACELESS_MESA
constexpr EGLint kSurfaceType          = 0x3033;
constexpr EGLint kPbufferBit           = 0x0001;
constexpr EGLint kRenderableType       = 0x3040;
constexpr EGLint kOpenGlBit            = 0x0008;
constexpr EGLint kAlphaSize            = 0x3021;
constexpr EGLint kBlueSize             = 0x3022;
constexpr EGLint kGreenSize            = 0x3023;
constexpr EGLint kRedSize              = 0x3024;
constexpr EGLint kDepthSize            = 0x3025;
constexpr EGLint kNone                 = 0x3038;
constexpr EGLenum kOpenGlApi           = 0x30A2;
constexpr EGLint kContextMajor         = 0x3098;
constexpr EGLint kContextMinor         = 0x30FB;
constexpr EGLint kProfileMask          = 0x30FD;
constexpr EGLint kCoreProfileBit       = 0x00000001;

std::string glString(GLenum name) {
    const GLubyte* s = glGetString(name);
    return s ? std::string(reinterpret_cast<const char*>(s)) : std::string("(none)");
}

}  // namespace

namespace ely {

HeadlessGl::~HeadlessGl() { destroy(); }

bool HeadlessGl::create(int major, int minor, std::string* error) {
    auto fail = [&](const char* what) {
        if (error) {
            char buf[192];
            std::snprintf(buf, sizeof(buf), "%s (EGL error 0x%04x)", what,
                          unsigned(eglGetError()));
            *error = buf;
        }
        destroy();
        return false;
    };

    EGLDisplay dpy = eglGetPlatformDisplay(kPlatformSurfaceless, nullptr, nullptr);
    if (!dpy) return fail("no surfaceless EGL display");
    display_ = dpy;

    EGLint eglMajor = 0, eglMinor = 0;
    if (!eglInitialize(dpy, &eglMajor, &eglMinor))
        return fail("eglInitialize failed");

    // A config is still required even with no surface: it fixes the formats
    // the context is compatible with. The depth size asked for here is what
    // any framebuffer this context renders to will be able to use.
    const EGLint configAttribs[] = {
        kSurfaceType, kPbufferBit,
        kRenderableType, kOpenGlBit,
        kRedSize, 8, kGreenSize, 8, kBlueSize, 8, kAlphaSize, 8,
        kDepthSize, 24,
        kNone
    };
    EGLConfig config = nullptr;
    EGLint numConfigs = 0;
    if (!eglChooseConfig(dpy, configAttribs, &config, 1, &numConfigs) ||
        numConfigs == 0)
        return fail("no EGL config with 8-bit colour and 24-bit depth");

    // Desktop GL, not GLES. Without this the default is GLES and the core
    // profile request below is silently meaningless.
    if (!eglBindAPI(kOpenGlApi)) return fail("eglBindAPI(OpenGL) failed");

    const EGLint contextAttribs[] = {
        kContextMajor, major,
        kContextMinor, minor,
        kProfileMask, kCoreProfileBit,
        kNone
    };
    EGLContext ctx = eglCreateContext(dpy, config, nullptr, contextAttribs);
    if (!ctx) return fail("could not create a core-profile context");
    context_ = ctx;

    // Surfaceless: both draw and read are EGL_NO_SURFACE. Everything this
    // renderer draws goes to a framebuffer object, so there is nothing a
    // window surface would add.
    if (!eglMakeCurrent(dpy, nullptr, nullptr, ctx))
        return fail("eglMakeCurrent failed (no EGL_KHR_surfaceless_context?)");

    // A no-op where libGL exports the entry points directly, which is every
    // platform this path runs on. Called anyway so the two ways of getting a
    // context behave identically.
    if (!loadGlFunctions(
            reinterpret_cast<void* (*)(const char*)>(eglGetProcAddress), error))
        return false;

    return true;
}

void HeadlessGl::destroy() {
    if (display_) {
        eglMakeCurrent(display_, nullptr, nullptr, nullptr);
        if (context_) eglDestroyContext(display_, context_);
        eglTerminate(display_);
    }
    context_ = nullptr;
    display_ = nullptr;
}

std::string HeadlessGl::version() const { return glString(GL_VERSION); }
std::string HeadlessGl::renderer() const { return glString(GL_RENDERER); }
std::string HeadlessGl::glslVersion() const {
    return glString(GL_SHADING_LANGUAGE_VERSION);
}

}  // namespace ely
