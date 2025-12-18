#ifndef SCENE_HPP_
#define SCENE_HPP_

#include "Common.hpp"
#include "Geometry.hpp"

#include <glm/glm.hpp>

#define SCENE_MAX_NUM_VERTICES 512
#define SCENE_MAX_NUM_HALF_EDGES 512
#define SCENE_MAX_NUM_FACES 512

#define SCENE_VERTEX_MAX_NUM_OUTGOING_HALF_EDGES 16
#define SCENE_VERTEX_MAX_NUM_INGOING_HALF_EDGES 16

#define SCENE_ID_NONE ((uint32_t)-1)

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

Scene_Vertex* Scene_AddVertex(Scene* scene, glm::vec3 position);

Scene_Face* Scene_ConstructFace(Scene* scene, Scene_Vertex** vertices, uint32_t num_vertices, glm::vec4 color);

bool32_t Scene_GenerateGeometry(
    const Scene* scene,
    SVertex*     vertices,
    uint32_t     max_num_vertices,
    uint32_t*    indices,
    uint32_t     max_num_indices,
    uint32_t*    out_num_vertices,
    uint32_t*    out_num_indices
);

// NOTE: ray_direction must be a unit vector
bool32_t Scene_RayCast_FindNearestIntersectingFace(
    Scene*       scene,
    glm::vec3    ray_origin,
    glm::vec3    ray_direction,
    float        ray_min_length,
    float        ray_max_length,
    Scene_Face** out_intersecting_face,
    glm::vec3*   out_intersection
);

// NOTE: ray_direction must be a unit vector
Scene_Vertex* Scene_RayCast_FindNearestVertex(
    Scene*    scene,
    glm::vec3 ray_origin,
    glm::vec3 ray_direction,
    float     max_distance
);

#endif // !SCENE_HPP_
