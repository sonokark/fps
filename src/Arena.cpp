#include "Arena.hpp"

#include <string.h>

void Arena_CreateFromUserMemory(Arena* arena, void* memory, uint64_t capacity)
{
	arena->capacity = capacity;
	arena->offset = 0;
	arena->memory = memory;
}

static bool32_t Arena_IsPowerOfTwo(uint64_t x)
{
	return (x & (x - 1)) == 0;
}

static uint64_t Arena_AlignForward(uint64_t address, uint64_t alignment)
{
	ASSERT(Arena_IsPowerOfTwo(alignment));

	uint64_t mod = address & (alignment - 1);

	return (mod != 0) ? (address + alignment - mod) : address;
}

static void Arena_CalculateAlignedStartAndNewOffset(
	Arena*    arena,
	uint64_t  size,
	uint64_t  alignment,
	uint64_t& out_aligned_start,
	uint64_t& out_new_offset)
{
	uint64_t start = (uint64_t)arena->memory + arena->offset;
	uint64_t aligned_start = Arena_AlignForward(start, alignment);

	uint64_t end = aligned_start + size;
	uint64_t total_size = end - start;

	out_aligned_start = aligned_start;
	out_new_offset = arena->offset + total_size;
}

bool32_t Arena_CanAllocateRegion(Arena* arena, uint64_t size, uint64_t alignment)
{
	uint64_t aligned_start, new_offset;
	Arena_CalculateAlignedStartAndNewOffset(arena, size, alignment, aligned_start, new_offset);

	return new_offset <= arena->capacity;
}

void* Arena_AllocateRegion(Arena* arena, uint64_t size, uint64_t alignment)
{
	uint64_t aligned_start, new_offset;
	Arena_CalculateAlignedStartAndNewOffset(arena, size, alignment, aligned_start, new_offset);

	if (new_offset > arena->capacity) return NULL;

	arena->offset = new_offset;
	return (void*)aligned_start;
}

void* Arena_PushRegion(Arena* arena, void* data, uint64_t size, uint64_t alignment)
{
	void* region = Arena_AllocateRegion(arena, size, alignment);
	if (!region) return NULL;

	memcpy(region, data, size);
	return region;
}
