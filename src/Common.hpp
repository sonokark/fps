#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <assert.h>
#include <stdint.h>

#define UNUSED(x) ((void)(x))

#define ARRAY_SIZE_U32(array) ((uint32_t)(sizeof(array) / sizeof(*(array))))

#define STR_(x) #x
#define STR(x) STR_(x)

#ifdef _DEBUG
#	define FPS_DEBUG_BUILD 1
#endif

#if FPS_DEBUG_BUILD
#	define ASSERT(...) assert(__VA_ARGS__)
#else
#	define ASSERT(...)
#endif

#define UNREACHABLE ASSERT(0)

typedef uint32_t bool32_t;

#define TRUE 1u
#define FALSE 0u

#endif