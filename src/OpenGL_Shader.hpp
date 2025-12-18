#ifndef OPENGL_SHADER_HPP_
#define OPENGL_SHADER_HPP_

#include "OpenGL.hpp"

// TODO: Use glShaderSource for concating the shader source strings

#define OPENGL_SHADER_GLSL_VERSION_STR "#version " STR(OPENGL_VERSION_MAJOR) STR(OPENGL_VERSION_MINOR) "0 core\n"

#define OPENGL_SHADER_GLSL_EXTENSIONS_STR "#extension GL_ARB_shader_draw_parameters : require\n"

inline const char* const OpenGL_Shader_Scene_VertexSource = OPENGL_SHADER_GLSL_VERSION_STR OPENGL_SHADER_GLSL_EXTENSIONS_STR
R"sh(

layout (location = 0) in vec3  a_position;
layout (location = 1) in vec3  a_normal;
layout (location = 2) in vec4  a_color;
layout (location = 3) in uvec3 a_cell_ids;

layout (location = 0) uniform mat4 u_projection;
layout (location = 1) uniform mat4 u_view;
layout (location = 2) uniform mat4 u_model;

layout (location = 3) uniform uint u_selected_face_id;

out vec3 v_normal;
out vec4 v_color;

void main()
{
	uint vertex_id = a_cell_ids.x;
	uint face_id = a_cell_ids.z;

	vec4 pick_color = vec4(0.0, 0.7, 0.7, 1.0);

	if (u_selected_face_id == face_id)
	{
		v_color = pick_color;
	}
	else
	{
		v_color = a_color;
	}

	v_normal = (u_model * vec4(a_normal.xyz, 0.0)).xyz; // NOTE: This is technically not correct

	gl_Position = u_projection * u_view * u_model * vec4(a_position.xyz + vec3(float(gl_DrawIDARB), 0.0, 0.0), 1.0);
}

)sh";

inline const char* const OpenGL_Shader_Scene_FragmentSource = OPENGL_SHADER_GLSL_VERSION_STR OPENGL_SHADER_GLSL_EXTENSIONS_STR
R"sh(

in vec4 v_color;
in vec3 v_normal;

out vec4 o_color;

const float light_bias = 0.2;
const vec3 light_direction = vec3(0.0, 0.0, -1.0);

void main()
{
	float light_factor = max(0.0, -dot(v_normal, light_direction)) * (1 - light_bias) + light_bias;
	o_color = light_factor * v_color;
}

)sh";

#define OPENGL_SHADER_EDITOR_GEOMETRY_DATA_BUFFER_DECLARATION \
R"sh(
#define EDITOR_GEOMETRY_MAX_NUM_POINTS 128
#define EDITOR_GEOMETRY_MAX_NUM_GRIDS 8

layout(std430, binding = 0) readonly buffer GeometryData
{
    vec4 point_positions[EDITOR_GEOMETRY_MAX_NUM_POINTS];
	
	mat4 grid_transforms[EDITOR_GEOMETRY_MAX_NUM_GRIDS];
	vec4 grid_colors[EDITOR_GEOMETRY_MAX_NUM_GRIDS];
};
)sh"

inline const char* const OpenGL_Shader_Editor_Geometry_VertexSource =
	OPENGL_SHADER_GLSL_VERSION_STR OPENGL_SHADER_GLSL_EXTENSIONS_STR
	OPENGL_SHADER_EDITOR_GEOMETRY_DATA_BUFFER_DECLARATION
R"sh(

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;

layout (location = 0) uniform mat4 u_projection;
layout (location = 1) uniform mat4 u_view;

out vec4 v_color;

void main()
{
	mat4 model = grid_transforms[gl_InstanceID];

	v_color = grid_colors[gl_InstanceID];
	gl_Position = u_projection * u_view * model * vec4(a_position, 1.0);
}

)sh";

inline const char* const OpenGL_Shader_Editor_Geometry_FragmentSource = OPENGL_SHADER_GLSL_VERSION_STR OPENGL_SHADER_GLSL_EXTENSIONS_STR
R"sh(

in vec4 v_color;

out vec4 o_color;

void main()
{
	o_color = v_color;
}

)sh";

inline const char* const OpenGL_Shader_Editor_Point_VertexSource = 
	OPENGL_SHADER_GLSL_VERSION_STR OPENGL_SHADER_GLSL_EXTENSIONS_STR
	OPENGL_SHADER_EDITOR_GEOMETRY_DATA_BUFFER_DECLARATION
R"sh(

layout (location = 0) in vec3 a_offset;

layout (location = 0) uniform mat4 u_projection;
layout (location = 1) uniform mat4 u_view;

out vec4 v_color;

void main()
{
	vec4 base_position = point_positions[gl_InstanceID];
	vec4 translated_position = u_view * base_position;
	vec4 position = translated_position + vec4(a_offset, 0.0);

	v_color = vec4(0.0, 1.0, 1.0, 1.0);
	gl_Position = u_projection * position;
}

)sh";

inline const char* const OpenGL_Shader_Editor_Point_FragmentSource = OpenGL_Shader_Editor_Geometry_FragmentSource;

GLuint OpenGL_Shader_CreateShader(GLenum type, const char* source);

GLuint OpenGL_Shader_CreateProgram(const GLuint* shaders, uint32_t num_shaders);

GLuint OpenGL_Shader_CreateProgramFromSources(const char* vertex_source, const char* fragment_source);

#endif