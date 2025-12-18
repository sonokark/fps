#include <stdio.h>
#include <float.h>

#include "Common.hpp"
#include "OpenGL.hpp"
#include "OpenGL_Shader.hpp"
#include "Geometry.hpp"
#include "Arena.hpp"
#include "Scene.hpp"
#include "Camera.hpp"

#include <GLFW/glfw3.h>

#define EDITOR_GEOMETRY_PERMANENT_MAX_NUM_VERTICES 1024
#define EDITOR_GEOMETRY_PERMANENT_MAX_NUM_INDICES 1024

#define EDITOR_GEOMETRY_SCENE_MAX_NUM_VERTICES 1024
#define EDITOR_GEOMETRY_SCENE_MAX_NUM_INDICES 1024

struct Editor_Geometry_Permanent_VertexBufferLayout
{
    PVertex vertices[EDITOR_GEOMETRY_PERMANENT_MAX_NUM_VERTICES];
};

struct Editor_Geometry_Permanent_IndexBufferLayout
{
    uint32_t indices[EDITOR_GEOMETRY_PERMANENT_MAX_NUM_INDICES];
};

struct Editor_Geometry_Scene_VertexBufferLayout
{
    SVertex vertices[EDITOR_GEOMETRY_SCENE_MAX_NUM_VERTICES];
};

struct Editor_Geometry_Scene_IndexBufferLayout
{  
    uint32_t indices[EDITOR_GEOMETRY_SCENE_MAX_NUM_INDICES];
};

#define EDITOR_GEOMETRY_MAX_NUM_POINTS 128
#define EDITOR_GEOMETRY_MAX_NUM_GRIDS 8

struct Editor_Geometry_DataSSBOLayout
{
    alignas(sizeof(float) * 4) glm::vec4 point_positions[EDITOR_GEOMETRY_MAX_NUM_POINTS];

    alignas(sizeof(float) * 4) glm::mat4 grid_transforms[EDITOR_GEOMETRY_MAX_NUM_GRIDS];
    alignas(sizeof(float) * 4) glm::vec4 grid_colors[EDITOR_GEOMETRY_MAX_NUM_GRIDS];
};

struct Editor_Geometry_Permanent
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    uint32_t grid_base_vertex;
    uint32_t grid_indices_offset;
    uint32_t grid_num_indices;

    uint32_t point_base_vertex;
    uint32_t point_indices_offset;
    uint32_t point_num_indices;
};

struct Editor_Geometry_Scene
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    uint32_t num_vertices;
    uint32_t num_indices;
};

struct Editor_Geometry
{
    Editor_Geometry_Permanent permanent_geometry;
    Editor_Geometry_Scene scene_geometry;

    GLuint data_ssbo;
    
    uint32_t num_grids;
    uint32_t num_points;
};

bool32_t Editor_Geometry_Permanent_Init(Editor_Geometry_Permanent* geometry)
{
    constexpr uint32_t vertex_buffer_size = sizeof(Editor_Geometry_Permanent_VertexBufferLayout);
    constexpr uint32_t index_buffer_size = sizeof(Editor_Geometry_Permanent_IndexBufferLayout);

    GLuint buffers[2];
    glCreateBuffers(ARRAY_SIZE_U32(buffers), buffers);

    GLuint vbo = buffers[0];
    GLuint ebo = buffers[1];

    glNamedBufferStorage(vbo, vertex_buffer_size, NULL, GL_MAP_WRITE_BIT);
    glNamedBufferStorage(ebo, index_buffer_size, NULL, GL_MAP_WRITE_BIT);

    Editor_Geometry_Permanent_VertexBufferLayout* vertex_buffer_data = (Editor_Geometry_Permanent_VertexBufferLayout*)glMapNamedBuffer(
        vbo, GL_WRITE_ONLY
    );

    ASSERT(vertex_buffer_data != NULL);

    Editor_Geometry_Permanent_IndexBufferLayout* index_buffer_data = (Editor_Geometry_Permanent_IndexBufferLayout*)glMapNamedBuffer(
        ebo, GL_WRITE_ONLY
    );

    ASSERT(index_buffer_data != NULL);

    // Push the geometry
    {
        uint32_t num_pushed_vertices = 0;
        uint32_t num_pushed_indices = 0;

        // Push the grid geometry
        {
            PVertex* const current_vertex = vertex_buffer_data->vertices + num_pushed_vertices;
            uint32_t* const current_index = index_buffer_data->indices + num_pushed_indices;

            const uint32_t num_remaining_vertices = EDITOR_GEOMETRY_PERMANENT_MAX_NUM_VERTICES - num_pushed_vertices;
            const uint32_t num_remaining_indices = EDITOR_GEOMETRY_PERMANENT_MAX_NUM_INDICES - num_pushed_indices;

            constexpr glm::vec2  min  = { -10, -10 };
            constexpr glm::vec2  max  = {  10,  10 };
            constexpr glm::uvec2 axes = {   0,   2 };

            Geometry_NumVerticesAndIndices nvi = Geometry_Grid_GetNumRequiredVerticesAndIndices(min, max);

            bool32_t push_result = Geometry_Grid_Push(
                current_vertex,
                num_remaining_vertices,
                current_index,
                num_remaining_indices,
                axes,
                min,
                max
            );

            ASSERT(push_result == TRUE);

            geometry->grid_base_vertex = num_pushed_vertices;
            geometry->grid_indices_offset = num_pushed_indices * sizeof(uint32_t);
            geometry->grid_num_indices = nvi.num_indices;

            num_pushed_vertices += nvi.num_vertices;
            num_pushed_indices += nvi.num_indices;
        }

        // Push the point geometry
        {
            PVertex* const current_vertex = vertex_buffer_data->vertices + num_pushed_vertices;
            uint32_t* const current_index = index_buffer_data->indices + num_pushed_indices;

            const uint32_t num_remaining_vertices = EDITOR_GEOMETRY_PERMANENT_MAX_NUM_VERTICES - num_pushed_vertices;
            const uint32_t num_remaining_indices = EDITOR_GEOMETRY_PERMANENT_MAX_NUM_INDICES - num_pushed_indices;

            constexpr glm::vec2 size = { 0.1f, 0.1f };

            Geometry_NumVerticesAndIndices nvi = Geometry_Point_GetNumRequiredVerticesAndIndices();

            bool32_t push_result = Geometry_Point_Push(
                current_vertex,
                num_remaining_vertices,
                current_index,
                num_remaining_indices,
                size
            );

            ASSERT(push_result == TRUE);

            geometry->point_base_vertex = num_pushed_vertices;
            geometry->point_indices_offset = num_pushed_indices * sizeof(uint32_t);
            geometry->point_num_indices = nvi.num_indices;

            num_pushed_vertices += nvi.num_vertices;
            num_pushed_indices += nvi.num_indices;
        }
    }

    GLboolean vertex_buffer_unmap_result = glUnmapNamedBuffer(vbo);
    ASSERT(vertex_buffer_unmap_result == TRUE);

    GLboolean index_buffer_unmap_result = glUnmapNamedBuffer(ebo);
    ASSERT(index_buffer_unmap_result == TRUE);

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(PVertex));
    glVertexArrayElementBuffer(vao, ebo);

    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(PVertex, position));
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    geometry->vao = vao;
    geometry->vbo = vbo;
    geometry->ebo = ebo;

    return TRUE;
}

bool32_t Editor_Geometry_Scene_Init(Editor_Geometry_Scene* geometry)
{
    constexpr uint32_t vertex_buffer_size = sizeof(Editor_Geometry_Scene_VertexBufferLayout);
    constexpr uint32_t index_buffer_size = sizeof(Editor_Geometry_Scene_IndexBufferLayout);

    GLuint buffers[2];
    glCreateBuffers(ARRAY_SIZE_U32(buffers), buffers);

    GLuint vbo = buffers[0];
    GLuint ebo = buffers[1];

    glNamedBufferStorage(vbo, vertex_buffer_size, NULL, GL_MAP_WRITE_BIT);
    glNamedBufferStorage(ebo, index_buffer_size, NULL, GL_MAP_WRITE_BIT);

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), (const void*)offsetof(SVertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), (const void*)offsetof(SVertex, normal));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SVertex), (const void*)offsetof(SVertex, color));
    glVertexAttribIPointer(3, 3, GL_UNSIGNED_INT, sizeof(SVertex), (const void*)offsetof(SVertex, cell_ids));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
#endif

    geometry->vao = vao;
    geometry->vbo = vbo;
    geometry->ebo = ebo;
    geometry->num_indices = 0;

    return TRUE;
}

bool32_t Editor_Geometry_Init(Editor_Geometry* geometry)
{
    bool32_t init_permanent_geometry_result = Editor_Geometry_Permanent_Init(&geometry->permanent_geometry);
    ASSERT(init_permanent_geometry_result == TRUE);

    bool32_t init_scene_geometry_result = Editor_Geometry_Scene_Init(&geometry->scene_geometry);
    ASSERT(init_scene_geometry_result == TRUE);

    GLuint data_ssbo;
    glCreateBuffers(1, &data_ssbo);
    glNamedBufferStorage(data_ssbo, sizeof(Editor_Geometry_DataSSBOLayout), NULL, GL_MAP_WRITE_BIT);

    // Fill the SSBO with some initial data
    {
        Editor_Geometry_DataSSBOLayout* data = (Editor_Geometry_DataSSBOLayout*)glMapNamedBuffer(data_ssbo, GL_WRITE_ONLY);
        ASSERT(data != NULL);

        data->grid_transforms[0] = glm::mat4(1.0f);
        data->grid_colors[0] = { 0.2f, 0.2f, 0.2f, 1.0f };

        GLboolean unmap_result = glUnmapNamedBuffer(data_ssbo);
        ASSERT(unmap_result == GL_TRUE);

        geometry->num_grids = 1;
        geometry->num_points = 0;
    }

    geometry->data_ssbo = data_ssbo;
    
    return TRUE;
}

bool32_t Editor_Geometry_Update(Editor_Geometry* geometry, const Scene* scene)
{
    // Update the scene geometry first
    {
        Editor_Geometry_Scene_VertexBufferLayout* vertex_buffer_data = (Editor_Geometry_Scene_VertexBufferLayout*)glMapNamedBuffer(
            geometry->scene_geometry.vbo,
            GL_WRITE_ONLY
        );

        ASSERT(vertex_buffer_data != NULL);

        Editor_Geometry_Scene_IndexBufferLayout* index_buffer_data = (Editor_Geometry_Scene_IndexBufferLayout*)glMapNamedBuffer(
            geometry->scene_geometry.ebo,
            GL_WRITE_ONLY
        );

        ASSERT(index_buffer_data != NULL);

        uint32_t num_vertices, num_indices;
        bool32_t generate_geometry_result = Scene_GenerateGeometry(
            scene,
            vertex_buffer_data->vertices,
            EDITOR_GEOMETRY_SCENE_MAX_NUM_VERTICES,
            index_buffer_data->indices,
            EDITOR_GEOMETRY_SCENE_MAX_NUM_INDICES,
            &num_vertices,
            &num_indices
        );
        
        ASSERT(generate_geometry_result == TRUE);

        bool32_t unmap_vbo_result = glUnmapNamedBuffer(geometry->scene_geometry.vbo);
        ASSERT(unmap_vbo_result == TRUE);

        bool32_t unmap_ebo_result = glUnmapNamedBuffer(geometry->scene_geometry.ebo);
        ASSERT(unmap_ebo_result == TRUE);

        geometry->scene_geometry.num_vertices = num_vertices;
        geometry->scene_geometry.num_indices = num_indices;
    }

    // Set the SSBO data
    {
        Editor_Geometry_DataSSBOLayout* data = (Editor_Geometry_DataSSBOLayout*)glMapNamedBuffer(geometry->data_ssbo, GL_WRITE_ONLY);
        ASSERT(data != NULL);

        for (uint32_t i = 0; i < scene->num_vertices; ++i)
        {
            const Scene_Vertex* vertex = scene->vertices + i;

            data->point_positions[i] = glm::vec4(vertex->position, 1.0f);
        }

        geometry->num_points = scene->num_vertices;

        GLboolean unmap_result = glUnmapNamedBuffer(geometry->data_ssbo);
        ASSERT(unmap_result == GL_TRUE);
    }

    return TRUE;
}

static bool32_t Input_Cursor_Locked = TRUE;

static bool32_t Input_Key_Pressed_W;
static bool32_t Input_Key_Pressed_A;
static bool32_t Input_Key_Pressed_S;
static bool32_t Input_Key_Pressed_D;
static bool32_t Input_Key_Pressed_Space;
static bool32_t Input_Key_Pressed_Shift;

static void Input_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    UNUSED(window);
    UNUSED(mods);
    UNUSED(scancode);

    if (action == GLFW_PRESS || action == GLFW_RELEASE)
    {
        switch (key)
        {
        case GLFW_KEY_W:     Input_Key_Pressed_W     = (action == GLFW_PRESS); break;
        case GLFW_KEY_A:     Input_Key_Pressed_A     = (action == GLFW_PRESS); break;
        case GLFW_KEY_S:     Input_Key_Pressed_S     = (action == GLFW_PRESS); break;
        case GLFW_KEY_D:     Input_Key_Pressed_D     = (action == GLFW_PRESS); break;
        case GLFW_KEY_SPACE: Input_Key_Pressed_Space = (action == GLFW_PRESS); break;

        case GLFW_KEY_RIGHT_SHIFT:
        case GLFW_KEY_LEFT_SHIFT:
            Input_Key_Pressed_Shift = (action == GLFW_PRESS);
            break;
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

    if (Input_MouseMotion_FirstMotion)
    {
        Input_MouseMotion_LastX = x;
        Input_MouseMotion_LastY = y;

        Input_MouseMotion_FirstMotion = FALSE;
        return;
    }

    Input_MouseMotion_DeltaX = x - Input_MouseMotion_LastX;
    Input_MouseMotion_DeltaY = y - Input_MouseMotion_LastY;

    Input_MouseMotion_LastX = x;
    Input_MouseMotion_LastY = y;
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

    GLuint program_scene = OpenGL_Shader_CreateProgramFromSources(OpenGL_Shader_Scene_VertexSource, OpenGL_Shader_Scene_FragmentSource);
    ASSERT(program_scene != 0);

    GLuint program_editor_geometry;
    GLuint program_editor_point;
    {
        GLuint shaders[3];

        shaders[0] = OpenGL_Shader_CreateShader(GL_VERTEX_SHADER, OpenGL_Shader_Editor_Geometry_VertexSource);
        ASSERT(shaders[0] != 0);

        shaders[1] = OpenGL_Shader_CreateShader(GL_FRAGMENT_SHADER, OpenGL_Shader_Editor_Geometry_FragmentSource);
        ASSERT(shaders[1] != 0);

        shaders[2] = OpenGL_Shader_CreateShader(GL_VERTEX_SHADER, OpenGL_Shader_Editor_Point_VertexSource);
        ASSERT(shaders[2] != 0);

        program_editor_geometry = OpenGL_Shader_CreateProgram(shaders, 2);
        ASSERT(program_editor_geometry != 0);

        program_editor_point = OpenGL_Shader_CreateProgram(shaders + 1, 2);
        ASSERT(program_editor_point != 0);

        glDeleteShader(shaders[0]);
        glDeleteShader(shaders[1]);
        glDeleteShader(shaders[2]);
    }

    Scene scene = {};
    {
        Scene_Vertex* scene_vertices[6];

        scene_vertices[0] = Scene_AddVertex(&scene, { -3.0f, 0.0f, 0.0f });
        ASSERT(scene_vertices[0] != NULL);

        scene_vertices[1] = Scene_AddVertex(&scene, { 3.0f, 0.0f, 0.0f });
        ASSERT(scene_vertices[1] != NULL);

        scene_vertices[2] = Scene_AddVertex(&scene, { 3.0f, 1.0f, 0.0f });
        ASSERT(scene_vertices[2] != NULL);

        scene_vertices[3] = Scene_AddVertex(&scene, { -3.0f, 1.0f, 0.0f });
        ASSERT(scene_vertices[3] != NULL);

        scene_vertices[4] = Scene_AddVertex(&scene, { 6.0f, 0.0f, 0.0f });
        ASSERT(scene_vertices[4] != NULL);

        scene_vertices[5] = Scene_AddVertex(&scene, { 6.0f, 1.0f, 0.0f });
        ASSERT(scene_vertices[5] != NULL);

        Scene_Vertex* f0v[4] = { scene_vertices[0], scene_vertices[1] ,scene_vertices[2], scene_vertices[3] };
        Scene_Vertex* f1v[4] = { scene_vertices[1], scene_vertices[4], scene_vertices[5], scene_vertices[2] };

        Scene_Face* f0 = Scene_ConstructFace(&scene, f0v, ARRAY_SIZE_U32(f0v), { 1.0f, 0.0f, 0.0f, 1.0f });
        ASSERT(f0 != NULL);

        Scene_Face* f1 = Scene_ConstructFace(&scene, f1v, ARRAY_SIZE_U32(f1v), { 0.0f, 1.0f, 0.0f, 1.0f });
        ASSERT(f1 != NULL);
    }

    Editor_Geometry editor_geometry;
    bool32_t editor_geometry_init_result = Editor_Geometry_Init(&editor_geometry);
    ASSERT(editor_geometry_init_result == TRUE);

    constexpr float fovy = glm::radians(45.0f);
    constexpr float near = 0.1f;

    float aspect = (float)window_width / window_height;

    float near_half_height = near * tanf(fovy * 0.5f);
    float near_half_width = aspect * near_half_height;

    glm::mat4 identity(1.0f);
    glm::mat4 projection = glm::perspective(fovy, aspect, near, 100.0f);

    Camera camera;
    camera.position = { 0.0f, 0.0f, 5.0f };
    camera.pitch = 0.0f;
    camera.yaw = -3.14f * 0.5f;

    Camera_RecomputeDirectionVectors(&camera);
    Camera_RecomputeViewMatrix(&camera);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, editor_geometry.data_ssbo);

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    float last_time = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float current_time = (float)glfwGetTime();

        uint32_t picked_face_id = SCENE_ID_NONE;
        uint32_t picked_vertex_id = SCENE_ID_NONE;

        if (last_time != 0.0f)
        {
            float delta_time = current_time - last_time;

            if (!Input_Cursor_Locked)
            {
                if (Input_Key_Pressed_W) Camera_MoveStraight(&camera, 5.0f * delta_time);
                if (Input_Key_Pressed_S) Camera_MoveStraight(&camera, -5.0f * delta_time);
                if (Input_Key_Pressed_D) Camera_Strafe(&camera, 5.0f * delta_time);
                if (Input_Key_Pressed_A) Camera_Strafe(&camera, -5.0f * delta_time);
                if (Input_Key_Pressed_Space) Camera_MoveVertically(&camera, 5.0f * delta_time);
                if (Input_Key_Pressed_Shift) Camera_MoveVertically(&camera, -5.0f * delta_time);

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
                glm::vec3 near_right = near_half_width * camera.right;
                glm::vec3 near_up = near_half_height * camera.up;

                glm::vec3 pick_direction = glm::normalize(near_forward + relative_mouse_x * near_right + relative_mouse_y * near_up);

                bool32_t face_shift_down = Input_Key_Pressed_S;
                bool32_t face_shift_up = Input_Key_Pressed_W;
                bool32_t vertex_shift_up = Input_Key_Pressed_A;
                bool32_t vertex_shift_down = Input_Key_Pressed_D;

                Scene_Face* hit_face;
                if (Scene_RayCast_FindNearestIntersectingFace(&scene, camera.position, pick_direction, 0.01f, 100.0f, &hit_face, NULL))
                {
                    picked_face_id = hit_face->id;

                    Scene_HalfEdge* current_half_edge = hit_face->half_edge;
                    Scene_Vertex* current_vertex = current_half_edge->origin_vertex;

                    Scene_Vertex* start_vertex = current_vertex;

                    do
                    {
                        if (face_shift_up) current_vertex->position.y += delta_time;
                        if (face_shift_down) current_vertex->position.y -= delta_time;

                        current_vertex = current_half_edge->end_vertex;
                        current_half_edge = current_half_edge->next_half_edge;
                    } while (current_vertex != start_vertex);
                }

                Scene_Vertex* hit_vertex = Scene_RayCast_FindNearestVertex(&scene, camera.position, pick_direction, 100.0f);
                if (hit_vertex)
                {
                    picked_vertex_id = hit_vertex->id;

                    if (vertex_shift_up) hit_vertex->position.y += delta_time;
                    if (vertex_shift_down) hit_vertex->position.y -= delta_time;
                }
            }
        }

        last_time = current_time;

        // Update the editor geometry

        bool32_t editor_geometry_update_result = Editor_Geometry_Update(&editor_geometry, &scene);
        ASSERT(editor_geometry_update_result == TRUE);

        // Setup for rendering

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Scene
     
        glUseProgram(program_scene);

        glUniformMatrix4fv(0, 1, GL_FALSE, (float*)&projection);
        glUniformMatrix4fv(1, 1, GL_FALSE, (float*)&camera.view);
        glUniformMatrix4fv(2, 1, GL_FALSE, (float*)&identity);
        glUniform1ui(3, picked_face_id);
    
        glBindVertexArray(editor_geometry.scene_geometry.vao);
        glDrawElements(GL_TRIANGLES, editor_geometry.scene_geometry.num_indices, GL_UNSIGNED_INT, (const void*)0);
        
        // Grid

        glUseProgram(program_editor_geometry);

        glUniformMatrix4fv(0, 1, GL_FALSE, (float*)&projection);
        glUniformMatrix4fv(1, 1, GL_FALSE, (float*)&camera.view);

        glBindVertexArray(editor_geometry.permanent_geometry.vao);

        glDrawElementsInstancedBaseVertexBaseInstance(
            GL_LINES,
            editor_geometry.permanent_geometry.grid_num_indices,
            GL_UNSIGNED_INT,
            (const void*)editor_geometry.permanent_geometry.grid_indices_offset,
            editor_geometry.num_grids,
            editor_geometry.permanent_geometry.grid_base_vertex,
            0
        );

        // Points

        glUseProgram(program_editor_point);

        glUniformMatrix4fv(0, 1, GL_FALSE, (float*)&projection);
        glUniformMatrix4fv(1, 1, GL_FALSE, (float*)&camera.view);
        glUniform1ui(2, picked_vertex_id);

        glDrawElementsInstancedBaseVertexBaseInstance(
            GL_TRIANGLES,
            editor_geometry.permanent_geometry.point_num_indices,
            GL_UNSIGNED_INT,
            (const void*)editor_geometry.permanent_geometry.point_indices_offset,
            editor_geometry.num_points,
            editor_geometry.permanent_geometry.point_base_vertex,
            0
        );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}