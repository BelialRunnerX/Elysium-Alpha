// A minimal OpenGL 3.3 core declaration set, and a loader for platforms that
// need one.
//
// Why this file exists rather than #include <GL/gl.h> plus glad: the
// environment this was developed in has Mesa's runtime libraries but not its
// headers, and no package registry to fetch them from. Declaring the ~60 entry
// points the renderer actually uses turned out to be the difference between a
// renderer that has been run and one that has only been written, and that is
// not a close call.
//
// This is a legitimate technique, not a hack — it is what glad and gl3w
// generate, only by hand and only for what is used. Two things make it safe:
//
//   * The GL types are fixed by the ABI (GLint is a 32-bit int on every
//     platform GL runs on, GLsizeiptr is ptrdiff_t), and the enum values are
//     fixed by the registry and have never changed.
//   * Everything here is exercised. If a constant were wrong the smoke test
//     would fail, the shader would not link, or the framebuffer would report
//     incomplete — and the renderer checks all three.
//
// TWO PLATFORMS, TWO MECHANISMS.
//
//   Linux/macOS: libGL exports the modern entry points directly, so these are
//   plain extern "C" declarations and the linker resolves them. Nothing to
//   load, and each call is a direct call.
//
//   Windows: opengl32.dll exports OpenGL 1.1 and nothing later. glCreateShader,
//   glGenVertexArrays and everything else this renderer depends on must be
//   fetched at run time, and only once a context is current. wglGetProcAddress
//   returns null for the 1.1 functions, and GetProcAddress on the DLL returns
//   null for everything newer, so a correct loader has to try both. GLFW's
//   glfwGetProcAddress already does exactly that, which is why loadGlFunctions
//   takes the getter as an argument rather than implementing it.
//
// On Windows the names below are function pointers rather than functions. Call
// syntax is identical, so nothing else in the engine has to know.
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

// --- Types (fixed by the OpenGL ABI) ---------------------------------------
typedef unsigned int   GLenum;
typedef unsigned char  GLboolean;
typedef unsigned int   GLbitfield;
typedef signed char    GLbyte;
typedef unsigned char  GLubyte;
typedef short          GLshort;
typedef unsigned short GLushort;
typedef int            GLint;
typedef unsigned int   GLuint;
typedef int            GLsizei;
typedef float          GLfloat;
typedef double         GLdouble;
typedef char           GLchar;
typedef void           GLvoid;
typedef ptrdiff_t      GLintptr;
typedef ptrdiff_t      GLsizeiptr;

#if defined(_WIN32)
  #define ELY_GLAPI __stdcall
#else
  #define ELY_GLAPI
#endif

// --- Constants --------------------------------------------------------------
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_NO_ERROR                       0
#define GL_TRIANGLES                      0x0004
#define GL_LESS                           0x0201
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_CW                             0x0900
#define GL_CCW                            0x0901
#define GL_CULL_FACE                      0x0B44
#define GL_DEPTH_TEST                     0x0B71
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_TEXTURE_2D                     0x0DE1
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_DEPTH_COMPONENT                0x1902
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_NEAREST                        0x2600
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_RGBA8                          0x8058
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5

// --- The entry points the renderer uses -------------------------------------
//
// One list, two expansions. Adding a GL call means adding one line here and
// nothing else — which matters, because the failure mode of forgetting the
// Windows half is a link error at best and a null-pointer call at worst.
#define ELY_GL_FUNCTIONS(X)                                                    \
  X(const GLubyte*, glGetString, (GLenum name))                                \
  X(GLenum, glGetError, (void))                                                \
  X(void, glEnable, (GLenum cap))                                              \
  X(void, glDisable, (GLenum cap))                                             \
  X(void, glDepthFunc, (GLenum func))                                          \
  X(void, glCullFace, (GLenum mode))                                           \
  X(void, glFrontFace, (GLenum mode))                                          \
  X(void, glClearColor, (GLfloat r, GLfloat g, GLfloat b, GLfloat a))          \
  X(void, glClear, (GLbitfield mask))                                          \
  X(void, glViewport, (GLint x, GLint y, GLsizei w, GLsizei h))                \
  X(void, glFinish, (void))                                                    \
  X(void, glPixelStorei, (GLenum pname, GLint param))                          \
  X(void, glReadPixels, (GLint x, GLint y, GLsizei w, GLsizei h, GLenum fmt,   \
                         GLenum type, void* pixels))                           \
  X(void, glDrawArrays, (GLenum mode, GLint first, GLsizei count))             \
  X(void, glDrawElements, (GLenum mode, GLsizei count, GLenum type,            \
                           const void* indices))                               \
  X(void, glGenBuffers, (GLsizei n, GLuint* buffers))                          \
  X(void, glBindBuffer, (GLenum target, GLuint buffer))                        \
  X(void, glBufferData, (GLenum target, GLsizeiptr size, const void* data,     \
                         GLenum usage))                                        \
  X(void, glBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size,   \
                            const void* data))                                 \
  X(void, glDeleteBuffers, (GLsizei n, const GLuint* buffers))                 \
  X(void, glBindBufferBase, (GLenum target, GLuint index, GLuint buffer))      \
  X(void, glGenVertexArrays, (GLsizei n, GLuint* arrays))                      \
  X(void, glBindVertexArray, (GLuint array))                                   \
  X(void, glDeleteVertexArrays, (GLsizei n, const GLuint* arrays))             \
  X(void, glEnableVertexAttribArray, (GLuint index))                           \
  X(void, glVertexAttribIPointer, (GLuint index, GLint size, GLenum type,      \
                                   GLsizei stride, const void* pointer))       \
  X(void, glVertexAttribPointer, (GLuint index, GLint size, GLenum type,       \
                                  GLboolean normalized, GLsizei stride,        \
                                  const void* pointer))                        \
  X(GLuint, glCreateShader, (GLenum type))                                     \
  X(void, glShaderSource, (GLuint shader, GLsizei count,                       \
                           const GLchar* const* string, const GLint* length))  \
  X(void, glCompileShader, (GLuint shader))                                    \
  X(void, glGetShaderiv, (GLuint shader, GLenum pname, GLint* params))         \
  X(void, glGetShaderInfoLog, (GLuint shader, GLsizei bufSize,                 \
                               GLsizei* length, GLchar* infoLog))              \
  X(void, glDeleteShader, (GLuint shader))                                     \
  X(GLuint, glCreateProgram, (void))                                           \
  X(void, glAttachShader, (GLuint program, GLuint shader))                     \
  X(void, glLinkProgram, (GLuint program))                                     \
  X(void, glGetProgramiv, (GLuint program, GLenum pname, GLint* params))       \
  X(void, glGetProgramInfoLog, (GLuint program, GLsizei bufSize,               \
                                GLsizei* length, GLchar* infoLog))             \
  X(void, glUseProgram, (GLuint program))                                      \
  X(void, glDeleteProgram, (GLuint program))                                   \
  X(GLint, glGetUniformLocation, (GLuint program, const GLchar* name))         \
  X(void, glUniformMatrix4fv, (GLint location, GLsizei count,                  \
                               GLboolean transpose, const GLfloat* value))     \
  X(void, glUniform4fv, (GLint location, GLsizei count, const GLfloat* value)) \
  X(void, glUniform1i, (GLint location, GLint v0))                             \
  X(GLuint, glGetUniformBlockIndex, (GLuint program, const GLchar* blockName)) \
  X(void, glUniformBlockBinding, (GLuint program, GLuint blockIndex,           \
                                  GLuint blockBinding))                        \
  X(void, glGenFramebuffers, (GLsizei n, GLuint* framebuffers))                \
  X(void, glBindFramebuffer, (GLenum target, GLuint framebuffer))              \
  X(void, glFramebufferTexture2D, (GLenum target, GLenum attachment,           \
                                   GLenum textarget, GLuint texture,           \
                                   GLint level))                               \
  X(void, glFramebufferRenderbuffer, (GLenum target, GLenum attachment,        \
                                      GLenum rbtarget, GLuint renderbuffer))   \
  X(GLenum, glCheckFramebufferStatus, (GLenum target))                         \
  X(void, glDeleteFramebuffers, (GLsizei n, const GLuint* framebuffers))       \
  X(void, glGenRenderbuffers, (GLsizei n, GLuint* renderbuffers))              \
  X(void, glBindRenderbuffer, (GLenum target, GLuint renderbuffer))            \
  X(void, glRenderbufferStorage, (GLenum target, GLenum internalformat,        \
                                  GLsizei width, GLsizei height))              \
  X(void, glDeleteRenderbuffers, (GLsizei n, const GLuint* renderbuffers))     \
  X(void, glGenTextures, (GLsizei n, GLuint* textures))                        \
  X(void, glBindTexture, (GLenum target, GLuint texture))                      \
  X(void, glTexImage2D, (GLenum target, GLint level, GLint internalformat,     \
                         GLsizei width, GLsizei height, GLint border,          \
                         GLenum format, GLenum type, const void* pixels))      \
  X(void, glTexParameteri, (GLenum target, GLenum pname, GLint param))         \
  X(void, glDeleteTextures, (GLsizei n, const GLuint* textures))

#if defined(_WIN32)
// Function pointers named exactly as the functions. Legal because no system GL
// header is included here, and the call syntax is unchanged.
#define ELY_GL_DECLARE(ret, name, args) extern ret(ELY_GLAPI* name) args;
ELY_GL_FUNCTIONS(ELY_GL_DECLARE)
#undef ELY_GL_DECLARE
#else
extern "C" {
#define ELY_GL_DECLARE(ret, name, args) ret name args;
ELY_GL_FUNCTIONS(ELY_GL_DECLARE)
#undef ELY_GL_DECLARE
}
#endif

namespace ely {

// Resolve the entry points above.
//
// `getProc` must be a getter valid for the *current* context; glfwGetProcAddress
// is the intended one. Where the functions link directly this returns true
// without doing anything, so callers can always call it and never need a
// platform test of their own.
//
// Returns false and names the first missing function on failure. A missing
// entry point means the context is older than 3.3 or was never made current,
// and both of those produce a crash rather than a message if the loader shrugs.
bool loadGlFunctions(void* (*getProc)(const char*), std::string* error);

// Turn a GL error code into something a human can act on. Called after every
// stage that can fail rather than once at the end: "GL_INVALID_OPERATION
// somewhere in the last two hundred calls" is not a bug report.
inline const char* glErrorName(GLenum e) {
    switch (e) {
        case GL_NO_ERROR: return "none";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        default: return "unknown";
    }
}

}  // namespace ely
