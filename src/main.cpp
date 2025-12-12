#include <stdio.h>

#include "Common.hpp"
#include "OpenGL.hpp"
#include "OpenGL_Shader.hpp"
#include "Geometry.hpp"

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

typedef struct DrawElementsIndirectCommand
{
    GLuint count;
    GLuint instance_count;
    GLuint first_index;
    GLint  base_vertex;
    GLuint base_instance;
};

int main(int, char**)
{
    const int window_width = 1280;
    const int window_height = 720;

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

    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "window", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "glfwCreateWindow failed.\n");

        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    bool32_t load_result = OpenGL_LoadFunctions(glfwGetProcAddress);
    ASSERT(load_result == TRUE);

    if (!OpenGL_IsExtensionAvailable("GL_ARB_shader_draw_parameters"))
    {
        fprintf(stderr, "Extension \"GL_ARB_shader_draw_parameters\" is not available.");

        glfwTerminate();
        return 1;
    }

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

        bool32_t get_extensions_result = OpenGL_GetAvailableExtensions(opengl_extensions, opengl_num_extensions);
        ASSERT(get_extensions_result == TRUE);

        fprintf(stderr, "OpenGL extensions:\n");
        for (uint32_t i = 0; i < opengl_num_extensions; ++i)
        {
            fprintf(stderr, " - %s\n", opengl_extensions[i]);
        }
    }

    GLuint default_program;
    {
        GLuint shaders[2];

        shaders[0] = OpenGL_Shader_CreateShader(GL_VERTEX_SHADER, OpenGL_Shader_DefaultVertexSource);
        ASSERT(shaders[0] != 0);

        shaders[1] = OpenGL_Shader_CreateShader(GL_FRAGMENT_SHADER, OpenGL_Shader_DefaultFragmentSource);
        ASSERT(shaders[1] != 0);

        default_program = OpenGL_Shader_CreateProgram(shaders, 2);
        ASSERT(default_program != 0);

        glDeleteShader(shaders[0]);
        glDeleteShader(shaders[1]);
    }

    glUseProgram(default_program);

    uint32_t ball_resolution = 16;

    uint32_t ball_num_vertices = Geometry_Ball_GetNumRequiredVertices(ball_resolution);
    uint32_t ball_num_indices = Geometry_Ball_GetNumRequiredIndices(ball_resolution);

    uint32_t max_num_vertices = 1024 * 1024;
    uint32_t max_num_indices = 1024 * 1024;

    GLuint vbo, ebo;
    glCreateBuffers(1, &vbo);
    glCreateBuffers(1, &ebo);

    glNamedBufferStorage(vbo, sizeof(CVertex) * max_num_vertices, NULL, GL_MAP_WRITE_BIT);
    glNamedBufferStorage(ebo, sizeof(uint32_t) * max_num_indices, NULL, GL_MAP_WRITE_BIT);

    CVertex* vertices = (CVertex*)glMapNamedBuffer(vbo, GL_WRITE_ONLY);
    uint32_t* indices = (uint32_t*)glMapNamedBuffer(ebo, GL_WRITE_ONLY);

    Geometry_Ball_GenerateColoredTriangularGeometry(vertices, max_num_vertices, indices, max_num_indices, ball_resolution, { 1.0f, 0.0f, 0.0f, 1.0f });
    Geometry_Ball_GenerateColoredTriangularGeometry(vertices + ball_num_vertices, max_num_vertices - ball_num_vertices, indices + ball_num_indices, max_num_indices - ball_num_indices, ball_resolution, { 0.0f, 0.5f, 0.5f, 1.0f });

    GLboolean vbo_unmap_result = glUnmapNamedBuffer(vbo);
    ASSERT(vbo_unmap_result == GL_TRUE);

    GLboolean ebo_unmap_result = glUnmapNamedBuffer(ebo);
    ASSERT(ebo_unmap_result == GL_TRUE);

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(CVertex));
    glVertexArrayElementBuffer(vao, ebo);

    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(CVertex, position));
    glVertexArrayAttribFormat(vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(CVertex, color));
    glVertexArrayAttribFormat(vao, 2, 3, GL_FLOAT, GL_FALSE, offsetof(CVertex, normal));

    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);
    glEnableVertexArrayAttrib(vao, 2);

    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 0);
    glVertexArrayAttribBinding(vao, 2, 0);
    
    uint32_t max_num_draw_commands = 1024;

    GLuint draw_indirect_buffer_object;
    glCreateBuffers(1, &draw_indirect_buffer_object);
    glNamedBufferStorage(draw_indirect_buffer_object, sizeof(DrawElementsIndirectCommand) * max_num_draw_commands, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

    DrawElementsIndirectCommand* draw_commands = (DrawElementsIndirectCommand*)glMapNamedBufferRange(
        draw_indirect_buffer_object,
        0,
        sizeof(DrawElementsIndirectCommand) * max_num_draw_commands,
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT
    );

    draw_commands[0].count = ball_num_indices;
    draw_commands[0].instance_count = 1;
    draw_commands[0].first_index = 0;
    draw_commands[0].base_vertex = 0;
    draw_commands[0].base_instance = 0;

    draw_commands[1].count = ball_num_indices;
    draw_commands[1].instance_count = 1;
    draw_commands[1].first_index = ball_num_indices;
    draw_commands[1].base_vertex = ball_num_vertices;
    draw_commands[1].base_instance = 0;

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_indirect_buffer_object);

    glm::mat4 identity(1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)window_width / window_height, 0.1f, 100.0f);
    
    glUniformMatrix4fv(0, 1, GL_FALSE, (float*)&projection);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        float time = (float)glfwGetTime();

        glm::mat4 model = glm::translate(identity, { 0.0f, 0.0f, -5.0f });
        model = glm::rotate(model, time, { 0.0f, 1.0f, 0.0f });

        glUniformMatrix4fv(1, 1, GL_FALSE, (float*)&model);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        glBindVertexArray(vao);
        glMultiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            (const void*)0,
            2,
            sizeof(DrawElementsIndirectCommand)
        );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}