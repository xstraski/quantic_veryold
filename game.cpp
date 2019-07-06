#include "game.h"
#include "game_memory.cpp"
#include "game_misc.cpp"

game_state GameState = {};

void
RegisterCommand(command_cache *Cache, const char *Name, command_callback *Callback) {
	Assert(Cache);
	Assert(Name);
	Assert(Callback);

	EnterTicketMutex(&Cache->Mutex);

	command *NewCommand = (command *)AllocFromPool(&GameState.CommandsPool);
	if (NewCommand) {
		strncpy(NewCommand->Name, Name, ArraySize(NewCommand->Name) - 1);
		NewCommand->Callback = Callback;

		NewCommand->NextCommand = Cache->TopCommand;
		NewCommand->PrevCommand = 0;
		if (Cache->TopCommand)
			Cache->TopCommand->PrevCommand = NewCommand;
		Cache->TopCommand = NewCommand;
		Cache->NumCommands++;
	} else {
		GameState.PlatformAPI->Outf("RegisterCommand[%s]: Out of memory!", Name);
	}

	LeaveTicketMutex(&Cache->Mutex);
}

inline command *
FindCommand(command_cache *Cache, const char *Name) {
	Assert(Cache);
	Assert(Name);
	
	command *Result = 0;
	for (Result = Cache->TopCommand; Result; Result = Result->NextCommand) {
		if (strcmp(Result->Name, Name) == 0)
			break;
	}

	return Result;
}

void
UnregisterCommand(command_cache *Cache, const char *Name) {
	Assert(Cache);
	Assert(Name);

	EnterTicketMutex(&Cache->Mutex);

	command *Command = FindCommand(Cache, Name);
	if (Command) {
		if (Command->NextCommand)
			Command->NextCommand->PrevCommand = Command->PrevCommand;
		if (Command->PrevCommand)
			Command->PrevCommand->NextCommand = Command->NextCommand;
		if (--Cache->NumCommands == 0)
			Cache->TopCommand = 0;

		FreeFromPool(&GameState.CommandsPool, Command);
	}

	LeaveTicketMutex(&Cache->Mutex);
}

void
ExecCommand(command_cache *Cache, const char *Command, ...) {
	Assert(Cache);
	Assert(Command);

	char FullCommand[1024] = {};
	CollectArgsN(FullCommand, ArraySize(FullCommand) - 1, Command);

	u32 NumTokens;
	char **Tokens = TokenizeString(&GameState.PerFrameHeap, Command, &NumTokens, " \t");
	if (Tokens) {
		command *Info = FindCommand(Cache, Tokens[0]);
		if (Info)
			Info->Callback(Tokens, NumTokens);
		
		FreeTokenizedString(&GameState.PerFrameHeap, Tokens, NumTokens);
	}
}

static void
PushSetting(setting_cache *Cache, const char *Name, const char *Value) {
	Assert(Cache);
	Assert(Name);
	Assert(Value);

	// NOTE(ivan): If already exists - change its value.
	for (setting *Setting = Cache->TopSetting; Setting; Setting = Setting->PrevSetting) {
		if (strcmp(Setting->Name, Name) == 0) {
			memset(Setting->Value, 0, sizeof(Setting->Value));
			strncpy(Setting->Value, Value, ArraySize(Setting->Value) - 1);

			return;
		}
	}

	// NOTE(ivan): Insert new setting.
	setting *NewSetting = (setting *)AllocFromPool(&GameState.SettingsPool);
	if (NewSetting) {
		strncpy(NewSetting->Name, Name, ArraySize(NewSetting->Name) - 1);
		strncpy(NewSetting->Value, Value, ArraySize(NewSetting->Value) - 1);

		NewSetting->NextSetting = 0;
		NewSetting->PrevSetting = Cache->TopSetting;
		if (Cache->TopSetting)
			Cache->TopSetting->NextSetting = NewSetting;
		Cache->TopSetting = NewSetting;
		Cache->NumSettings++;
	} else {
		GameState.PlatformAPI->Outf("PushSetting[%s][%s]: Out of memory!", Name, Value);
	}
}

b32
LoadSettingsFromFile(setting_cache *Cache, const char *FileName) {
	Assert(Cache);
	Assert(FileName);

	file_handle FileHandle = GameState.PlatformAPI->FOpen(FileName, FileAccessType_OpenForReading);
	if (FileHandle != NOTFOUND) {
		char LineBuffer[1024] = {};
		while (GetLineFromFile(FileHandle, LineBuffer, ArraySize(LineBuffer) - 1)) {
			u32 NumTokens;
			char **Tokens = TokenizeString(&GameState.PerFrameHeap, LineBuffer, &NumTokens, " \t");
			if (Tokens) {
				if (NumTokens >= 2)
					PushSetting(Cache, Tokens[0], Tokens[1]);

				FreeTokenizedString(&GameState.PerFrameHeap, Tokens, NumTokens);
			}

		}
		
		GameState.PlatformAPI->FClose(FileHandle);
	}
	
	return false;
}

b32
SaveSettingsToFile(setting_cache *Cache, const char *FileName) {
	Assert(Cache);
	Assert(FileName);

	b32 Result = false;

	EnterTicketMutex(&Cache->Mutex);

	file_handle FileHandle = GameState.PlatformAPI->FOpen(FileName, FileAccessType_OpenForWriting);
	if (FileHandle != NOTFOUND) {
		Result = true;
		
		for (setting *Setting = Cache->TopSetting; Setting; Setting = Setting->PrevSetting) {
			char String[1024] = {};
			u32 StringLength = snprintf(String, ArraySize(String) - 1, "%s %s", Setting->Name, Setting->Value);

			GameState.PlatformAPI->FWrite(FileHandle, String, StringLength);
		}

		GameState.PlatformAPI->FClose(FileHandle);
	}

	LeaveTicketMutex(&Cache->Mutex);
	
	return false;
}

const char *
GetSetting(setting_cache *Cache, const char *Name) {
	UnusedParam(Cache);
	UnusedParam(Name);

	const char *Result = 0;

	EnterTicketMutex(&Cache->Mutex);

	for (setting *Setting = Cache->TopSetting; Setting; Setting = Setting->PrevSetting) {
		if (strcmp(Setting->Name, Name) == 0) {
			Result = Setting->Value;
			break;
		}
	}

	LeaveTicketMutex(&Cache->Mutex);
	
	return 0;
}

inline void
OutMemoryStackStats(memory_stack *Stack) {
	Assert(Stack);

	EnterTicketMutex(&Stack->Mutex);
	
	const f64 Mb = (f64)(1024 * 1024);
	GameState.PlatformAPI->Outf("[%s] : eated %.3f Mb, used %.3f Mb, free %.3f Mb.",
								Stack->Name,
								(f32)(Stack->Piece.Size / Mb),
								(f32)(Stack->Mark / Mb),
								(f32)((Stack->Piece.Size - Stack->Mark) / Mb));

	LeaveTicketMutex(&Stack->Mutex);
}

inline void
OutMemoryPoolStats(memory_pool *Pool) {
	Assert(Pool);

	EnterTicketMutex(&Pool->Mutex);
	
	const f64 Mb = (f64)(1024 * 1024);
	GameState.PlatformAPI->Outf("[%s] : eated %.3f Mb, blocksize %d bytes, alloc %d blocks, free %d blocks.",
								Pool->Name,
								(f32)(((Pool->BlockSize + sizeof(memory_pool_block)) * Pool->MaxBlocks) / Mb),
								Pool->BlockSize,
								Pool->NumAllocBlocks,
								Pool->NumFreeBlocks);

	LeaveTicketMutex(&Pool->Mutex);
}

inline void
OutMemoryHeapStats(memory_heap *Heap) {
	Assert(Heap);

	EnterTicketMutex(&Heap->Mutex);

	const f64 Mb = (f64)(1024 * 1024);
	GameState.PlatformAPI->Outf("[%s] : eated %.3f Mb, used %.3f Mb, %d blocks.",
								Heap->Name,
								(f32)(Heap->Piece.Size / Mb),
								(f32)(Heap->UsedSize / Mb),
								Heap->NumBlocks);

	LeaveTicketMutex(&Heap->Mutex);
}

static void
OutMemoryTableStats(void) {
	GameState.PlatformAPI->Outf("------------------------------------------------------------------------------------");
	
	OutMemoryHeapStats(&GameState.PerFrameHeap);
	OutMemoryStackStats(&GameState.PermanentStack);
	
	OutMemoryPoolStats(&GameState.CommandsPool);
	OutMemoryPoolStats(&GameState.SettingsPool);
	
	GameState.PlatformAPI->Outf("------------------------------------------------------------------------------------");
	const f64 Mb = (f64)(1024 * 1024);
	EnterTicketMutex(&GameState.GameMemory->Mutex);
	GameState.PlatformAPI->Outf("* Game primary storage total size: %.3f Mb.",
								(f64)GameState.GameMemory->StorageTotalSize / Mb);
	GameState.PlatformAPI->Outf("* Game primary stroage left space size: %.3f Mb",
								(f64)GameState.GameMemory->FreeStorage.Size / Mb);
	LeaveTicketMutex(&GameState.GameMemory->Mutex);
	GameState.PlatformAPI->Outf("------------------------------------------------------------------------------------");
	
}

static b32
CommandQuit(char **Params, u32 NumParams) {
	if (NumParams >= 2) {
		QuitGame(atoi(Params[1]));
	} else {
		QuitGame(0);
	}

	return true;
}

static b32
CommandRestart(char **Params, u32 NumParams) {
	UnusedParam(Params);
	UnusedParam(NumParams);

	RestartGame();
	return true;
}

#if INTERNAL
static b32
CommandCauseAV(char **Params, u32 NumParams) {
	UnusedParam(Params);
	UnusedParam(NumParams);

	*((s32 *)0) = 1;
	return true; // NOTE(ivan): Never gets here.
}
#endif // #if INTERNAL

extern "C" GAME_TRIGGER(GameTrigger) {
	// NOTE(ivan): Various game file names.
	static const char GameDefaultSettingsFileName[] = "default.set";
	static const char GameUserSettingsFileName[] = "user.set";

	switch (TriggerType) {
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// NOTE(ivan): Game initialization.
		////////////////////////////////////////////////////////////////////////////////////////////////////
	case GameTriggerType_Prepare: {
		// NOTE(ivan): Set APIs.
		GameState.PlatformAPI = PlatformAPI;

		// NOTE(ivan): Set platform-exchangable data.
		GameState.GameMemory = GameMemory;
		GameState.GameClocks = GameClocks;
		GameState.GameInput = GameInput;

		// NOTE(ivan): Organize memory partitions.
		// TODO(ivan): Calibrate memory partitions sizes to make them
		// as adequate as possible.
		GameState.PlatformAPI->Outf("Partitioning game primary storage...");
		u32 FreeStoragePercent = 100;
		FreeStoragePercent = CreateMemoryHeap(&GameState.PerFrameHeap, "PerFrameHeap",
											  Percentage(10, FreeStoragePercent));
		FreeStoragePercent = CreateMemoryStack(&GameState.PermanentStack, "PermanentStack",
											   Percentage(10, FreeStoragePercent));
		FreeStoragePercent = CreateMemoryPool(&GameState.CommandsPool, "CommandsPool",
											  sizeof(command), Percentage(10, FreeStoragePercent));
		FreeStoragePercent = CreateMemoryPool(&GameState.SettingsPool, "SettingsPool",
											  sizeof(setting), Percentage(10, FreeStoragePercent));
		OutMemoryTableStats();

		// NOTE(ivan): Register base commands.
		RegisterCommand(&GameState.CommandCache, "quit", CommandQuit);
		RegisterCommand(&GameState.CommandCache, "restart", CommandRestart);
		if (IsInternal()) {
			RegisterCommand(&GameState.CommandCache, "causeav", CommandCauseAV);
		}

		// NOTE(ivan): Load settings.
		LoadSettingsFromFile(&GameState.SettingCache, GameDefaultSettingsFileName);
		LoadSettingsFromFile(&GameState.SettingCache, GameUserSettingsFileName);
	} break;

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// NOTE(ivan): Game de-initialization.
		////////////////////////////////////////////////////////////////////////////////////////////////////
	case GameTriggerType_Release: {
		// NOTE(ivan): Save settings.
		SaveSettingsToFile(&GameState.SettingCache, GameUserSettingsFileName);
	} break;

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// NOTE(ivan): Game frame update.
		////////////////////////////////////////////////////////////////////////////////////////////////////
	case GameTriggerType_Frame: {
		// NOTE(ivan): Clean up per-frame heap.
		ResetMemoryHeap(&GameState.PerFrameHeap);

#if INTERNAL		
		// NOTE(ivan): Restart if requested.
		if (GameState.GameInput->KbButtons[KeyCode_F1].IsDown)
			RestartGame();
#endif		
	} break;
	}
}
