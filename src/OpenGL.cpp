#include "OpenGL.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

PFN_glGetString glGetString;
PFN_glGetStringi glGetStringi;
PFN_glGetIntegerv glGetIntegerv;
PFN_glEnable glEnable;

PFN_glDebugMessageCallback glDebugMessageCallback;
PFN_glDebugMessageControl glDebugMessageControl;

PFN_glClearColor glClearColor;
PFN_glClear glClear;

#define OPENGL_LOAD_FUNCTION(get_proc_address, name) \
{                                                    \
    name = (PFN_ ## name)get_proc_address(#name);    \
    ASSERT(name != NULL);                            \
}

bool32_t OpenGL_LoadFunctions(OpenGL_PFN_GetProcAddress get_opengl_proc_address)
{
    OPENGL_LOAD_FUNCTION(get_opengl_proc_address, glGetString);
    OPENGL_LOAD_FUNCTION(get_opengl_proc_address, glGetStringi);
    OPENGL_LOAD_FUNCTION(get_opengl_proc_address, glGetIntegerv);
    OPENGL_LOAD_FUNCTION(get_opengl_proc_address, glEnable);

    OPENGL_LOAD_FUNCTION(get_opengl_proc_address, glDebugMessageCallback);
    OPENGL_LOAD_FUNCTION(get_opengl_proc_address, glDebugMessageControl);

    OPENGL_LOAD_FUNCTION(get_opengl_proc_address, glClearColor);
    OPENGL_LOAD_FUNCTION(get_opengl_proc_address, glClear);

    return TRUE;
}

#define OPENGL_NUM_AVAILABLE_EXTENSIONS_NOT_SET ((uint32_t)-1)

static uint32_t OpenGL_NumAvailableExtensions = OPENGL_NUM_AVAILABLE_EXTENSIONS_NOT_SET;
static const char** OpenGL_AvailableExtensions;

static bool32_t OpenGL_PopulateAvailableExtensionsIfNeeded(void)
{
    if (OpenGL_NumAvailableExtensions != OPENGL_NUM_AVAILABLE_EXTENSIONS_NOT_SET)
        return TRUE;

    GLint num_extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);

    if (num_extensions > 0)
    {
        OpenGL_AvailableExtensions = (const char**)malloc(sizeof(const char*) * num_extensions);
        ASSERT(OpenGL_AvailableExtensions != NULL);

        OpenGL_NumAvailableExtensions = num_extensions;

        for (uint32_t i = 0; i < OpenGL_NumAvailableExtensions; ++i)
        {
            OpenGL_AvailableExtensions[i] = (const char*)glGetStringi(GL_EXTENSIONS, i);
        }
    }
    else
    {
        OpenGL_AvailableExtensions = NULL;
        OpenGL_NumAvailableExtensions = 0;
    }

    return TRUE;
}

bool32_t OpenGL_IsExtensionAvailable(const char* extension_name)
{
    ASSERT(extension_name != NULL);

    bool32_t populate_result = OpenGL_PopulateAvailableExtensionsIfNeeded();
    ASSERT(populate_result == TRUE);

    for (uint32_t i = 0; i < OpenGL_NumAvailableExtensions; ++i)
    {
        const char* current_extension_name = OpenGL_AvailableExtensions[i];

        if (strcmp(extension_name, current_extension_name) == 0)
            return TRUE;
    }

    return FALSE;
}

bool32_t OpenGL_GetAvailableExtensions(const char*** out_availabe_extensions, uint32_t* out_num_extensions)
{
    ASSERT(out_availabe_extensions != NULL);
    ASSERT(out_num_extensions != NULL);

    bool32_t populate_result = OpenGL_PopulateAvailableExtensionsIfNeeded();
    ASSERT(populate_result == TRUE);

    *out_availabe_extensions = OpenGL_AvailableExtensions;
    *out_num_extensions = OpenGL_NumAvailableExtensions;
    
    return TRUE;
}

const char* OpenGL_Debug_GetSourceString(GLenum source)
{
    static const char* unknown = "Unknown";

    static const char* api             = "API";
    static const char* window_system   = "Window System";
    static const char* shader_compiler = "Shader Compiler";
    static const char* third_party     = "Third Party";
    static const char* application     = "Application";
    static const char* other           = "Other";

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             return api;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return window_system;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return shader_compiler;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     return third_party;
        case GL_DEBUG_SOURCE_APPLICATION:     return application;
        case GL_DEBUG_SOURCE_OTHER:           return other;
    }

    UNREACHABLE;

    return unknown;
}

const char* OpenGL_Debug_GetTypeString(GLenum type)
{
    static const char* unknown = "Unknown";

    static const char* error               = "Error";
    static const char* deprecated_behavior = "Deprecated Behaviour";
    static const char* undefined_behavior  = "Undefined Behaviour";
    static const char* portability         = "Portability";
    static const char* performance         = "Performance";
    static const char* marker              = "Marker";
    static const char* push_group          = "Push Group";
    static const char* pop_group           = "Pop Group";
    static const char* other               = "Other";

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               return error;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return deprecated_behavior;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return undefined_behavior;
        case GL_DEBUG_TYPE_PORTABILITY:         return portability;
        case GL_DEBUG_TYPE_PERFORMANCE:         return performance;
        case GL_DEBUG_TYPE_MARKER:              return marker;
        case GL_DEBUG_TYPE_PUSH_GROUP:          return push_group;
        case GL_DEBUG_TYPE_POP_GROUP:           return pop_group;
        case GL_DEBUG_TYPE_OTHER:               return other;
    }

    UNREACHABLE;

    return unknown;
}

const char* OpenGL_Debug_GetSeverityString(GLenum severity)
{
    static const char* unknown = "Unknown";

    static const char* high         = "High";
    static const char* medium       = "Medium";
    static const char* low          = "Low";
    static const char* notification = "Notification";

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         return high;
        case GL_DEBUG_SEVERITY_MEDIUM:       return medium;
        case GL_DEBUG_SEVERITY_LOW:          return low;
        case GL_DEBUG_SEVERITY_NOTIFICATION: return notification;
    }

    UNREACHABLE;

    return unknown;
}

void APIENTRY OpenGL_Debug_WriteOutputToStderr(
    GLenum       source,
    GLenum       type,
    unsigned int id,
    GLenum       severity,
    GLsizei      length,
    const char*  message,
    const void*  user_param
)
{
    UNUSED(id);
    UNUSED(length);
    UNUSED(user_param);

    fprintf(
        stderr,
        "[OpenGL Debug | Source: %s | Type: %s | Severity: %s ]: %s\n",
        OpenGL_Debug_GetSourceString(source),
        OpenGL_Debug_GetTypeString(type),
        OpenGL_Debug_GetSeverityString(severity),
        message
    );
}
