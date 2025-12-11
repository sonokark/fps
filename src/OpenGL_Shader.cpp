#include "OpenGL_Shader.hpp"

GLuint OpenGL_Shader_CreateShader(GLenum type, const char* source)
{
	GLuint shader = glCreateShader(type);
	ASSERT(shader != 0);

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	ASSERT(status != 0);

	return shader;
}

GLuint OpenGL_Shader_CreateProgram(const GLuint* shader, uint32_t num_shaders)
{
	GLuint program = glCreateProgram();
	ASSERT(program != 0);

	for (uint32_t i = 0; i < num_shaders; ++i)
		glAttachShader(program, shader[i]);

	glLinkProgram(program);
	
	for (uint32_t i = 0; i < num_shaders; ++i)
		glDetachShader(program, shader[i]);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	ASSERT(status != 0);

	return program;
}
