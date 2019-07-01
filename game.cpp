#include "game.h"
#include "game_memory.cpp"
#include "game_misc.cpp"
#include "game_steamworks.cpp"

game_state GameState = {};

void
RegisterCommand(command_cache *Cache, const char *Name, command_callback *Callback) {
	Assert(Cache);
	Assert(Name);
	Assert(Callback);
}

void
UnregisterCommand(command_cache *Cache, const char *Name) {
	Assert(Cache);
	Assert(Name);
}

void
ExecCommand(command_cache *Cache, const char *Command, ...) {
	Assert(Cache);
	Assert(Command);
}

inline void
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
		GameState.PlatformAPI->Outf("PushSetting: Out of memory!");
	}
}

b32
LoadSettingsFromFile(setting_cache *Cache, const char *FileName) {
	UnusedParam(Cache);
	UnusedParam(FileName);
	
	return false;
}

b32
SaveSettingsToFile(setting_cache *Cache, const char *FileName) {
	UnusedParam(Cache);
	UnusedParam(FileName);
	
	return false;
}

const char *
GetSetting(setting_cache *Cache, const char *Name) {
	UnusedParam(Cache);
	UnusedParam(Name);
	
	return 0;
}

inline void
OutMemoryStackStats(memory_stack *Stack) {
	Assert(Stack);
	
	const f64 Mb = (f64)(1024 * 1024);
	GameState.PlatformAPI->Outf("[%s] : eated %f Mb, used %f Mb, free %f Mb.",
								Stack->Name,
								(f64)Stack->Piece.Size / Mb,
								(f64)Stack->Mark / Mb,
								(f64)(Stack->Piece.Size - Stack->Mark) / Mb);
}

inline void
OutMemoryPoolStats(memory_pool *Pool) {
	Assert(Pool);
	
	const f64 Mb = (f64)(1024 * 1024);
	GameState.PlatformAPI->Outf("[%s] : eated %f Mb, blocksize %f Mb, alloc %d blocks, free %d blocks.",
								Pool->Name,
								(f64)((Pool->BlockSize + sizeof(memory_pool_block)) * Pool->MaxBlocks) / Mb,
								(f64)Pool->BlockSize / Mb,
								Pool->NumAllocBlocks,
								Pool->NumFreeBlocks);
}

static void
OutMemoryTableStats(void) {
	const f64 Mb = (f64)(1024 * 1024);
	
	GameState.PlatformAPI->Outf("------------------------------------------------------------------------------------");
	
	OutMemoryStackStats(&GameState.PerFrameStack);
	OutMemoryStackStats(&GameState.PermanentStack);
	
	OutMemoryPoolStats(&GameState.CommandsPool);
	OutMemoryPoolStats(&GameState.SettingsPool);
	
	GameState.PlatformAPI->Outf("------------------------------------------------------------------------------------");
	
	GameState.PlatformAPI->Outf("* Game primary storage total size: %f Mb.",
								(f64)GameState.GameMemory->StorageTotalSize / Mb);
	GameState.PlatformAPI->Outf("* Game primary stroage left space size: %f Mb",
								(f64)GameState.GameMemory->FreeStorage.Size / Mb);
	GameState.PlatformAPI->Outf("------------------------------------------------------------------------------------");
	
}

static void
SteamWarningMessageHook(int SeverityLevel, const char *Message) {
	if (SeverityLevel == 0)
		GameState.PlatformAPI->Outf("[Steam] %s", Message);
	else
		GameState.PlatformAPI->Outf("[Steam-warning] %s", Message);
}

static b32
CommandQuit(char **Params, u32 NumParams) {
	UnusedParam(Params);
	UnusedParam(NumParams);
	
	GameState.PlatformAPI->QuitRequested = true;
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
	switch (TriggerType) {
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// NOTE(ivan): Game initialization.
		////////////////////////////////////////////////////////////////////////////////////////////////////
	case GameTriggerType_Prepare: {
		// NOTE(ivan): Set APIs.
		GameState.PlatformAPI = PlatformAPI;
		GameState.SteamworksAPI = SteamworksAPI;

		// NOTE(ivan): Set platform-exchangable data.
		GameState.GameMemory = GameMemory;
		GameState.GameClocks = GameClocks;
		GameState.GameInput = GameInput;

		// NOTE(ivan): Initialize Steamworks.
		GameState.PlatformAPI->Outf("Initializing Steamworks...");
		if (GameState.SteamworksAPI->Init()) {
			GainAccessToSteamworksInterfaces(GameState.SteamworksAPI);
			
			// NOTE(ivan): If Steam is not running or the game wasn't started through Steam,
			// SteamworksAPI->RestartAppIfNecessary() starts the local Steam client and also launches the game again.
			if (!GameState.SteamworksAPI->RestartAppIfNecessary(GAME_STEAM_APPID)) {
				// NOTE(ivan): Set Steamworks hooks.
				GameState.SteamworksAPI->Client->SetWarningMessageHook(SteamWarningMessageHook);

				// NOTE(ivan): Check whether the user has logged into Steam.
				if (GameState.SteamworksAPI->User->BLoggedOn()) {
					// NOTE(ivan): Organize memory partitions.
					GameState.PlatformAPI->Outf("Partitioning game primary storage...");
					u32 FreeStoragePercent = 100;
					FreeStoragePercent = CreateMemoryStack(&GameState.PerFrameStack, "PerFrameStack",
														   Percentage(10, FreeStoragePercent));
					FreeStoragePercent = CreateMemoryStack(&GameState.PermanentStack, "PermanentStack",
														   Percentage(30, FreeStoragePercent));
					FreeStoragePercent = CreateMemoryPool(&GameState.CommandsPool, "CommandsPool",
														  sizeof(command), Percentage(10, FreeStoragePercent));
					FreeStoragePercent = CreateMemoryPool(&GameState.SettingsPool, "SettingsPool",
														  sizeof(setting), Percentage(10, FreeStoragePercent));
					OutMemoryTableStats();

					// NOTE(ivan): Register commands.
					RegisterCommand(&GameState.CommandCache, "quit", CommandQuit);
					if (IsInternal()) {
						RegisterCommand(&GameState.CommandCache, "causeav", CommandCauseAV);
					}

					// NOTE(ivan): Load settings.
					LoadSettingsFromFile(&GameState.SettingCache, "default.set");
					LoadSettingsFromFile(&GameState.SettingCache, "user.set");
				} else {
					// NOTE(ivan): User must be logged into Steam.
					GameState.PlatformAPI->Crashf(GAMENAME " requires user to be logged into Steam!");
				}
			} else {
				// NOTE(ivan): Restart the game by Steam.
				GameState.PlatformAPI->QuitRequested = true;
			}
		} else {
			// NOTE(ivan): Game cannot initialize Steamworks.
			GameState.PlatformAPI->Crashf(GAMENAME " cannot initialize Steamworks!");
		}
	} break;

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// NOTE(ivan): Game de-initialization.
		////////////////////////////////////////////////////////////////////////////////////////////////////
	case GameTriggerType_Release: {
		// NOTE(ivan): Save settings.
		SaveSettingsToFile(&GameState.SettingCache, "user.set");

		// NOTE(ivan): Shutdown Steamworks.
		GameState.SteamworksAPI->Shutdown();
	} break;

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// NOTE(ivan): Game frame update.
		////////////////////////////////////////////////////////////////////////////////////////////////////
	case GameTriggerType_Frame: {
		// NOTE(ivan): Clear per-frame stack.
		ResetMemoryStack(&GameState.PerFrameStack);
	} break;
	}
}
