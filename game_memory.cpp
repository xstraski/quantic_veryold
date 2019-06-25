#include "game_memory.h"

inline u8 *
EatGameMemory(uptr Size) {
	Assert(Size);

	EnterTicketMutex(&GameState.GameMemory->Mutex);

	u8 *Result = 0;
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
		// TODO(ivan): Diagnostics.
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
	
	uptr RealSize = Size = sizeof(uptr);
	if ((Stack->Mark + RealSize) < Stack->Piece.Size) {
		*((uptr *)Stack->Piece.Base + Stack->Mark + Size) = Size;
		Result = Stack->Piece.Base + Stack->Mark;
		Stack->Mark += RealSize;

		memset(Result, 0, Size);
	}

	LeaveTicketMutex(&Stack->Mutex);

	return Result;
}
