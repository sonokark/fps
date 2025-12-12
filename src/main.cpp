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

#define SCENE_MAX_NUM_VERTICES 512
#define SCENE_MAX_NUM_HALF_EDGES 512
#define SCENE_MAX_NUM_FACES 512

#define SCENE_VERTEX_MAX_NUM_OUTGOING_HALF_EDGES 16
#define SCENE_VERTEX_MAX_NUM_INGOING_HALF_EDGES 16

struct Scene_Vertex;
struct Scene_HalfEdge;
struct Scene_Face;

struct Scene_Vertex
{
    glm::vec3 position;

    Scene_HalfEdge* outgoing_half_edges[SCENE_VERTEX_MAX_NUM_OUTGOING_HALF_EDGES];
    uint32_t num_outgoing_half_edges;
    
    Scene_HalfEdge* ingoing_half_edges[SCENE_VERTEX_MAX_NUM_INGOING_HALF_EDGES];
    uint32_t num_ingoing_half_edges;
};

struct Scene_HalfEdge
{
    Scene_Vertex* origin_vertex;
    Scene_Vertex* end_vertex;

    Scene_HalfEdge* opposite_half_edge;
    Scene_HalfEdge* next_half_edge;
    Scene_HalfEdge* prev_half_edge;

    Scene_Face* face;
};

struct Scene_Face
{
    glm::vec4 color;

    Scene_HalfEdge* half_edge;
};

struct Scene
{
    Scene_Vertex vertices[SCENE_MAX_NUM_VERTICES];
    uint32_t num_vertices;

    Scene_HalfEdge half_edges[SCENE_MAX_NUM_HALF_EDGES];
    uint32_t num_half_edges;

    Scene_Face faces[SCENE_MAX_NUM_FACES];
    uint32_t num_faces;
};

Scene_Vertex* Scene_AddVertex(Scene* scene, glm::vec3 position)
{
    if (scene->num_vertices >= SCENE_MAX_NUM_VERTICES)
        return NULL;

    uint32_t vertex_index = scene->num_vertices;

    Scene_Vertex* vertex = scene->vertices + vertex_index;
    vertex->position = position;
    vertex->num_outgoing_half_edges = 0;
    vertex->num_ingoing_half_edges = 0;

    ++scene->num_vertices;
    return vertex;
}

Scene_Face* Scene_ConstructFace(Scene* scene, Scene_Vertex** vertices, uint32_t num_vertices, glm::vec4 color)
{
    ASSERT(num_vertices > 0);

    if (scene->num_half_edges + num_vertices > SCENE_MAX_NUM_HALF_EDGES)
        return NULL;

    if (scene->num_faces >= SCENE_MAX_NUM_FACES)
        return NULL;

    Scene_Face* face = scene->faces + scene->num_faces;
    
    uint32_t half_edge_index_base = scene->num_half_edges;

    face->half_edge = scene->half_edges + half_edge_index_base;
    face->color = color;

    for (uint32_t i = 0; i < num_vertices; ++i)
    {
        Scene_Vertex* v0 = vertices[i];
        Scene_Vertex* v1 = vertices[(i + 1) % num_vertices];

        Scene_HalfEdge* current_half_edge = scene->half_edges + half_edge_index_base + i;
        Scene_HalfEdge* prev_half_edge    = scene->half_edges + half_edge_index_base + (i + num_vertices - 1) % num_vertices;
        Scene_HalfEdge* next_half_edge    = scene->half_edges + half_edge_index_base + (i + 1) % num_vertices;

        current_half_edge->origin_vertex      = v0;
        current_half_edge->end_vertex         = v1;
        current_half_edge->opposite_half_edge = NULL;
        current_half_edge->next_half_edge     = next_half_edge;
        current_half_edge->prev_half_edge     = prev_half_edge;
        current_half_edge->face               = face;

        ASSERT(v0->num_outgoing_half_edges < SCENE_VERTEX_MAX_NUM_OUTGOING_HALF_EDGES);
        ASSERT(v1->num_ingoing_half_edges < SCENE_VERTEX_MAX_NUM_INGOING_HALF_EDGES);

        v0->outgoing_half_edges[v0->num_outgoing_half_edges++] = current_half_edge;
        v1->ingoing_half_edges[v1->num_ingoing_half_edges++] = current_half_edge;

        for (uint32_t j = 0; j < v1->num_outgoing_half_edges; ++j)
        {
            Scene_HalfEdge* outgoing_half_edge = v1->outgoing_half_edges[j];

            if (outgoing_half_edge->end_vertex == v0)
            {
                current_half_edge->opposite_half_edge = outgoing_half_edge;
                outgoing_half_edge->opposite_half_edge = current_half_edge;

                break;
            }
        }
    }

    scene->num_half_edges += num_vertices;
    ++scene->num_faces;

    return face;
}

bool32_t Scene_GenerateGeometry(
    const Scene* scene,
    CVertex*     vertices,
    uint32_t     max_num_vertices,
    uint32_t*    indices,
    uint32_t     max_num_indices,
    uint32_t*    out_num_vertices,
    uint32_t*    out_num_indices
)
{
    uint32_t vertex_index = 0;
    uint32_t index_index = 0;

    for (uint32_t i = 0; i < scene->num_faces; ++i)
    {
        uint32_t start_vertex_index = vertex_index;

        const Scene_Face* current_face = scene->faces + i;
        
        const Scene_Vertex* start_vertex = current_face->half_edge->origin_vertex;
        const Scene_Vertex* next_vertex  = current_face->half_edge->end_vertex;
        const Scene_Vertex* prev_vertex  = current_face->half_edge->prev_half_edge->origin_vertex;

        glm::vec3 face_normal = glm::cross(
            next_vertex->position - start_vertex->position,
            prev_vertex->position - start_vertex->position
        );

        const Scene_HalfEdge* current_half_edge = current_face->half_edge;
        const Scene_Vertex*   current_vertex    = current_half_edge->origin_vertex;

        do
        {
            ASSERT(vertex_index < max_num_vertices);

            vertices[vertex_index].position = current_vertex->position;
            vertices[vertex_index].color    = current_face->color;
            vertices[vertex_index].normal   = face_normal;

            ++vertex_index;

            current_vertex = current_half_edge->end_vertex;
            current_half_edge = current_half_edge->next_half_edge;
        }
        while (current_vertex != start_vertex);

        uint32_t v0i = start_vertex_index;
        uint32_t v1i = start_vertex_index + 1;

        for (uint32_t vi = start_vertex_index + 2; vi < vertex_index; ++vi)
        {
            ASSERT(index_index < max_num_indices);

            indices[index_index++] = v0i;
            indices[index_index++] = v1i;
            indices[index_index++] = vi;

            v1i = vi;
        }
    }

    *out_num_vertices = vertex_index;
    *out_num_indices = index_index;

    return TRUE;
}

struct DrawElementsIndirectCommand
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

    Scene_Vertex* scene_vertices[4];

    Scene scene = {};
    scene_vertices[0] = Scene_AddVertex(&scene, { -3.0f, -3.0f, 0.0f });
    ASSERT(scene_vertices[0] != NULL);
    
    scene_vertices[1] = Scene_AddVertex(&scene, { 3.0f, -3.0f, 0.0f });
    ASSERT(scene_vertices[1] != NULL);

    scene_vertices[2] = Scene_AddVertex(&scene, { 3.0f,  3.0f, 0.0f });
    ASSERT(scene_vertices[2] != NULL);
    
    scene_vertices[3] = Scene_AddVertex(&scene, { -3.0f,  3.0f, 0.0f });
    ASSERT(scene_vertices[3] != NULL);

    Scene_Face* scene_face = Scene_ConstructFace(&scene, scene_vertices, ARRAY_SIZE_U32(scene_vertices), { 1.0f, 0.0f, 0.0f, 1.0f });
    ASSERT(scene_face != NULL);

    uint32_t max_num_vertices = 16 * 1024 * 1024;
    uint32_t max_num_indices = 16 * 1024 * 1024;

    GLuint vbo, ebo;
    glCreateBuffers(1, &vbo);
    glCreateBuffers(1, &ebo);

    glNamedBufferStorage(vbo, sizeof(CVertex) * max_num_vertices, NULL, GL_MAP_WRITE_BIT);
    glNamedBufferStorage(ebo, sizeof(uint32_t) * max_num_indices, NULL, GL_MAP_WRITE_BIT);

    CVertex* vertices = (CVertex*)glMapNamedBuffer(vbo, GL_WRITE_ONLY);
    uint32_t* indices = (uint32_t*)glMapNamedBuffer(ebo, GL_WRITE_ONLY);

    uint32_t num_vertices, num_indices;
    bool32_t generate_geometry_result = Scene_GenerateGeometry(&scene, vertices, max_num_vertices, indices, max_num_indices, &num_vertices, &num_indices);
    ASSERT(generate_geometry_result == TRUE);

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

    draw_commands[0].count = num_indices;
    draw_commands[0].instance_count = 1;
    draw_commands[0].first_index = 0;
    draw_commands[0].base_vertex = 0;
    draw_commands[0].base_instance = 0;

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
        //model = glm::rotate(model, time, { 0.0f, 1.0f, 0.0f });

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