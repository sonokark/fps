#ifndef GEOMETRY_HPP_
#define GEOMETRY_HPP_

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Common.hpp"

struct CVertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec3 normal;
};

struct SVertex
{
    glm::vec3  position;
    glm::vec3  normal;
    glm::vec4  color;
    glm::uvec3 cell_ids;
};

struct PVertex
{
    glm::vec3 position;
};

struct Geometry_NumVerticesAndIndices
{
    uint32_t num_vertices;
    uint32_t num_indices;
};

constexpr inline Geometry_NumVerticesAndIndices Geometry_Ball_GetNumRequiredVerticesAndIndices(uint32_t resolution);

bool32_t Geometry_Ball_Push(
    CVertex*  vertices,
    uint32_t  max_num_vertices,
    uint32_t* indices,
    uint32_t  max_num_indices,
    uint32_t  resolution,
    glm::vec4 color
);

constexpr inline Geometry_NumVerticesAndIndices Geometry_Grid_GetNumRequiredVerticesAndIndices(glm::uvec2 min, glm::uvec2 max);

bool32_t Geometry_Grid_Push(
    PVertex*   vertices,
    uint32_t   max_num_vertices,
    uint32_t*  indices,
    uint32_t   max_num_indices,
    glm::uvec2 axes,
    glm::ivec2 min,
    glm::ivec2 max
);

constexpr inline Geometry_NumVerticesAndIndices Geometry_Point_GetNumRequiredVerticesAndIndices(void);

bool32_t Geometry_Point_Push(
    PVertex*  vertices,
    uint32_t  max_num_vertices,
    uint32_t* indices,
    uint32_t  max_num_indices,
    glm::vec2 size
);

// Implementation of inline functions

constexpr inline Geometry_NumVerticesAndIndices Geometry_Ball_GetNumRequiredVerticesAndIndices(uint32_t resolution)
{
    uint32_t num_planes = resolution - 1;
    uint32_t num_middle_layers = resolution - 2;
    uint32_t num_triangles = resolution * 2 + num_middle_layers * resolution * 2;

    return { num_planes * resolution + 2, num_triangles * 3 };
}

constexpr inline Geometry_NumVerticesAndIndices Geometry_Grid_GetNumRequiredVerticesAndIndices(glm::uvec2 min, glm::uvec2 max)
{
    const uint32_t num_lines = (max.x - min.x + 1) + (max.y - min.y + 1);

    const uint32_t num_vertices = num_lines * 2 - 4;
    const uint32_t num_indices = num_lines * 2;

    return { num_vertices, num_indices };
}

constexpr inline Geometry_NumVerticesAndIndices Geometry_Point_GetNumRequiredVerticesAndIndices(void)
{
    return { 4, 6 };
}

#endif