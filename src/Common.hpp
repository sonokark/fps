#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <assert.h>
#include <stdint.h>

#ifdef _DEBUG
#	define FPS_DEBUG_BUILD 1
#endif

#if FPS_DEBUG_BUILD
#	define ASSERT assert
#else
#	define ASSERT
#endif

typedef uint32_t bool32_t;

#define TRUE 1u
#define FALSE 0u

#endif