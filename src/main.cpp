#include <stdio.h>

#include "Common.hpp"
#include "OpenGL.hpp"

#include <GLFW/glfw3.h>

int main(int, char**)
{
    if (!glfwInit())
    {
        fprintf(stderr, "GLFW init failed.\n");
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

    bool32_t load_result = OpenGL_LoadFunctions();
    ASSERT(load_result == TRUE);

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