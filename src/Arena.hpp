#ifndef ARENA_HPP_
#define ARENA_HPP_

#include "Common.hpp"

struct Arena
{
	uint64_t capacity;
	uint64_t offset;

	void* memory;
};

void Arena_CreateFromUserMemory(Arena* arena, void* memory, uint64_t capacity);

bool32_t Arena_CanAllocateRegion(Arena* arena, uint64_t size, uint64_t alignment);

void* Arena_AllocateRegion(Arena* arena, uint64_t size, uint64_t alignment);

void* Arena_PushRegion(Arena* arena, void* data, uint64_t size, uint64_t alignment);

#endif