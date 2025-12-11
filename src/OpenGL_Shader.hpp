#ifndef OPENGL_SHADER_HPP_
#define OPENGL_SHADER_HPP_

#include "OpenGL.hpp"

#define OPENGL_SHADER_GLSL_VERSION_STR "#version " STR(OPENGL_VERSION_MAJOR) STR(OPENGL_VERSION_MINOR) "0 core\n"

inline const char* const OpenGL_Shader_DefaultVertexSource = OPENGL_SHADER_GLSL_VERSION_STR
R"sh(

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec3 a_normal;

out vec2 v_uv;

void main()
{
	v_uv = a_uv;
	gl_Position = vec4(a_position, 1.0);
}

)sh";

inline const char* const OpenGL_Shader_DefaultFragmentSource = OPENGL_SHADER_GLSL_VERSION_STR
R"sh(

in vec2 v_uv;

out vec4 o_color;

void main()
{
	o_color = vec4(v_uv.x, v_uv.y, 0.0, 1.0);
}

)sh";

GLuint OpenGL_Shader_CreateShader(GLenum type, const char* source);

GLuint OpenGL_Shader_CreateProgram(const GLuint* shader, uint32_t num_shaders);

#endif