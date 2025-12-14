#include <stdio.h>
#include <float.h>

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

#define SCENE_ID_NONE ((uint32_t)-1)

struct LVertex
{
    glm::vec3  position;
    glm::vec3  normal;
    glm::vec4  color;
    glm::uvec3 cell_ids;
};

struct Scene_Vertex;
struct Scene_HalfEdge;
struct Scene_Face;

struct Scene_Vertex
{
    uint32_t id;

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
    uint32_t id;

    glm::vec4 color;

    glm::vec3 normal;
    float offset;

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
    vertex->id = vertex_index;
    vertex->position = position;
    vertex->num_outgoing_half_edges = 0;
    vertex->num_ingoing_half_edges = 0;

    ++scene->num_vertices;
    return vertex;
}

Scene_Face* Scene_ConstructFace(Scene* scene, Scene_Vertex** vertices, uint32_t num_vertices, glm::vec4 color)
{
    ASSERT(num_vertices >= 3);

    if (scene->num_half_edges + num_vertices > SCENE_MAX_NUM_HALF_EDGES)
        return NULL;

    if (scene->num_faces >= SCENE_MAX_NUM_FACES)
        return NULL;

    uint32_t half_edge_index_base = scene->num_half_edges;

    Scene_Vertex* start_vertex = vertices[0];
    Scene_Vertex* next_vertex = vertices[1];
    Scene_Vertex* prev_vertex = vertices[num_vertices - 1];

    glm::vec3 face_normal = glm::normalize(
        glm::cross(
            next_vertex->position - start_vertex->position,
            prev_vertex->position - start_vertex->position
        )
    );

    Scene_Face* face = scene->faces + scene->num_faces;
    face->id         = scene->num_faces;
    face->half_edge  = scene->half_edges + half_edge_index_base;
    face->color      = color;
    face->normal     = face_normal;
    face->offset     = -glm::dot(face_normal, start_vertex->position);

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
    LVertex*     vertices,
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

        const Scene_HalfEdge* current_half_edge = current_face->half_edge;
        const Scene_Vertex*   current_vertex    = current_half_edge->origin_vertex;

        const Scene_Vertex* start_vertex = current_vertex;

        do
        {
            ASSERT(vertex_index < max_num_vertices);

            LVertex* geometry_vertex = vertices + vertex_index;
            geometry_vertex->position   = current_vertex->position;
            geometry_vertex->normal     = current_face->normal;
            geometry_vertex->color      = current_face->color;
            geometry_vertex->cell_ids.x = current_vertex->id;
            geometry_vertex->cell_ids.z = current_face->id;

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

// NOTE: ray_direction must be a unit vector
bool32_t Scene_RayCast_FindNearestIntersectingFace(
    Scene*       scene,
    glm::vec3    ray_origin,
    glm::vec3    ray_direction,
    float        ray_min_length,
    float        ray_max_length,
    Scene_Face** out_intersecting_face,
    glm::vec3*   out_intersection
)
{
    const float eps = 10e-5f;

    float       hit_min_distance = FLT_MAX;
    Scene_Face* hit_face         = NULL;
    glm::vec3   hit_intersection = {};

    for (uint32_t i = 0; i < scene->num_faces; ++i)
    {
        Scene_Face* current_face = scene->faces + i;

        float denom = glm::dot(current_face->normal, ray_direction);

        if (denom > -eps && denom < eps)
            continue;

        float t = -(current_face->offset + glm::dot(current_face->normal, ray_origin)) / denom;
        if (t < ray_min_length || t > ray_max_length)
            continue;

        if (t < hit_min_distance)
        {
            glm::vec3 intersection = ray_origin + t * ray_direction;

            Scene_HalfEdge* current_half_edge = current_face->half_edge;
            Scene_Vertex*   current_vertex    = current_half_edge->origin_vertex;

            Scene_Vertex* start_vertex = current_vertex;

            do
            {
                Scene_Vertex* next_vertex = current_half_edge->end_vertex;

                glm::vec3 w = glm::cross(
                    next_vertex->position - current_vertex->position,
                    intersection - current_vertex->position
                );

                if (glm::dot(w, current_face->normal) < 0.0f)
                    goto fail;

                current_vertex = next_vertex;
                current_half_edge = current_half_edge->next_half_edge;
            }
            while (current_vertex != start_vertex);

            hit_min_distance = t;
            hit_face = current_face;
            hit_intersection = intersection;
        }

    fail:
        continue;
    }

    if (!hit_face)
        return FALSE;

    if (out_intersecting_face) *out_intersecting_face = hit_face;
    if (out_intersection) *out_intersection = hit_intersection;

    return TRUE;
}

struct Camera
{
    glm::vec3 position;
    float yaw;
    float pitch;

    glm::vec3 right;
    glm::vec3 forward;
    glm::vec3 up;

    glm::mat4 view;
};

void Camera_MoveStraight(Camera* camera, float distance)
{
    camera->position += distance * camera->forward;
}

void Camera_Strafe(Camera* camera, float distance)
{
    camera->position += distance * camera->right;
}

void Camera_Rotate(Camera* camera, float delta_yaw, float delta_pitch)
{
    constexpr float max_pitch = glm::radians(89.0f);
    constexpr float min_pitch = -glm::radians(89.0f);

    camera->yaw += delta_yaw;

    camera->pitch += delta_pitch;

    if (camera->pitch > max_pitch)
        camera->pitch = max_pitch;

    if (camera->pitch < min_pitch)
        camera->pitch = min_pitch;
}

void Camera_RecomputeDirectionVectors(Camera* camera)
{
    camera->forward.x = cosf(camera->yaw) * cosf(camera->pitch);
    camera->forward.y = sinf(camera->pitch);
    camera->forward.z = sinf(camera->yaw) * cosf(camera->pitch);

    camera->right.x = -sinf(camera->yaw);
    camera->right.y = 0.0f;
    camera->right.z = cosf(camera->yaw);

    camera->up = glm::cross(camera->right, camera->forward);
}

void Camera_RecomputeViewMatrix(Camera* camera)
{
    camera->view = glm::lookAt(camera->position, camera->position + camera->forward, camera->up);
}

struct DrawElementsIndirectCommand
{
    GLuint count;
    GLuint instance_count;
    GLuint first_index;
    GLint  base_vertex;
    GLuint base_instance;
};

static bool32_t Input_Cursor_Locked = TRUE;

static bool32_t Input_Key_Pressed_W;
static bool32_t Input_Key_Pressed_A;
static bool32_t Input_Key_Pressed_S;
static bool32_t Input_Key_Pressed_D;

static void Input_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    UNUSED(window);
    UNUSED(mods);
    UNUSED(scancode);

    if (action == GLFW_PRESS || action == GLFW_RELEASE)
    {
        switch (key)
        {
        case GLFW_KEY_W: Input_Key_Pressed_W = (action == GLFW_PRESS); break;
        case GLFW_KEY_A: Input_Key_Pressed_A= (action == GLFW_PRESS); break;
        case GLFW_KEY_S: Input_Key_Pressed_S = (action == GLFW_PRESS); break;
        case GLFW_KEY_D: Input_Key_Pressed_D = (action == GLFW_PRESS); break;
        }
    }
}

static bool32_t Input_MouseMotion_FirstMotion = TRUE;
static float Input_MouseMotion_DeltaX = 0.0f;
static float Input_MouseMotion_DeltaY = 0.0f;
static float Input_MouseMotion_LastX = 0.0f;
static float Input_MouseMotion_LastY = 0.0f;

static void Input_MouseMotionCallback(GLFWwindow* window, double xpos, double ypos)
{
    UNUSED(window);

    float x = (float)xpos;
    float y = (float)ypos;

    Input_MouseMotion_LastX = x;
    Input_MouseMotion_LastY = y;

    static float last_x;
    static float last_y;

    if (Input_MouseMotion_FirstMotion)
    {
        last_x = x;
        last_y = y;
        
        Input_MouseMotion_FirstMotion = FALSE;
        return;
    }

    Input_MouseMotion_DeltaX = x - last_x;
    Input_MouseMotion_DeltaY = y - last_y;

    last_x = x;
    last_y = y;
}

static void Input_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    UNUSED(mods);

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            Input_Cursor_Locked = FALSE;

            Input_MouseMotion_FirstMotion = TRUE;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            Input_Cursor_Locked = TRUE;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
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

    glfwSetKeyCallback(window, Input_KeyCallback);
    glfwSetCursorPosCallback(window, Input_MouseMotionCallback);
    glfwSetMouseButtonCallback(window, Input_MouseButtonCallback);

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

    Scene_Vertex* scene_vertices[6];

    Scene scene = {};
    scene_vertices[0] = Scene_AddVertex(&scene, { -3.0f, -3.0f, 0.0f });
    ASSERT(scene_vertices[0] != NULL);
    
    scene_vertices[1] = Scene_AddVertex(&scene, { 3.0f, -3.0f, 0.0f });
    ASSERT(scene_vertices[1] != NULL);

    scene_vertices[2] = Scene_AddVertex(&scene, { 3.0f,  3.0f, 0.0f });
    ASSERT(scene_vertices[2] != NULL);
    
    scene_vertices[3] = Scene_AddVertex(&scene, { -3.0f,  3.0f, 0.0f });
    ASSERT(scene_vertices[3] != NULL);

    scene_vertices[4] = Scene_AddVertex(&scene, {  6.0f, -3.0f, 0.0f });
    ASSERT(scene_vertices[4] != NULL);

    scene_vertices[5] = Scene_AddVertex(&scene, {  6.0f,  3.0f, 0.0f });
    ASSERT(scene_vertices[5] != NULL);

    Scene_Vertex* f0v[4] = { scene_vertices[0], scene_vertices[1] ,scene_vertices[2], scene_vertices[3] };
    Scene_Vertex* f1v[4] = { scene_vertices[1], scene_vertices[4], scene_vertices[5], scene_vertices[2] };

    Scene_Face* f0 = Scene_ConstructFace(&scene, f0v, ARRAY_SIZE_U32(f0v), { 1.0f, 0.0f, 0.0f, 1.0f });
    ASSERT(f0 != NULL);
    
    Scene_Face* f1 = Scene_ConstructFace(&scene, f1v, ARRAY_SIZE_U32(f1v), { 0.0f, 1.0f, 0.0f, 1.0f });
    ASSERT(f1 != NULL);

    uint32_t max_num_vertices = 1024;
    uint32_t max_num_indices = 1024;

    GLuint vbo, ebo;
    glCreateBuffers(1, &vbo);
    glCreateBuffers(1, &ebo);

    glNamedBufferStorage(vbo, sizeof(LVertex) * max_num_vertices, NULL, GL_MAP_WRITE_BIT);
    glNamedBufferStorage(ebo, sizeof(uint32_t) * max_num_indices, NULL, GL_MAP_WRITE_BIT);

#if 0
    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(LVertex));
    glVertexArrayElementBuffer(vao, ebo);

    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(LVertex, position));
    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(LVertex, normal));
    glVertexArrayAttribFormat(vao, 2, 4, GL_FLOAT, GL_FALSE, offsetof(LVertex, color));

    // NOTE: The following call apparently does not work correctly on some Intel iGPUs (I've been testing the code on one such iGPU)
    // See: https://community.intel.com/t5/Graphics/glVertexArrayAttribIFormat-not-working-correctly-on-Intel-HD/td-p/1687827
    glVertexArrayAttribIFormat(vao, 3, 3, GL_UNSIGNED_INT, offsetof(LVertex, cell_ids));

    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);
    glEnableVertexArrayAttrib(vao, 2);
    glEnableVertexArrayAttrib(vao, 3);

    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 0);
    glVertexArrayAttribBinding(vao, 2, 0);
    glVertexArrayAttribBinding(vao, 3, 0);
#else
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LVertex), (const void*)offsetof(LVertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LVertex), (const void*)offsetof(LVertex, normal));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(LVertex), (const void*)offsetof(LVertex, color));
    glVertexAttribIPointer(3, 3, GL_UNSIGNED_INT, sizeof(LVertex), (const void*)offsetof(LVertex, cell_ids));
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
#endif

#if 0
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

    glFlushMappedNamedBufferRange(draw_indirect_buffer_object, 0, sizeof(DrawElementsIndirectCommand) * max_num_draw_commands);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_indirect_buffer_object);
#endif

    constexpr float fovy = glm::radians(45.0f);
    constexpr float near = 0.1f;

    float aspect = (float)window_width / window_height;

    float near_half_height = near * tanf(fovy * 0.5f);
    float near_half_width = aspect * near_half_height;

    glm::mat4 identity(1.0f);
    glm::mat4 projection = glm::perspective(fovy, aspect, near, 100.0f);

    glUniformMatrix4fv(0, 1, GL_FALSE, (float*)&projection);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    Camera camera;
    camera.position = { 0.0f, 0.0f, 5.0f };
    camera.pitch = 0.0f;
    camera.yaw = -3.14f * 0.5f;

    Camera_RecomputeDirectionVectors(&camera);
    Camera_RecomputeViewMatrix(&camera);

    float last_time = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float current_time = (float)glfwGetTime();

        uint32_t picked_face_id = SCENE_ID_NONE;

        if (last_time != 0.0f)
        {
            float delta_time = current_time - last_time;

            if (!Input_Cursor_Locked)
            {
                if (Input_Key_Pressed_W) Camera_MoveStraight(&camera, 5.0f * delta_time);
                if (Input_Key_Pressed_S) Camera_MoveStraight(&camera, -5.0f * delta_time);
                if (Input_Key_Pressed_D) Camera_Strafe(&camera, 5.0f * delta_time);
                if (Input_Key_Pressed_A) Camera_Strafe(&camera, -5.0f * delta_time);

                Camera_Rotate(&camera, Input_MouseMotion_DeltaX * 2.0f * delta_time, -Input_MouseMotion_DeltaY * 2.0f * delta_time);
                Input_MouseMotion_DeltaX = 0.0f;
                Input_MouseMotion_DeltaY = 0.0f;

                Camera_RecomputeDirectionVectors(&camera);
                Camera_RecomputeViewMatrix(&camera);
            }
            else
            {
                float relative_mouse_x = 2.0f * ((Input_MouseMotion_LastX / window_width) - 0.5f);
                float relative_mouse_y = 2.0f * (0.5f - (Input_MouseMotion_LastY / window_height));

                glm::vec3 near_forward = near * camera.forward;
                glm::vec3 near_right = camera.right * near_half_width;
                glm::vec3 near_up = camera.up * near_half_height;

                glm::vec3 pick_direction = glm::normalize(near_forward + relative_mouse_x * near_right + relative_mouse_y * near_up);

                bool32_t shift_down = Input_Key_Pressed_S;
                bool32_t shift_up = Input_Key_Pressed_W;

                Scene_Face* hit_face;
                if (Scene_RayCast_FindNearestIntersectingFace(&scene, camera.position, pick_direction, 0.01f, 100.0f, &hit_face, NULL))
                {
                    picked_face_id = hit_face->id;

                    Scene_HalfEdge* current_half_edge = hit_face->half_edge;
                    Scene_Vertex* current_vertex = current_half_edge->origin_vertex;

                    Scene_Vertex* start_vertex = current_vertex;

                    do
                    {
                        if (shift_up) current_vertex->position.y += delta_time;
                        if (shift_down) current_vertex->position.y -= delta_time;

                        current_vertex = current_half_edge->end_vertex;
                        current_half_edge = current_half_edge->next_half_edge;
                    } while (current_vertex != start_vertex);
                }
            }
        }

        last_time = current_time;

        // Regenerate the scene geometry

        LVertex* vertices = (LVertex*)glMapNamedBuffer(vbo, GL_WRITE_ONLY);
        uint32_t* indices = (uint32_t*)glMapNamedBuffer(ebo, GL_WRITE_ONLY);

        uint32_t num_vertices, num_indices;
        bool32_t generate_geometry_result = Scene_GenerateGeometry(&scene, vertices, max_num_vertices, indices, max_num_indices, &num_vertices, &num_indices);
        ASSERT(generate_geometry_result == TRUE);

        bool32_t unmap_vbo_result = glUnmapNamedBuffer(vbo);
        ASSERT(unmap_vbo_result == TRUE);

        bool32_t unmap_ebo_result = glUnmapNamedBuffer(ebo);
        ASSERT(unmap_ebo_result == TRUE);

        // Setup for rendering

        glUniformMatrix4fv(1, 1, GL_FALSE, (float*)&camera.view);
        glUniformMatrix4fv(2, 1, GL_FALSE, (float*)&identity);
        glUniform1ui(3, picked_face_id);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, (const void*)0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}