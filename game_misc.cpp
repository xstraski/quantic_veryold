#include "game_misc.h"

char **
TokenizeString(memory_heap *Heap, const char *String, u32 *NumTokens, const char *Delims) {
	Assert(Heap);
	Assert(String);
	Assert(NumTokens);
	Assert(Delims);

	const char *Ptr = String;
	*NumTokens = 0;

	// NOTE(ivan): Iterate to count tokens.
	b32 WasDelim = true;
	while (true) {
		if (strchr(Delims, *Ptr) || *Ptr == 0) {
			if (!WasDelim)
				(*NumTokens)++;

			WasDelim = true;
		} else {
			WasDelim = false;
		}

		if (*Ptr == 0)
			break;

		Ptr++;
	}

	if ((*NumTokens) == 0)
		return 0;

	// NOTE(ivan): Allocate needed space.
	char **Result = (char **)AllocFromHeap(Heap, sizeof(char *) * (*NumTokens));
	if (Result) {
		// NOTE(ivan): Iterate all over again to capture tokens.
		u32 It = 0;
		const char *Last = Ptr = String;

		WasDelim = true;
		while (true) {
			if (strchr(Delims, *Ptr) || *Ptr == 0) {
				if (!WasDelim) {
					u32 Diff = (u32)(Ptr - Last);

					Result[It] = (char *)AllocFromHeap(Heap, sizeof(char) * (Diff + 1));
					if (Result[It]) {
						strncpy(Result[It], Last, Diff);
						Result[It][Diff] = 0;

						Last = Ptr + 1;
						It++;
					} else {
						for (u32 Index = 0; Index < It; Index++)
							FreeFromHeap(Heap, Result[Index]);
						FreeFromHeap(Heap, Result);
						break;
					}
				}

				WasDelim = true;
			} else {
				WasDelim = false;
			}

			if (*Ptr == 0)
				break;

			Ptr++;
		}
	}

	return Result;
}

void
FreeTokenizedString(memory_heap *Heap, char **Tokens, u32 NumTokens) {
	Assert(Heap);
	Assert(Tokens);
	Assert(NumTokens);

	for (u32 Index = 0; Index < NumTokens; Index++)
		FreeFromHeap(Heap, Tokens[Index]);
	FreeFromHeap(Heap, Tokens);
}

piece
ReadEntireFile(memory_stack *Stack, const char *FileName) {
	Assert(Stack);
	Assert(FileName);

	piece Result = {};

	file_handle FileHandle = GameState.PlatformAPI->FOpen(FileName, FileAccessType_OpenForReading);
	if (FileHandle != NOTFOUND) {
		Result.Size = SafeTruncateU64(GetFileSizeByHandle(FileHandle));
		Result.Base = (u8 *)AllocFromStack(Stack, Result.Size);
		if (Result.Base) {
			if (GameState.PlatformAPI->FRead(FileHandle, Result.Base, (u32)Result.Size) == Result.Size) {
				// NOTE(ivan): Success.
			} else {
				PopStack(Stack);
				Result.Base = 0;
				Result.Size = 0;
			}
		}
		
		GameState.PlatformAPI->FClose(FileHandle);
	}
	
	return Result;
}

b32
WriteEntireFile(const char *FileName, void *Buffer, uptr Size) {
	Assert(FileName);
	Assert(Buffer);
	Assert(Size);

	b32 Result = false;

	file_handle FileHandle = GameState.PlatformAPI->FOpen(FileName, FileAccessType_OpenForWriting);
	if (FileHandle != NOTFOUND) {
		u32 Size32 = SafeTruncateU64(Size);
		if (GameState.PlatformAPI->FWrite(FileHandle, Buffer, Size32) == Size32)
			Result = true;
		
		GameState.PlatformAPI->FClose(FileHandle);
	}

	return Result;
}

uptr
GetFileSizeByName(const char *FileName) {
	Assert(FileName);
	
	uptr Result = 0;
	
	file_handle FileHandle = GameState.PlatformAPI->FOpen(FileName, FileAccessType_OpenForReading);
	if (FileHandle != NOTFOUND) {
		Result = GetFileSizeByHandle(FileHandle);

		GameState.PlatformAPI->FClose(FileHandle);
	}

	return Result;
}

uptr
GetFileSizeByHandle(file_handle FileHandle) {
	Assert(FileHandle != NOTFOUND);

	uptr Result = 0;
	uptr PrevPos = 0;

	if (GameState.PlatformAPI->FSeek(FileHandle, 0, FileSeekOrigin_Current, &PrevPos)) {
		if (GameState.PlatformAPI->FSeek(FileHandle, 0, FileSeekOrigin_End, &Result)) {
			GameState.PlatformAPI->FSeek(FileHandle, PrevPos, FileSeekOrigin_Begin, 0);
		}
	}

	return Result;
}

u32
GetLineFromFile(file_handle FileHandle, char *Buffer, u32 BufferSize) {
	Assert(FileHandle != NOTFOUND);

	char C;
	u32 Total = 0;
	while (true) {
		if (GameState.PlatformAPI->FRead(FileHandle, &C, sizeof(char))) {
			if (C == 0)
				break; // NOTE(ivan): Eof.
			if (Total == (BufferSize - 1))
				break; // NOTE(ivan): End of destination space.
			if (C == '\n')
				break; // NOTE(ivan): New line reached.

			Buffer[Total] = C;
			Total++;
		} else {
			break;
		}
	}

	if (Total) {
		// NOTE(ivan): Remove '\r' symbol if CRLF.
		if (Buffer[Total - 1] == '\r') {
			Buffer[Total] = 0;
			Total--;
		}
	}

	// NOTE(ivan): Add null terminator.
	Buffer[Total] = 0;
	
	return Total;
}
