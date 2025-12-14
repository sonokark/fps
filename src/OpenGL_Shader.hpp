#ifndef OPENGL_SHADER_HPP_
#define OPENGL_SHADER_HPP_

#include "OpenGL.hpp"

#define OPENGL_SHADER_GLSL_VERSION_STR "#version " STR(OPENGL_VERSION_MAJOR) STR(OPENGL_VERSION_MINOR) "0 core\n"

#define OPENGL_SHADER_GLSL_EXTENSIONS_STR "#extension GL_ARB_shader_draw_parameters : require\n"

inline const char* const OpenGL_Shader_DefaultVertexSource = OPENGL_SHADER_GLSL_VERSION_STR OPENGL_SHADER_GLSL_EXTENSIONS_STR
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

inline const char* const OpenGL_Shader_DefaultFragmentSource = OPENGL_SHADER_GLSL_VERSION_STR OPENGL_SHADER_GLSL_EXTENSIONS_STR
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

GLuint OpenGL_Shader_CreateShader(GLenum type, const char* source);

GLuint OpenGL_Shader_CreateProgram(const GLuint* shaders, uint32_t num_shaders);

#endif