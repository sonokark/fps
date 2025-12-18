#include "Geometry.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

bool32_t Geometry_Ball_GenerateColoredTriangularGeometry(
    CVertex*  vertices,
    uint32_t  max_num_vertices,
    uint32_t* indices,
    uint32_t  max_num_indices,
    uint32_t  resolution,
    glm::vec4 color
)
{
    Geometry_NumVerticesAndIndices nvi = Geometry_Ball_GetNumRequiredVerticesAndIndices(resolution);

    ASSERT(max_num_vertices >= nvi.num_vertices);
    ASSERT(max_num_indices >= nvi.num_indices);

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

bool32_t Geometry_Grid_Push(
    PVertex*   vertices,
    uint32_t   max_num_vertices,
    uint32_t*  indices,
    uint32_t   max_num_indices,
    glm::uvec2 axes,
    glm::ivec2 min,
    glm::ivec2 max
)
{
    const float const_axis_level = 0.0f;

    Geometry_NumVerticesAndIndices nvi = Geometry_Grid_GetNumRequiredVerticesAndIndices(min, max);

    ASSERT(nvi.num_vertices <= max_num_vertices);
    ASSERT(nvi.num_indices <= max_num_indices);

    uint32_t const_axis = 2;
    if (axes.x != 0 && axes.y != 0) const_axis = 0;
    else if (axes.x != 1 && axes.y != 1) const_axis = 1;

    PVertex* current_vertex = vertices;
    uint32_t* current_index = indices;

    uint32_t current_vertex_index = 0;

    for (uint32_t primary_axis_index = 0; primary_axis_index < 2; ++primary_axis_index)
    {
        uint32_t secondary_axis_index = 1 - primary_axis_index;

        uint32_t primary_axis = axes[primary_axis_index];
        uint32_t secondary_axis = axes[secondary_axis_index];

        for (int32_t p = min[primary_axis_index] + 1; p < max[primary_axis_index]; ++p)
        {
            current_vertex->position[primary_axis]   = (float)p;
            current_vertex->position[secondary_axis] = (float)min[secondary_axis_index];
            current_vertex->position[const_axis]     = const_axis_level;

            ++current_vertex;

            current_vertex->position[primary_axis]   = (float)p;
            current_vertex->position[secondary_axis] = (float)max[secondary_axis_index];
            current_vertex->position[const_axis]     = const_axis_level;

            ++current_vertex;

            *(current_index++) = current_vertex_index++;
            *(current_index++) = current_vertex_index++;
        }
    }

    // Add the corners

    current_vertex->position[axes.x]     = (float)min.x;
    current_vertex->position[axes.y]     = (float)min.y;
    current_vertex->position[const_axis] = const_axis_level;

    ++current_vertex;

    current_vertex->position[axes.x]     = (float)max.x;
    current_vertex->position[axes.y]     = (float)min.y;
    current_vertex->position[const_axis] = const_axis_level;

    ++current_vertex;

    current_vertex->position[axes.x]     = (float)min.x;
    current_vertex->position[axes.y]     = (float)max.y;
    current_vertex->position[const_axis] = const_axis_level;

    ++current_vertex;

    current_vertex->position[axes.x]     = (float)max.x;
    current_vertex->position[axes.y]     = (float)max.y;
    current_vertex->position[const_axis] = const_axis_level;

    ++current_vertex;

    *(current_index++) = current_vertex_index;
    *(current_index++) = current_vertex_index + 1;
    *(current_index++) = current_vertex_index;
    *(current_index++) = current_vertex_index + 2;
    *(current_index++) = current_vertex_index + 3;
    *(current_index++) = current_vertex_index + 1;
    *(current_index++) = current_vertex_index + 3;
    *(current_index++) = current_vertex_index + 2;

    return TRUE;
}

bool32_t Geometry_Point_Push(
    PVertex*  vertices,
    uint32_t  max_num_vertices,
    uint32_t* indices,
    uint32_t  max_num_indices,
    glm::vec2 size
)
{
    Geometry_NumVerticesAndIndices nvi = Geometry_Point_GetNumRequiredVerticesAndIndices();

    ASSERT(nvi.num_vertices <= max_num_vertices);
    ASSERT(nvi.num_indices <= max_num_indices);

    const float right = 0.5f * size.x;
    const float left = -right;
    const float top = 0.5f * size.y;
    const float bottom = -top;

    PVertex* current_vertex = vertices;

    *(current_vertex++) = { { left,  top, 0.0f } };
    *(current_vertex++) = { { right, top, 0.0f } };
    *(current_vertex++) = { { left,  bottom, 0.0f } };
    *(current_vertex++) = { { right, bottom, 0.0f } };

    uint32_t* current_index = indices;

    *(current_index++) = 0;
    *(current_index++) = 2;
    *(current_index++) = 3;

    *(current_index++) = 0;
    *(current_index++) = 3;
    *(current_index++) = 1;

    return TRUE;
}
