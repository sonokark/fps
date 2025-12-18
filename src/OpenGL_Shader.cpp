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

GLuint OpenGL_Shader_CreateProgram(const GLuint* shaders, uint32_t num_shaders)
{
	GLuint program = glCreateProgram();
	ASSERT(program != 0);

	for (uint32_t i = 0; i < num_shaders; ++i)
		glAttachShader(program, shaders[i]);

	glLinkProgram(program);
	
	for (uint32_t i = 0; i < num_shaders; ++i)
		glDetachShader(program, shaders[i]);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	ASSERT(status != 0);

	return program;
}

GLuint OpenGL_Shader_CreateProgramFromSources(const char* vertex_source, const char* fragment_source)
{
	GLuint shaders[2];

	shaders[0] = OpenGL_Shader_CreateShader(GL_VERTEX_SHADER, vertex_source);
	ASSERT(shaders[0] != 0);

	shaders[1] = OpenGL_Shader_CreateShader(GL_FRAGMENT_SHADER, fragment_source);
	ASSERT(shaders[1] != 0);

	GLuint program = OpenGL_Shader_CreateProgram(shaders, 2);
	ASSERT(program != 0);

	glDeleteShader(shaders[0]);
	glDeleteShader(shaders[1]);

	return program;
}
