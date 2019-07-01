#include "game_misc.h"

char **
TokenizeString(memory_stack *Stack, const char *String, u32 *NumTokens, const char *Delims) {
	Assert(Stack);
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
	char **Result = (char **)AllocFromStack(Stack, sizeof(char *) * (*NumTokens));
	if (Result) {
		// NOTE(ivan): Iterate all over again to capture tokens.
		u32 It = 0;
		const char *Last = Ptr = String;

		WasDelim = true;
		while (true) {
			if (strchr(Delims, *Ptr) || *Ptr == 0) {
				if (!WasDelim) {
					u32 Diff = (u32)(Ptr - Last);

					Result[It] = (char *)AllocFromStack(Stack, sizeof(char) * (Diff + 1));
					if (Result[It]) {
						strncpy(Result[It], Last, Diff);
						Result[It][Diff] = 0;

						Last = Ptr + 1;
						It++;
					} else {
						for (u32 Index = 0; Index < It; Index++)
							PopStack(Stack);
						PopStack(Stack);
					}
				}

				WasDelim = true;
			} else {
				WasDelim = false;
			}

			if (*Ptr == 0)
				break;
		}
	}

	return Result;
}

void
FreeTokenizedString(memory_stack *Stack, u32 NumTokens) {
	Assert(Stack);
	Assert(NumTokens);

	for (u32 Index = 0; Index < NumTokens; Index++)
		PopStack(Stack);
	PopStack(Stack);
}

piece
ReadEntireFile(memory_stack *Stack, const char *FileName) {
	Assert(Stack);
	Assert(FileName);

	piece Result = {};

	file_handle FileHandle = GameState.PlatformAPI->FOpen(FileName, FileAccessType_OpenForReading);
	if (FileHandle != NOTFOUND) {
		Result.Size = SafeTruncateU64(GetFileSizeByHandle(&FileHandle));
		Result.Base = AllocFromStack(Stack, Result.Size);
		if (Result.Base) {
			if (GameState.PlatformAPI->FRead(FileHandle, Result.Base, Result.Size) == Result.Size) {
				// NOTE(ivan): Success.
			} else {
				PopStack(Stack);
				Result.Base = Result.Size = 0;
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
		if (GameState.PlatformAPI->FWrite(FileHandle, Buffer, Size) == Result.Size)
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
	Assert(FileHandle);

	uptr Result = 0;
	uptr PrevPos = 0;

	if (GameState.PlatformAPI->FSeek(FileHandle, 0, FileSeekOrigin_Current, &PrevPos)) {
		if (GameState.PlatformAPI->FSeek(FileHandle, 0, FileSeekOrigin_End, &Result)) {
			GameState.PlatformAPI->FSeek(FileHandle, PrevPos, FileSeekOrigin_Begin, 0);
		}
	}

	return Result;
}
