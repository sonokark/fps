#include "Geometry.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

bool32_t Geometry_Ball_GenerateColoredTriangularGeometry(
    CVertex*         vertices,
    uint32_t         max_num_vertices,
    uint32_t*        indices,
    uint32_t         max_num_indices,
    uint32_t         resolution,
    const glm::vec4& color
)
{
    ASSERT(max_num_vertices >= Geometry_Ball_GetNumRequiredVertices(resolution));
    ASSERT(max_num_indices >= Geometry_Ball_GetNumRequiredIndices(resolution));

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

    return TRUE;
}