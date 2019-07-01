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

u32
CreateMemoryStack(memory_stack *Stack, const char *Name, u32 SizePercentage) {
	Assert(Stack);
	Assert(Name);
	Assert(SizePercentage);

	u32 Result = 0;

	EnterTicketMutex(&Stack->Mutex);

	uptr Size = PercentageToSize(SizePercentage, GameState.GameMemory->StorageTotalSize);
	Stack->Piece.Base = EatGameMemory(Size);
	if (Stack->Piece.Base) {
		strncpy(Stack->Name, Name, ArraySize(Stack->Name) - 1);
		
		Stack->Piece.Size = Size;
		Stack->Mark = 0;

		Result = SizeToPercentage(GameState.GameMemory->FreeStorage.Size,
								  GameState.GameMemory->StorageTotalSize);
	} else {
		GameState.PlatformAPI->Crashf("CreateMemoryStack[%s]: Out of memory!", Name);
	}

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
	return (memory_pool_block *)(Pool->Base + ((sizeof(memory_pool_block) + Pool->BlockSize) * Index));
}

inline u8 *
GetBlockData(memory_pool_block *Block) {
	Assert(Block);
	return (u8 *)(Block + sizeof(memory_pool_block));
}

inline memory_pool_block *
GetDataBlock(void *Data) {
	Assert(Data);
	return (memory_pool_block *)((u8 *)Data - sizeof(memory_pool_block));
}

u32
CreateMemoryPool(memory_pool *Pool, const char *Name, uptr BlockSize, u32 SizePercentage) {
	Assert(Pool);
	Assert(Name);
	Assert(BlockSize);
	Assert(SizePercentage);

	u32 Result = 0;

	EnterTicketMutex(&Pool->Mutex);

	uptr Size = PercentageToSize(SizePercentage, GameState.GameMemory->StorageTotalSize);
	u32 BlocksInSize = (u32)(Size / BlockSize); uptr SizeWasted = Size % BlockSize;
	u32 BlocksToAlloc = BlocksInSize + (SizeWasted ? 1 : 0);
	uptr SizeToAlloc = (sizeof(memory_pool_block) + BlockSize) * BlocksToAlloc;

	Pool->Base = EatGameMemory(SizeToAlloc);
	if (Pool->Base) {
		Pool->Size = SizeToAlloc;
		Pool->BlockSize = BlockSize;
		Pool->MaxBlocks = BlocksToAlloc;

		strncpy(Pool->Name, Name, ArraySize(Pool->Name) - 1);

		Pool->AllocBlocks = 0;
		Pool->FreeBlocks = GetBlockByIndex(Pool, 0);
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

	LeaveTicketMutex(&Pool->Mutex);

	return Result;
}

void
ResetMemoryPool(memory_pool *Pool) {
	Assert(Pool);

	EnterTicketMutex(&Pool->Mutex);
	
	for (u32 Index = 0; Index < Pool->MaxBlocks; Index++) {
		memory_pool_block *Block = GetBlockByIndex(Pool, Index);
		u8 *BlockData = GetBlockData(Block);

		memset(BlockData, 0, Pool->BlockSize);
	}

	LeaveTicketMutex(&Pool->Mutex);
}

void *
AllocFromPool(memory_pool *Pool) {
	Assert(Pool);

	void *Result = 0;
	
	EnterTicketMutex(&Pool->Mutex);

	memory_pool_block *TargetBlock = Pool->FreeBlocks;
	if (TargetBlock) {
		Result = GetBlockData(TargetBlock);

		if (TargetBlock->PrevBlock)
			TargetBlock->PrevBlock = TargetBlock->NextBlock;
		if (TargetBlock->NextBlock)
			TargetBlock->NextBlock = TargetBlock->PrevBlock;
		if (--Pool->NumFreeBlocks == 0)
			Pool->FreeBlocks = 0;

		Pool->AllocBlocks->PrevBlock = TargetBlock;
		TargetBlock->NextBlock = Pool->AllocBlocks;
		TargetBlock->PrevBlock = 0;
		Pool->AllocBlocks = TargetBlock;
		Pool->NumAllocBlocks++;

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

	memory_pool_block *Block = GetDataBlock(Base);

	if (Block->NextBlock)
		Block->NextBlock->PrevBlock = Block->NextBlock;
	if (Block->PrevBlock)
		Block->PrevBlock->NextBlock = Block->PrevBlock;
	if (--Pool->NumAllocBlocks == 0)
		Pool->AllocBlocks = 0;

	Pool->FreeBlocks->PrevBlock = Block;
	Block->NextBlock = Pool->AllocBlocks;
	Block->PrevBlock = 0;
	Pool->FreeBlocks = Block;
	Pool->NumFreeBlocks++;

	LeaveTicketMutex(&Pool->Mutex);
}
