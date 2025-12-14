#ifndef OPENGL_HPP_
#define OPENGL_HPP_

#include "Common.hpp"

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 5

typedef void(*OpenGL_PFN_Proc)(void);
typedef OpenGL_PFN_Proc(*OpenGL_PFN_GetProcAddress)(const char* name);

#if defined(_WIN32) && !defined(APIENTRY)
#   define APIENTRY __stdcall
#endif

#ifndef APIENTRY
#   define APIENTRY
#endif
#ifndef APIENTRYP
#   define APIENTRYP APIENTRY *
#endif

#if defined(_WIN64)
typedef long long GLsizeiptr;
typedef long long GLintptr;
#else
typedef long GLsizeiptr;
typedef long GLintptr;
#endif

typedef char GLchar;
typedef unsigned char GLboolean;
typedef uint8_t GLubyte;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef unsigned int GLbitfield;

typedef void (APIENTRYP GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_DONT_CARE 0x1100
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_NUM_EXTENSIONS 0x821D
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_DEBUG_SOURCE_API 0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY 0x8249
#define GL_DEBUG_SOURCE_APPLICATION 0x824A
#define GL_DEBUG_SOURCE_OTHER 0x824B
#define GL_DEBUG_TYPE_ERROR 0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#define GL_DEBUG_TYPE_PORTABILITY 0x824F
#define GL_DEBUG_TYPE_PERFORMANCE 0x8250
#define GL_DEBUG_TYPE_MARKER 0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP 0x8269
#define GL_DEBUG_TYPE_POP_GROUP 0x826A
#define GL_DEBUG_TYPE_OTHER 0x8251
#define GL_DEBUG_SEVERITY_HIGH 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#define GL_DEBUG_SEVERITY_LOW 0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B
#define GL_CONTEXT_FLAGS 0x821E
#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#define GL_DEBUG_OUTPUT 0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#define GL_DEPTH_TEST 0x0B71
#define GL_CULL_FACE 0x0B44
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_MAP_WRITE_BIT 0x0002
#define GL_MAP_PERSISTENT_BIT 0x0040
#define GL_MAP_FLUSH_EXPLICIT_BIT 0x0010
#define GL_STATIC_DRAW 0x88E4
#define GL_WRITE_ONLY 0x88B9
#define GL_TRIANGLES 0x0004
#define GL_FRONT_AND_BACK 0x0408
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02
#define GL_DRAW_INDIRECT_BUFFER 0x8F3F
#define GL_INT 0x1404

typedef const GLubyte* (APIENTRYP PFN_glGetString)(GLenum name);
typedef const GLubyte* (APIENTRYP PFN_glGetStringi)(GLenum name, GLuint index);
typedef void (APIENTRYP PFN_glGetIntegerv)(GLenum pname, GLint* data);
typedef void (APIENTRYP PFN_glEnable)(GLenum cap);
typedef void (APIENTRYP PFN_glDebugMessageCallback)(GLDEBUGPROC callback, const void* userParam);
typedef void (APIENTRYP PFN_glDebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
typedef GLuint (APIENTRYP PFN_glCreateShader)(GLenum type);
typedef void (APIENTRYP PFN_glShaderSource)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
typedef void (APIENTRYP PFN_glCompileShader)(GLuint shader);
typedef void (APIENTRYP PFN_glGetShaderiv)(GLuint shader, GLenum pname, GLint* params);
typedef void (APIENTRYP PFN_glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (APIENTRYP PFN_glDeleteShader)(GLuint shader);
typedef GLuint (APIENTRYP PFN_glCreateProgram)(void);
typedef void (APIENTRYP PFN_glAttachShader)(GLuint program, GLuint shader);
typedef void (APIENTRYP PFN_glLinkProgram)(GLuint program);
typedef void (APIENTRYP PFN_glDetachShader)(GLuint program, GLuint shader);
typedef void (APIENTRYP PFN_glGetProgramiv)(GLuint program, GLenum pname, GLint* params);
typedef void (APIENTRYP PFN_glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (APIENTRYP PFN_glDeleteProgram)(GLuint program);
typedef void (APIENTRYP PFN_glUseProgram)(GLuint program);
typedef void (APIENTRYP PFN_glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRYP PFN_glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (APIENTRYP PFN_glClear)(GLbitfield mask);
typedef void (APIENTRYP PFN_glGenVertexArrays)(GLsizei n, GLuint* arrays);
typedef void (APIENTRYP PFN_glBindVertexArray)(GLuint array);
typedef void (APIENTRYP PFN_glEnableVertexAttribArray)(GLuint index);
typedef void (APIENTRYP PFN_glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void (APIENTRYP PFN_glGenBuffers)(GLsizei n, GLuint* buffers);
typedef void (APIENTRYP PFN_glBindBuffer)(GLenum target, GLuint buffer);
typedef void (APIENTRYP PFN_glBufferStorage)(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags);
typedef void* (APIENTRYP PFN_glMapBuffer)(GLenum target, GLenum access);
typedef GLboolean (APIENTRYP PFN_glUnmapBuffer)(GLenum target);
typedef void (APIENTRYP PFN_glBufferData)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void* (APIENTRYP PFN_glMapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (APIENTRYP PFN_glFlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length);
typedef void (APIENTRYP PFN_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void* indices);
typedef void (APIENTRYP PFN_glPolygonMode)(GLenum face, GLenum mode);
typedef void (APIENTRYP PFN_glCreateVertexArrays)(GLsizei n, GLuint* arrays);
typedef void (APIENTRYP PFN_glVertexArrayVertexBuffer)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef void (APIENTRYP PFN_glVertexArrayElementBuffer)(GLuint vaobj, GLuint buffer);
typedef void (APIENTRYP PFN_glVertexArrayAttribFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void (APIENTRYP PFN_glEnableVertexArrayAttrib)(GLuint vaobj, GLuint index);
typedef void (APIENTRYP PFN_glVertexArrayAttribBinding)(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
typedef void (APIENTRYP PFN_glCreateBuffers)(GLsizei n, GLuint* buffers);
typedef void (APIENTRYP PFN_glNamedBufferStorage)(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags);
typedef void* (APIENTRYP PFN_glMapNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (APIENTRYP PFN_glFlushMappedNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizeiptr length);
typedef void (APIENTRYP PFN_glMultiDrawElementsIndirect)(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride);
typedef void* (APIENTRYP PFN_glMapNamedBuffer)(GLuint buffer, GLenum access);
typedef GLboolean(APIENTRYP PFN_glUnmapNamedBuffer)(GLuint buffer);
typedef void (APIENTRYP PFN_glVertexArrayAttribIFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void (APIENTRYP PFN_glUniform1ui)(GLint location, GLuint v0);
typedef GLint(APIENTRYP PFN_glGetAttribLocation)(GLuint program, const GLchar* name);
typedef void (APIENTRYP PFN_glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);

extern PFN_glGetString glGetString;
extern PFN_glGetStringi glGetStringi;
extern PFN_glGetIntegerv glGetIntegerv;
extern PFN_glEnable glEnable;
extern PFN_glDebugMessageCallback glDebugMessageCallback;
extern PFN_glDebugMessageControl glDebugMessageControl;
extern PFN_glCreateShader glCreateShader;
extern PFN_glShaderSource glShaderSource;
extern PFN_glCompileShader glCompileShader;
extern PFN_glGetShaderiv glGetShaderiv;
extern PFN_glGetShaderInfoLog glGetShaderInfoLog;
extern PFN_glDeleteShader glDeleteShader;
extern PFN_glCreateProgram glCreateProgram;
extern PFN_glAttachShader glAttachShader;
extern PFN_glLinkProgram glLinkProgram;
extern PFN_glDetachShader glDetachShader;
extern PFN_glGetProgramiv glGetProgramiv;
extern PFN_glGetProgramInfoLog glGetProgramInfoLog;
extern PFN_glDeleteProgram glDeleteProgram;
extern PFN_glUseProgram glUseProgram;
extern PFN_glUniformMatrix4fv glUniformMatrix4fv;
extern PFN_glClearColor glClearColor;
extern PFN_glClear glClear;
extern PFN_glGenVertexArrays glGenVertexArrays;
extern PFN_glBindVertexArray glBindVertexArray;
extern PFN_glEnableVertexAttribArray glEnableVertexAttribArray;
extern PFN_glVertexAttribPointer glVertexAttribPointer;
extern PFN_glGenBuffers glGenBuffers;
extern PFN_glBindBuffer glBindBuffer;
extern PFN_glBufferStorage glBufferStorage;
extern PFN_glMapBuffer glMapBuffer;
extern PFN_glUnmapBuffer glUnmapBuffer;
extern PFN_glBufferData glBufferData;
extern PFN_glMapBufferRange glMapBufferRange;
extern PFN_glFlushMappedBufferRange glFlushMappedBufferRange;
extern PFN_glDrawElements glDrawElements;
extern PFN_glPolygonMode glPolygonMode;
extern PFN_glCreateVertexArrays glCreateVertexArrays;
extern PFN_glVertexArrayVertexBuffer glVertexArrayVertexBuffer;
extern PFN_glVertexArrayElementBuffer glVertexArrayElementBuffer;
extern PFN_glVertexArrayAttribFormat glVertexArrayAttribFormat;
extern PFN_glEnableVertexArrayAttrib glEnableVertexArrayAttrib;
extern PFN_glVertexArrayAttribBinding glVertexArrayAttribBinding;
extern PFN_glCreateBuffers glCreateBuffers;
extern PFN_glNamedBufferStorage glNamedBufferStorage;
extern PFN_glMapNamedBufferRange glMapNamedBufferRange;
extern PFN_glFlushMappedNamedBufferRange glFlushMappedNamedBufferRange;
extern PFN_glMultiDrawElementsIndirect glMultiDrawElementsIndirect;
extern PFN_glMapNamedBuffer glMapNamedBuffer;
extern PFN_glUnmapNamedBuffer glUnmapNamedBuffer;
extern PFN_glVertexArrayAttribIFormat glVertexArrayAttribIFormat;
extern PFN_glUniform1ui glUniform1ui;
extern PFN_glGetAttribLocation glGetAttribLocation;
extern PFN_glVertexAttribIPointer glVertexAttribIPointer;

bool32_t OpenGL_LoadFunctions(OpenGL_PFN_GetProcAddress get_opengl_proc_address);

bool32_t OpenGL_IsExtensionAvailable(const char* extension_name);

bool32_t OpenGL_GetAvailableExtensions(const char**& out_availabe_extensions, uint32_t& out_num_extensions);

const char* OpenGL_Debug_GetSourceString(GLenum source);

const char* OpenGL_Debug_GetTypeString(GLenum type);

const char* OpenGL_Debug_GetSeverityString(GLenum severity);

void APIENTRY OpenGL_Debug_WriteOutputToStderr(
    GLenum       source,
    GLenum       type,
    unsigned int id,
    GLenum       severity,
    GLsizei      length,
    const char*  message,
    const void*  user_param
);

#endif