#include "glcore.h"

#if defined(_WIN32)
// The pointer definitions. One line each, from the same list the header
// declares them from, so the two can never drift apart.
#define ELY_GL_DEFINE(ret, name, args) ret(ELY_GLAPI* name) args = nullptr;
ELY_GL_FUNCTIONS(ELY_GL_DEFINE)
#undef ELY_GL_DEFINE
#endif

namespace ely {

bool loadGlFunctions(void* (*getProc)(const char*), std::string* error) {
#if defined(_WIN32)
    if (!getProc) {
        if (error) *error = "no GL function getter supplied";
        return false;
    }
    const char* missing = nullptr;

    // Assigned through the pointer's own type rather than a cast to a generic
    // function pointer: if the signature in the header ever disagrees with the
    // one the driver has, this is a compile error rather than a stack smash.
#define ELY_GL_LOAD(ret, name, args)                                          \
    name = reinterpret_cast<ret(ELY_GLAPI*) args>(getProc(#name));            \
    if (!name && !missing) missing = #name;
    ELY_GL_FUNCTIONS(ELY_GL_LOAD)
#undef ELY_GL_LOAD

    if (missing) {
        if (error) {
            *error = std::string("this OpenGL context does not provide ")
                   + missing
                   + ". The renderer needs a 3.3 core context; check that one "
                     "was requested and made current before loading.";
        }
        return false;
    }
    return true;
#else
    // Nothing to do: libGL exports these and the linker has already bound them.
    (void)getProc;
    (void)error;
    return true;
#endif
}

}  // namespace ely
