#include "game_memory.h"

inline u8 *
EatGameMemory(uptr Size) {
	Assert(Size);

	u8 *Result = 0;

	EnterTicketMutex(&GameState.GameMemory->Mutex);

	if (Size < GameState.GameMemory->FreeStorage.Size)
		Result = ConsumeSize(&GameState.GameMemory->FreeStorage, Size);

	LeaveTicketMutex(&GameState.GameMemory->Mutex);

	return Result;
}

inline u32
GetFreeGameMemorySizePercentage(void) {
	u32 Result;

	EnterTicketMutex(&GameState.GameMemory->Mutex);
	Result = SizeToPercentage(GameState.GameMemory->FreeStorage.Size,
							  GameState.GameMemory->StorageTotalSize);
	LeaveTicketMutex(&GameState.GameMemory->Mutex);

	return Result;
}

inline uptr
CalculateGameMemorySizeByPercent(u32 SizePercentage) {
	uptr Result;
	
	EnterTicketMutex(&GameState.GameMemory->Mutex);
	Result = PercentageToSize(SizePercentage,
							  GameState.GameMemory->StorageTotalSize);
	LeaveTicketMutex(&GameState.GameMemory->Mutex);

	return Result;
}

u32
CreateMemoryStack(memory_stack *Stack, const char *Name, u32 SizePercentage) {
	Assert(Stack);
	Assert(Name);
	Assert(SizePercentage);

	u32 Result = 0;

	EnterTicketMutex(&Stack->Mutex);

	uptr Size = CalculateGameMemorySizeByPercent(SizePercentage);
	Stack->Piece.Base = EatGameMemory(Size);
	if (Stack->Piece.Base) {
		strncpy(Stack->Name, Name, ArraySize(Stack->Name) - 1);
		
		Stack->Piece.Size = Size;
		Stack->Mark = 0;
	} else {
		GameState.PlatformAPI->Crashf("CreateMemoryStack[%s]: Out of memory!", Name);
	}

	Result = GetFreeGameMemorySizePercentage();

	LeaveTicketMutex(&Stack->Mutex);

	return Result;
}

void
ResetMemoryStack(memory_stack *Stack) {
	Assert(Stack);

	EnterTicketMutex(&Stack->Mutex);

	Stack->Mark = 0;
	memset(Stack->Piece.Base, 0, Stack->Piece.Size);

	LeaveTicketMutex(&Stack->Mutex);
}

void *
AllocFromStack(memory_stack *Stack, uptr Size) {
	Assert(Stack);
	Assert(Size);

	void *Result = 0;

	EnterTicketMutex(&Stack->Mutex);
	
	uptr RealSize = Size + sizeof(uptr);
	if ((Stack->Mark + RealSize) < Stack->Piece.Size) {
		*((uptr *)Stack->Piece.Base + Stack->Mark + Size) = Size;
		Result = Stack->Piece.Base + Stack->Mark;
		Stack->Mark += RealSize;

		memset(Result, 0, Size);
	} else {
		GameState.PlatformAPI->Outf("AllocFromStack[%s]: Out of memory!", Stack->Name);
	}

	LeaveTicketMutex(&Stack->Mutex);

	return Result;
}

void
PopStack(memory_stack *Stack) {
	Assert(Stack);

	EnterTicketMutex(&Stack->Mutex);

	uptr Size = *((uptr *)(Stack->Piece.Base + Stack->Mark - sizeof(uptr)));
	uptr RealSize = Size + sizeof(uptr);

	Stack->Mark -= RealSize;

	LeaveTicketMutex(&Stack->Mutex);
}

inline memory_pool_block *
GetBlockByIndex(memory_pool *Pool, u32 Index) {
	Assert(Pool);
	return (memory_pool_block *)(Pool->Piece.Base + ((sizeof(memory_pool_block) + Pool->BlockSize) * Index));
}

inline memory_pool_block *
GetBlockByData(void *Data) {
	Assert(Data);
	return (memory_pool_block *)((u8 *)Data - sizeof(memory_pool_block));
}

inline u8 *
GetBlockData(memory_pool_block *Block) {
	Assert(Block);
	return ((u8 *)Block + sizeof(memory_pool_block));
}

u32
CreateMemoryPool(memory_pool *Pool, const char *Name, uptr BlockSize, u32 SizePercentage) {
	Assert(Pool);
	Assert(Name);
	Assert(BlockSize);
	Assert(SizePercentage);

	u32 Result = 0;

	EnterTicketMutex(&Pool->Mutex);

	uptr Size = CalculateGameMemorySizeByPercent(SizePercentage);
	uptr FullBlockSize = sizeof(memory_pool_block) + BlockSize;
	u32 BlocksInSize = (u32)(Size / FullBlockSize);
	u32 BlocksToAlloc = BlocksInSize + ((Size % FullBlockSize) ? 1 : 0);
	uptr SizeToAlloc = FullBlockSize * BlocksToAlloc;

	Pool->Piece.Base = EatGameMemory(SizeToAlloc);
	if (Pool->Piece.Base) {
		Pool->Piece.Size = SizeToAlloc;
		Pool->BlockSize = BlockSize;
		Pool->MaxBlocks = BlocksToAlloc;

		strncpy(Pool->Name, Name, ArraySize(Pool->Name) - 1);

		Pool->AllocBlocks = 0;
		Pool->FreeBlocks = (memory_pool_block *)Pool->Piece.Base;
		for (u32 Index = 0; Index < Pool->MaxBlocks; Index++) {
			memory_pool_block *NewBlock = GetBlockByIndex(Pool, Index);

			if (Index > 0)
				NewBlock->PrevBlock = GetBlockByIndex(Pool, Index - 1);
			if (Index < (Pool->MaxBlocks - 1))
				NewBlock->NextBlock = GetBlockByIndex(Pool, Index + 1);
		}

		Pool->NumAllocBlocks = 0;
		Pool->NumFreeBlocks = Pool->MaxBlocks;
	} else {
		GameState.PlatformAPI->Crashf("CreateMemoryPool[%s]: Out of memory!", Name);
	}

	Result = GetFreeGameMemorySizePercentage();
	
	LeaveTicketMutex(&Pool->Mutex);

	return Result;
}

void
ResetMemoryPool(memory_pool *Pool) {
	Assert(Pool);

	EnterTicketMutex(&Pool->Mutex);
	
	Pool->AllocBlocks = 0;
	Pool->FreeBlocks = GetBlockByIndex(Pool, 0);
	for (u32 Index = 0; Index < Pool->MaxBlocks; Index++) {
		memory_pool_block *NewBlock = GetBlockByIndex(Pool, Index);
		
		if (Index > 0)
			NewBlock->PrevBlock = GetBlockByIndex(Pool, Index - 1);
		if (Index < (Pool->MaxBlocks - 1))
			NewBlock->NextBlock = GetBlockByIndex(Pool, Index + 1);

		u8 *NewData = GetBlockData(NewBlock);
		memset(NewData, 0, Pool->BlockSize);
	}
	
	Pool->NumAllocBlocks = 0;
	Pool->NumFreeBlocks = Pool->MaxBlocks;

	LeaveTicketMutex(&Pool->Mutex);
}

void *
AllocFromPool(memory_pool *Pool) {
	Assert(Pool);

	void *Result = 0;
	
	EnterTicketMutex(&Pool->Mutex);

	memory_pool_block *TargetBlock = Pool->FreeBlocks;
	if (TargetBlock) {
		if (TargetBlock->PrevBlock)
			TargetBlock->PrevBlock->NextBlock = TargetBlock->NextBlock;
		if (TargetBlock->NextBlock)
			TargetBlock->NextBlock->PrevBlock = TargetBlock->PrevBlock;
		Pool->FreeBlocks = TargetBlock->NextBlock;
		Pool->NumFreeBlocks--;
		
		if (Pool->AllocBlocks)
			Pool->AllocBlocks->PrevBlock = TargetBlock;
		TargetBlock->NextBlock = Pool->AllocBlocks;
		TargetBlock->PrevBlock = 0;
		Pool->AllocBlocks = TargetBlock;
		Pool->NumAllocBlocks++;

		Result = GetBlockData(TargetBlock);
		memset(Result, 0, Pool->BlockSize);
	} else {
		GameState.PlatformAPI->Outf("AllocFromPool[%s]: Out of memory!", Pool->Name);
	}
	
	LeaveTicketMutex(&Pool->Mutex);

	return Result;
}

void
FreeFromPool(memory_pool *Pool, void *Base) {
	Assert(Pool);
	Assert(Base);

	EnterTicketMutex(&Pool->Mutex);

	memory_pool_block *Block = GetBlockByData(Base);

	if (Block->NextBlock)
		Block->NextBlock->PrevBlock = Block->PrevBlock;
	if (Block->PrevBlock)
		Block->PrevBlock->NextBlock = Block->NextBlock;
	Pool->NumAllocBlocks--;

	if (Pool->FreeBlocks)
		Pool->FreeBlocks->PrevBlock = Block;
	Block->NextBlock = Pool->AllocBlocks;
	Block->PrevBlock = 0;
	Pool->FreeBlocks = Block;
	Pool->NumFreeBlocks++;

	LeaveTicketMutex(&Pool->Mutex);
}

u32
CreateMemoryHeap(memory_heap *Heap, const char *Name, u32 SizePercentage) {
	Assert(Heap);
	Assert(Name);
	Assert(SizePercentage);

	u32 Result = 0;

	EnterTicketMutex(&Heap->Mutex);

	uptr Size = CalculateGameMemorySizeByPercent(SizePercentage);
	Heap->Piece.Base = EatGameMemory(Size);
	if (Heap->Piece.Base) {
		Heap->Piece.Size = Size;

		strncpy(Heap->Name, Name, ArraySize(Heap->Name) - 1);

		memory_heap_block *EntireBlock = (memory_heap_block *)Heap->Piece.Base;
		EntireBlock->Size = Heap->Piece.Size - sizeof(memory_heap_block);
		EntireBlock->IsFree = true;
		EntireBlock->NextBlock = 0;
		EntireBlock->PrevBlock = 0;

		Heap->Blocks = EntireBlock;
		Heap->NumBlocks = 0;
		Heap->UsedSize = 0;
	} else {
		GameState.PlatformAPI->Crashf("CreateMemoryHeap[%s]: Out of memory!", Name);
	}

	Result = GetFreeGameMemorySizePercentage();

	LeaveTicketMutex(&Heap->Mutex);
	
	return Result;
}

void
ResetMemoryHeap(memory_heap *Heap) {
	Assert(Heap);

	EnterTicketMutex(&Heap->Mutex);

	memory_heap_block *EntireBlock = (memory_heap_block *)Heap->Piece.Base;
	EntireBlock->Size = Heap->Piece.Size - sizeof(memory_heap_block);
	EntireBlock->IsFree = true;
	EntireBlock->NextBlock = 0;
	EntireBlock->PrevBlock = 0;
	
	Heap->Blocks = EntireBlock;
	Heap->NumBlocks = 0;
	Heap->UsedSize = 0;

	LeaveTicketMutex(&Heap->Mutex);
}

void *
AllocFromHeap(memory_heap *Heap, uptr Size) {
	Assert(Heap);
	Assert(Size);

	void *Result = 0;

	EnterTicketMutex(&Heap->Mutex);

	memory_heap_block *HostBlock = 0;
	for (HostBlock = Heap->Blocks; HostBlock; HostBlock = HostBlock->NextBlock) {
		if (HostBlock->IsFree && HostBlock->Size >= Size)
			break;
	}

	if (HostBlock) {
		uptr RealSize = Size + sizeof(memory_heap_block);

		if (HostBlock->Size == RealSize) {
			HostBlock->IsFree = false;
			Result = (void *)((u8 *)HostBlock + sizeof(memory_heap_block));
		} else {
			HostBlock->Size -= RealSize;

			memory_heap_block *NewBlock = (memory_heap_block *)((u8 *)HostBlock
																+ sizeof(memory_heap_block) + HostBlock->Size);
			NewBlock->Size = Size;
			NewBlock->IsFree = false;
			NewBlock->NextBlock = HostBlock->NextBlock;
			NewBlock->PrevBlock = HostBlock;

			if (HostBlock->NextBlock)
				HostBlock->NextBlock->PrevBlock = NewBlock;
			HostBlock->NextBlock = NewBlock;

			Heap->NumBlocks++;
			Heap->UsedSize += RealSize;
			
			Result = (void *)((u8 *)NewBlock + sizeof(memory_heap_block));
			memset(Result, 0, Size);
		}
	} else {
		GameState.PlatformAPI->Outf("AllocFromHeap[%s]: Out of memory!", Heap->Name);
	}
	
	LeaveTicketMutex(&Heap->Mutex);

	return Result;
}

void
FreeFromHeap(memory_heap *Heap, void *Base) {
	Assert(Heap);
	Assert(Base);

	EnterTicketMutex(&Heap->Mutex);

	memory_heap_block *Block = (memory_heap_block *)((u8 *)Base - sizeof(memory_heap_block));
	uptr RealSize = Block->Size + sizeof(memory_heap_block);

	memory_heap_block *HostBlock = 0;
	memory_heap_block *NeighborBlock = 0;
	if (Block->NextBlock && Block->NextBlock->IsFree) {
		HostBlock = Block;
		NeighborBlock = Block->NextBlock;
	}
	if (Block->PrevBlock && Block->PrevBlock->IsFree) {
		HostBlock = Block->PrevBlock;
		NeighborBlock = Block;
	}

	if (HostBlock && NeighborBlock) {
		HostBlock->Size += NeighborBlock->Size + sizeof(memory_heap_block);
		HostBlock->IsFree = true;
		HostBlock->NextBlock = NeighborBlock->NextBlock;
		if (NeighborBlock->NextBlock)
			NeighborBlock->NextBlock->PrevBlock = HostBlock;
	} else {
		Block->IsFree = true;
	}

	Heap->UsedSize -= RealSize;

	LeaveTicketMutex(&Heap->Mutex);
}
