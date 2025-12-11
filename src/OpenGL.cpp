#include "OpenGL.hpp"

#include <GLFW/glfw3.h>

PFN_glClearColor glClearColor;
PFN_glClear glClear;

#define OPENGL_LOAD_FUNCTION(name)                  \
{                                                   \
    name = (PFN_ ## name)glfwGetProcAddress(#name); \
    ASSERT(name != NULL);                           \
}

bool32_t OpenGL_LoadFunctions(void)
{
    OPENGL_LOAD_FUNCTION(glClearColor);
    OPENGL_LOAD_FUNCTION(glClear);

    return true;
}
