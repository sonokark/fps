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

constexpr inline uint32_t Geometry_Ball_GetNumRequiredVertices(uint32_t resolution);

constexpr inline uint32_t Geometry_Ball_GetNumRequiredIndices(uint32_t resolution);

bool32_t Geometry_Ball_GenerateColoredTriangularGeometry(
    CVertex*         vertices,
    uint32_t         max_num_vertices,
    uint32_t*        indices,
    uint32_t         max_num_indices,
    uint32_t         resolution,
    const glm::vec4& color
);

// Implementation of inline functions

constexpr inline uint32_t Geometry_Ball_GetNumRequiredVertices(uint32_t resolution)
{
    uint32_t num_planes = resolution - 1;

    return num_planes * resolution + 2;
}

constexpr inline uint32_t Geometry_Ball_GetNumRequiredIndices(uint32_t resolution)
{
    uint32_t num_middle_layers = resolution - 2;
    uint32_t num_triangles = resolution * 2 + num_middle_layers * resolution * 2;

    return num_triangles * 3;
}

#endif