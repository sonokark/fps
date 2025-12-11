#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Common.hpp"
#include "OpenGL.hpp"
#include "OpenGL_Shader.hpp"

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

struct CVertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec3 normal;
};

constexpr inline uint32_t GetBallNumVertices(uint32_t resolution)
{
    uint32_t num_planes = resolution - 1;

    return num_planes * resolution + 2;
}

constexpr inline uint32_t GetBallNumIndices(uint32_t resolution)
{
    uint32_t num_middle_layers = resolution - 2;
    uint32_t num_triangles = resolution * 2 + num_middle_layers * resolution * 2;

    return num_triangles * 3;
}

static void GenerateColoredUnitBallTriangluarGeometry(
    CVertex*         vertices,
    uint32_t         max_num_vertices,
    uint32_t*        indices,
    uint32_t         max_num_indices,
    uint32_t         resolution,
    const glm::vec4& color
)
{
    ASSERT(max_num_vertices >= GetBallNumVertices(resolution));
    ASSERT(max_num_indices >= GetBallNumIndices(resolution));

    // TODO: Use rotation matrices for updating the position vector

    uint32_t vertex_index = 0;

    const uint32_t top_vertex_index = vertex_index;
    vertices[vertex_index++] = { { 0.0f,  1.0f, 0.0f }, color, { 0.0f,  1.0f, 0.0f } };
    
    const uint32_t bottom_vertex_index = vertex_index;
    vertices[vertex_index++] = { { 0.0f, -1.0f, 0.0f }, color, { 0.0f, -1.0f, 0.0f } };

    float delta_yaw = 2.0f * (float)M_PI / resolution;
    float delta_pitch = -(float)M_PI / resolution;
    
    float current_pitch = (float)M_PI * 0.5f + delta_pitch;

    glm::vec3 current_position;

    for (uint32_t i = 0; i < resolution - 1; ++i)
    {
        current_position.y = sinf(current_pitch);
        float current_pitch_cos = cosf(current_pitch);

        float current_yaw = 0.0f;

        for (uint32_t j = 0; j < resolution; ++j)
        {
            current_position.x = current_pitch_cos * cosf(current_yaw);
            current_position.z = current_pitch_cos * sinf(current_yaw);

            vertices[vertex_index++] = { current_position, color, current_position };

            current_yaw += delta_yaw;
        }

        current_pitch += delta_pitch;
    }

    uint32_t index_index = 0;
    
    const uint32_t num_planes = resolution - 1;

    for (uint32_t i = 0; i < resolution; ++i)
    {
        uint32_t current_vertex_index = 2 + i;
        uint32_t next_vertex_index = 2 + (i + 1) % resolution;

        indices[index_index++] = next_vertex_index;
        indices[index_index++] = current_vertex_index;
        indices[index_index++] = top_vertex_index;

        current_vertex_index = 2 + (num_planes - 1) * resolution + i;
        next_vertex_index = 2 + (num_planes - 1) * resolution + (i + 1) % resolution;

        indices[index_index++] = current_vertex_index;
        indices[index_index++] = next_vertex_index;
        indices[index_index++] = bottom_vertex_index;
    }

    uint32_t upper_vertex_index_offset = 2;
    uint32_t lower_vertex_index_offset = upper_vertex_index_offset + resolution;

    for (uint32_t layer_index = 0; layer_index < num_planes; ++layer_index)
    {
        for (uint32_t i = 0; i < resolution; ++i)
        {
            uint32_t ur_vertex_index = upper_vertex_index_offset + i;
            uint32_t ul_vertex_index = upper_vertex_index_offset + (i + 1) % resolution;
            uint32_t lr_vertex_index = lower_vertex_index_offset + i;
            uint32_t ll_vertex_index = lower_vertex_index_offset + (i + 1) % resolution;

            indices[index_index++] = ul_vertex_index;
            indices[index_index++] = ll_vertex_index;
            indices[index_index++] = lr_vertex_index;
            
            indices[index_index++] = ul_vertex_index;
            indices[index_index++] = lr_vertex_index;
            indices[index_index++] = ur_vertex_index;
        }

        upper_vertex_index_offset = lower_vertex_index_offset;
        lower_vertex_index_offset += resolution;
    }
}

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

    uint32_t resolution = 16;
    uint32_t num_vertices = GetBallNumVertices(resolution);
    uint32_t num_indices = GetBallNumIndices(resolution);
    
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CVertex) * num_vertices, NULL, GL_STATIC_DRAW);

    CVertex* vertex_buffer_data = (CVertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * num_indices, NULL, GL_STATIC_DRAW);

    uint32_t* element_buffer_data = (uint32_t*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

    GenerateColoredUnitBallTriangluarGeometry(
        vertex_buffer_data,
        num_vertices,
        element_buffer_data,
        num_indices,
        resolution,
        { 0.0f, 0.5f, 0.5f, 1.0f }
    );

    GLboolean unmap_array_buffer_result = glUnmapBuffer(GL_ARRAY_BUFFER);
    ASSERT(unmap_array_buffer_result == GL_TRUE);

    GLboolean unmap_element_buffer_result = glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    ASSERT(unmap_element_buffer_result == GL_TRUE);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CVertex), (const void*)offsetof(CVertex, position));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(CVertex), (const void*)offsetof(CVertex, color));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CVertex), (const void*)offsetof(CVertex, normal));

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
        
        glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, (const void*)0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}