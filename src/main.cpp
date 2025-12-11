#include <stdio.h>

#include "Common.hpp"
#include "OpenGL.hpp"

#include <GLFW/glfw3.h>

int main(int, char**)
{
    if (!glfwInit())
    {
        fprintf(stderr, "glfwInit failed.\n");
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // TODO: Enable only on debug builds

    // glfwWindowHint(GLFW_SAMPLES, ...);
    // glfwWindowHint(GLFW_SRGB_CAPABLE, ...);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "window", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "glfwCreateWindow failed.\n");

        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    bool32_t load_result = OpenGL_LoadFunctions(glfwGetProcAddress);
    ASSERT(load_result == TRUE);

    GLint context_flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
    if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        fprintf(stderr, "OpenGL debugging enabled.\n");

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        glDebugMessageCallback(OpenGL_Debug_WriteOutputToStderr, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }

    // Query OpenGL version and extensions
    {
        const char* opengl_version = (const char*)glGetString(GL_VERSION);
        fprintf(stderr, "OpenGL version: %s\n", opengl_version);

        uint32_t opengl_num_extensions;
        const char** opengl_extensions;

        bool32_t get_extensions_result = OpenGL_GetAvailableExtensions(&opengl_extensions, &opengl_num_extensions);
        ASSERT(get_extensions_result == TRUE);

        fprintf(stderr, "OpenGL extensions:\n");
        for (uint32_t i = 0; i < opengl_num_extensions; ++i)
        {
            fprintf(stderr, " - %s\n", opengl_extensions[i]);
        }
    }

    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}