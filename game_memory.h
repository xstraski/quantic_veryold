#ifndef GAME_MEMORY_H
#define GAME_MEMORY_H

#include "game_platform.h"

// NOTE(ivan): Memory management partitions/containers.
//
// Each container (stack, pool, etc...) is a limited partition of a primary storage provided in game_memory structure.
// Each container's initialization function (CreateMemoryStack, CreateMemoryPool, etc...) takes a percentage value that
// tells how much space should be eated from primary storage, and returns percentage value of how much space is left
// in there, and this percentage value should be used to initialize next partition. It is important,
// because some specialized containers (f.e. memory pool) does not allocate an exact given percentage of required memory,
// sometimes it allocates more or less, because it is almost impossible to manually calculate wanted memory space for all
// pool blocks in a form of percentage, especially when a primary storage total size is always various.
//
// All memory partitions are living in game_state structure, and all initial memory partitionnig is done in
// GameTrigger() function's initialization stage. Typical partition strategy: creating first partition with some percentage
// of wanted memory and getting left free space as a percent. After that, creating next partition with a percent of
// wanted memory calculated using previously owned percentage of left free space. And so on...
//
// None of the containers declared below can/should ever be released, freed, deallocated, whatever you call it.
// The entire primary storage gets allocated and deallocated in platform abstraction layer at program's initialization
// and deinitialization respectively. However, it is possible to *reset* the partition, which means complete partition
// cleanup, zeroing included.

inline uptr
PercentageToSize(u32 SizePercentage, uptr TotalSize) {
	Assert(SizePercentage);
	Assert(TotalSize);

	return (uptr)(((f64)TotalSize / 100.0) * SizePercentage);
}
inline u32
SizeToPercentage(uptr Size, uptr TotalSize) {
	Assert(Size);
	Assert(TotalSize);

	return (u32)(((f64)Size / (f64)TotalSize) * 100.0);
}

// NOTE(ivan): Single-sided memory stack.
struct memory_stack {
	char Name[128];

	piece Piece;
	uptr Mark;

	ticket_mutex Mutex;
};

// NOTE(ivan): CreateMemoryStack() returns a percentage of left free space of primary storage.
// It never eats more memory than the caller defined in SizePercentage.
u32 CreateMemoryStack(memory_stack *Stack, const char *Name, u32 SizePercentage);
void ResetMemoryStack(memory_stack *Stack);

void *AllocFromStack(memory_stack *Stack, uptr Size);
void PopStack(memory_stack *Stack);

// NOTE(ivan): Memory pool block. This structure lives in the beginning of allocated block space,
// before the actual data memory. Total memory size for each block is sizeof(memory_pool_block) + BlockSize.
struct memory_pool_block {
	memory_pool_block *NextBlock;
	memory_pool_block *PrevBlock;
};

// NOTE(ivan): Memory pool.
struct memory_pool {
	char Name[128];

	piece Piece;
	memory_pool_block *AllocBlocks; // NOTE(ivan): Linked list of all allocated blocks.
	memory_pool_block *FreeBlocks;  // NOTE(ivan): Linked list of all free blocks.
	u32 NumAllocBlocks;
	u32 NumFreeBlocks;

	uptr BlockSize;
	u32 MaxBlocks;

	ticket_mutex Mutex;
};

// NOTE(ivan): CreateMemoryPool() returns percentage of left free space of game primary storage.
// It might be larger that the caller expects, because memory pool allocates more space
// if a given SizePercentage converted to a wanted size of bytes is too small for all blocks
// with a given BlockSize.
u32 CreateMemoryPool(memory_pool *Pool, const char *Name, uptr BlockSize, u32 SizePercentage);
void ResetMemoryPool(memory_pool *Pool);

void * AllocFromPool(memory_pool *Pool);
void FreeFromPool(memory_pool *Pool, void *Base);

// NOTE(ivan): Memory heap block.
struct memory_heap_block {
	memory_heap_block *NextBlock;
	memory_heap_block *PrevBlock;

	uptr Size;;
	b32 IsFree;
};

// NOTE(ivan): Memory heap.
struct memory_heap {
	char Name[128];
	
	piece Piece;
	memory_heap_block *Blocks;
	u32 NumBlocks;

	ticket_mutex Mutex;
};

// NOTE(ivan): CreateMemoryHeap() returns percentage of left free space of game primary storage.
// It never eats more memory than the caller defined in SizePercentage.
u32 CreateMemoryHeap(memory_heap *Heap, const char *Name, u32 SizePercentage);
void ResetMemoryHeap(memory_heap *Heap);

void * AllocFromHeap(memory_heap *Heap, uptr Size);
void FreeFromHeap(memory_heap *Heap, void *Base);

#endif // #ifndef GAME_MEMORY_H
