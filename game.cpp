#include "game.h"
#include "game_misc.cpp"
#include "game_memory.cpp"

game_state GameState = {};

static void
OutCPUInfo(void) {}

static void
OutRAMInfo(void) {}

extern "C" GAME_TRIGGER(GameTrigger) {
	switch (TriggerType) {
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// NOTE(ivan): Game initialization.
		////////////////////////////////////////////////////////////////////////////////////////////////////
	case GameTriggerType_Prepare: {
		// NOTE(ivan): Set platform layer interface.
		GameState.PlatformAPI = PlatformAPI;

		// NOTE(ivan): Set platform-exchangable data.
		GameState.GameMemory = GameMemory;
		GameState.GameClocks = GameClocks;
		GameState.GameInput = GameInput;

		// NOTE(ivan): Organize memory partitions.
		u32 FreeStoragePercent = 100;
		FreeStoragePercent = CreateMemoryStack(&GameState.PerFrameStack, "PerFrameStack",
											   Percentage(10, FreeStoragePercent));
		FreeStoragePercent = CreateMemoryStack(&GameState.PermanentStack, "PermanentStack",
											   Percentange(30, FreeStaragePercent));
		FreeStoragePercent = CreateMemoryStack(&GameState.CommandsPool, "CommandsPool",
											   Percentage(10, FreeStoragePercent));
		FreeStoragePercent = CreateMemoryStack(&GameState.SettingsPool, "SettingsPool",
											   Percentage(10, FreeStoragePercent));

		// NOTE(ivan): Register commands.
		RegisterCommand(&GameState.CommandsRegistry, "quit", CommandQuit);
		RegisterCommand(&GameState.CommandsRegistry, "causeav", CommandCauseAV);
		RegisterCommand(&GameState.CommandsRegistry, "printcpu", CommandPrintCPU);
		RegisterCommand(&GameState.CommandsRegistry, "printram", CommandPrintRAM);

		// NOTE(ivan): Out generic host information.
		OutCPUInfo();
		OutRAMInfo();
	} break;

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// NOTE(ivan): Game de-initialization.
		////////////////////////////////////////////////////////////////////////////////////////////////////
	case GameTriggerType_Release: {
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
