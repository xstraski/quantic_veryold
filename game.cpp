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
	return false;
}

b32
SaveSettingsToFile(setting_cache *Cache, const char *FileName) {
	return false;
}

const char *
GetSetting(setting_cache *Cache, const char *Name) {
	return 0;
}

static void
OutCPUInfo(void) {}

static void
OutRAMInfo(void) {}

static void
SteamWarningMessageHook(int SeverityLevel, const char *Text) {
	GameState.PlatformAPI->Outf("[Steam][%s] %s", SeverityLevel == 0 ? "msg" : "wrn", Text);
}

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
		if (GameState.SteamworksAPI->Init()) {
			GainAccessToSteamworksInterfaces(GameState.SteamworksAPI);
			
			// NOTE(ivan): If Steam is not running or the game wasn't started through Steam,
			// SteamworksAPI->RestartAppIfNecessary() starts the local Steam client and also launches the game again.
			if (!GameState.SteamworksAPI->RestartAppIfNecessary(GAME_STEAM_APPID)) {
				// NOTE(ivan): Set Steamworks hooks.
				GameState.SteamworksAPI->Client->SetWarningMessageHook(SteamWarnningMessageHook);

				// NOTE(ivan): Check whether the user has logged into Steam.
				if (GameState.SteamworksAPI->User->BLoggedOn()) {
					// NOTE(ivan): Organize memory partitions.
					u32 FreeStoragePercent = 100;
					FreeStoragePercent = CreateMemoryStack(&GameState.PerFrameStack, "PerFrameStack",
														   Percentage(10, FreeStoragePercent));
					FreeStoragePercent = CreateMemoryStack(&GameState.PermanentStack, "PermanentStack",
														   Percentange(30, FreeStaragePercent));
					FreeStoragePercent = CreateMemoryPool(&GameState.CommandsPool, "CommandsPool",
														  Percentage(10, FreeStoragePercent));
					FreeStoragePercent = CreateMemoryPool(&GameState.SettingsPool, "SettingsPool",
														  Percentage(10, FreeStoragePercent));

					// NOTE(ivan): Register commands.
					RegisterCommand(&GameState.CommandCache, "quit", CommandQuit);
					RegisterCommand(&GameState.CommandCache, "causeav", CommandCauseAV);
					RegisterCommand(&GameState.CommandCache, "printcpu", CommandPrintCPU);
					RegisterCommand(&GameState.CommandCache, "printram", CommandPrintRAM);

					// NOTE(ivan): Load settings.
					LoadSettingsFromFile(&GameState.SettingCache, "default.set");
					LoadSettingsFromFile(&GameState.SettingCache, "user.set");

					// NOTE(ivan): Out generic host information.
					OutCPUInfo();
					OutRAMInfo();
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
		SaveSettingsToFile(&GameState.SettinngCache, "user.set");

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
