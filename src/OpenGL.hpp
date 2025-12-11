#ifndef OPENGL_HPP_
#define OPENGL_HPP_

#include "Common.hpp"

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 5

#if defined(_WIN32) && !defined(APIENTRY)
#   define APIENTRY __stdcall
#endif

#ifndef APIENTRY
#   define APIENTRY
#endif
#ifndef APIENTRYP
#   define APIENTRYP APIENTRY *
#endif

typedef float GLfloat;
typedef unsigned int GLbitfield;

#define GL_COLOR_BUFFER_BIT 0x00004000

typedef void (APIENTRYP PFN_glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (APIENTRYP PFN_glClear)(GLbitfield mask);

extern PFN_glClearColor glClearColor;
extern PFN_glClear glClear;

bool32_t OpenGL_LoadFunctions(void);

#endif