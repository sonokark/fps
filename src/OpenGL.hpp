#ifndef OPENGL_HPP_
#define OPENGL_HPP_

#include "Common.hpp"

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 5

typedef void(*OpenGL_PFN_Proc)(void);
typedef OpenGL_PFN_Proc(*OpenGL_PFN_GetProcAddress)(const char* name);

#if defined(_WIN32) && !defined(APIENTRY)
#   define APIENTRY __stdcall
#endif

#ifndef APIENTRY
#   define APIENTRY
#endif
#ifndef APIENTRYP
#   define APIENTRYP APIENTRY *
#endif

typedef char GLchar;
typedef unsigned char GLboolean;
typedef uint8_t GLubyte;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef unsigned int GLbitfield;

typedef void (APIENTRYP GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_DONT_CARE 0x1100

#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_NUM_EXTENSIONS 0x821D

#define GL_DEBUG_SOURCE_API 0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY 0x8249
#define GL_DEBUG_SOURCE_APPLICATION 0x824A
#define GL_DEBUG_SOURCE_OTHER 0x824B

#define GL_DEBUG_TYPE_ERROR 0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#define GL_DEBUG_TYPE_PORTABILITY 0x824F
#define GL_DEBUG_TYPE_PERFORMANCE 0x8250
#define GL_DEBUG_TYPE_MARKER 0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP 0x8269
#define GL_DEBUG_TYPE_POP_GROUP 0x826A
#define GL_DEBUG_TYPE_OTHER 0x8251

#define GL_DEBUG_SEVERITY_HIGH 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#define GL_DEBUG_SEVERITY_LOW 0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B

#define GL_CONTEXT_FLAGS 0x821E
#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002

#define GL_DEBUG_OUTPUT 0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242

#define GL_COLOR_BUFFER_BIT 0x00004000

typedef const GLubyte* (APIENTRYP PFN_glGetString)(GLenum name);
typedef const GLubyte* (APIENTRYP PFN_glGetStringi)(GLenum name, GLuint index);
typedef void (APIENTRYP PFN_glGetIntegerv)(GLenum pname, GLint* data);
typedef void (APIENTRYP PFN_glEnable)(GLenum cap);

typedef void (APIENTRYP PFN_glDebugMessageCallback)(GLDEBUGPROC callback, const void* userParam);
typedef void (APIENTRYP PFN_glDebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);

typedef void (APIENTRYP PFN_glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (APIENTRYP PFN_glClear)(GLbitfield mask);

extern PFN_glGetString glGetString;
extern PFN_glGetStringi glGetStringi;
extern PFN_glGetIntegerv glGetIntegerv;
extern PFN_glEnable glEnable;

extern PFN_glDebugMessageCallback glDebugMessageCallback;
extern PFN_glDebugMessageControl glDebugMessageControl;

extern PFN_glClearColor glClearColor;
extern PFN_glClear glClear;

bool32_t OpenGL_LoadFunctions(OpenGL_PFN_GetProcAddress get_opengl_proc_address);

bool32_t OpenGL_IsExtensionAvailable(const char* extension_name);

bool32_t OpenGL_GetAvailableExtensions(const char*** out_availabe_extensions, uint32_t* out_num_extensions);

const char* OpenGL_Debug_GetSourceString(GLenum source);

const char* OpenGL_Debug_GetTypeString(GLenum type);

const char* OpenGL_Debug_GetSeverityString(GLenum severity);

void APIENTRY OpenGL_Debug_WriteOutputToStderr(
    GLenum       source,
    GLenum       type,
    unsigned int id,
    GLenum       severity,
    GLsizei      length,
    const char*  message,
    const void*  user_param
);

#endif