cmake_minimum_required(VERSION 3.21)
project("fps")

include(FetchContent)

FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG        7b6aead9fb88b3623e3b3725ebb42670cbe4c579
)

FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG        a532f5b1cf27d6a3c099437e6959cf7e398a0a67
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glm)

set(EXECUTABLE_NAME "fps")

add_executable(
    ${EXECUTABLE_NAME}
    "src/main.cpp"
	"src/Common.hpp"
	"src/OpenGL.hpp"
	"src/OpenGL.cpp"
	"src/OpenGL_Shader.hpp"
	"src/OpenGL_Shader.cpp"
	"src/Arena.hpp"
	"src/Arena.cpp"
	"src/Geometry.hpp"
    "src/Geometry.cpp"
)

set_property(TARGET ${EXECUTABLE_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${EXECUTABLE_NAME} PROPERTY CXX_STANDARD 17)

if(MSVC)
    target_compile_options(${EXECUTABLE_NAME} PRIVATE /W4)
else()
    target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(${EXECUTABLE_NAME} PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE glfw glm::glm)
