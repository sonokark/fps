#include "Scene.hpp"

#include <glm/gtc/matrix_transform.hpp>

Scene_Vertex* Scene_AddVertex(Scene* scene, glm::vec3 position)
{
    if (scene->num_vertices >= SCENE_MAX_NUM_VERTICES)
        return NULL;

    uint32_t vertex_index = scene->num_vertices;

    Scene_Vertex* vertex = scene->vertices + vertex_index;
    vertex->id                      = vertex_index;
    vertex->position                = position;
    vertex->num_outgoing_half_edges = 0;
    vertex->num_ingoing_half_edges  = 0;

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
    Scene_Vertex* next_vertex  = vertices[1];
    Scene_Vertex* prev_vertex  = vertices[num_vertices - 1];

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
    SVertex*     vertices,
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

            SVertex* geometry_vertex = vertices + vertex_index;
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
